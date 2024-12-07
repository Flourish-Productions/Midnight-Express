// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "OdbClient.h"
#include "Misc/ScopeLock.h"
#include "Database/Core/SqlTypes.h"
#include "Database/Core/SqlErrors.h"

#include "DatabaseConnectorModule.h"

#include <forward_list>

#define DECLARE_CACHE(type) std::forward_list<TUniquePtr<type>> Cache_ ## type;
#define CACHE_DATA(type, value, ...) do {					\
	TUniquePtr<type> Cached = MakeUnique<type>(value);		\
	Statement.bind(i, Cached.Get() __VA_ARGS__);			\
	Cache_ ## type .emplace_front(MoveTemp(Cached));		\
	} while(0)

static constexpr int32 ExecuteQueryConnectionLostRetryCount = 2;

static nanodbc::timestamp Convert(const FDatabaseTimestamp & Timestamp)
{
	nanodbc::timestamp Raw;

	Raw.year	= Timestamp.Year;
	Raw.month	= Timestamp.Month;
	Raw.day		= Timestamp.Day;
	Raw.hour	= Timestamp.Hour;
	Raw.min		= Timestamp.Minute;
	Raw.sec		= Timestamp.Second;
	Raw.fract	= Timestamp.Fract;

	return Raw;
}

static FDatabaseTimestamp Convert(const nanodbc::timestamp & RawTimestamp)
{
	FDatabaseTimestamp Timestamp;

	Timestamp.Day		= RawTimestamp.day;
	Timestamp.Fract		= RawTimestamp.fract;
	Timestamp.Hour		= RawTimestamp.hour;
	Timestamp.Minute	= RawTimestamp.min;
	Timestamp.Month		= RawTimestamp.month;
	Timestamp.Second	= RawTimestamp.sec;
	Timestamp.Year		= RawTimestamp.year;

	return Timestamp;
}

static nanodbc::date Convert(const FDatabaseDate & Date)
{
	nanodbc::date Raw;

	Raw.year = Date.Year;
	Raw.month = Date.Month;
	Raw.day = Date.Day;

	return Raw;

}

static FDatabaseDate Convert(const nanodbc::date & RawDate)
{
	FDatabaseDate Date;

	Date.Day = RawDate.day;
	Date.Month = RawDate.month;
	Date.Year = RawDate.year;

	return Date;
}

static FDatabaseValue Convert(const nanodbc::result& QueryResult, int32 Index)
{
	if (QueryResult.is_null(Index))
	{
		return FDatabaseValue::Null();
	}

	const int32 Type = QueryResult.column_datatype(Index);
	switch (Type)
	{

	case SQL_CHAR:
	case SQL_VARCHAR:
	case SQL_WCHAR:
	case SQL_WVARCHAR:
	case SQL_WLONGVARCHAR:
	case SQL_LONGVARCHAR:
		return UTF8_TO_TCHAR(QueryResult.get<std::string>(Index, "").c_str());

	case SQL_FLOAT:
		return(QueryResult.get<float>(Index, 0.f));
		break;

	case SQL_DOUBLE:
	case SQL_NUMERIC:
	case SQL_DECIMAL:
	case SQL_REAL:
		return(QueryResult.get<double>(Index, 0.));

	case SQL_INTEGER:
	case SQL_BIGINT:
		return(QueryResult.get<int64>(Index, 0));

	case SQL_SMALLINT:
	case SQL_TINYINT:
	case SQL_TYPE_TINYINT:
		return(QueryResult.get<int32>(Index, 0));

	case SQL_DATETIME:
		return Convert(QueryResult.get<nanodbc::date>(Index, nanodbc::date()));

	case SQL_TIMESTAMP:
	case SQL_TYPE_TIMESTAMP:
		return Convert(QueryResult.get<nanodbc::timestamp>(Index, nanodbc::timestamp()));

	default:
	{
#if WITH_EDITOR
		const FString TypeName = UTF8_TO_TCHAR(QueryResult.column_datatype_name(Index).c_str());
		UE_LOG(LogDatabaseConnector, Warning, TEXT("Type %d:%s not handled. Replacing with NULL. ")
			TEXT("If you want this type to be supported, send an email to pandores.marketplace@gmail.com. ")
			TEXT("Please, include \"%d:%s\" in your email."),
			Type, *TypeName, Type, *TypeName);
#else
		UE_LOG(LogDatabaseConnector, Warning, TEXT("Type %d is not supported."), Type);
#endif
	}

	case SQL_UNKNOWN_TYPE:
		return FDatabaseValue::Null();
	}

	return FDatabaseValue::Null();
}

static nanodbc::result ExecuteStatementWithParameters(nanodbc::statement& Statement, const TArray<FDatabaseValue>& Parameters)
{
	using StdString		= std::string;
	using NanoTimestamp = nanodbc::timestamp;
	using NanoDate		= nanodbc::date;

	// The parameters must outlive the query.
	// We store them with only one copy in unique pointers inside lists
	// so they get released only after the query.
	DECLARE_CACHE(bool);
	DECLARE_CACHE(int32);
	DECLARE_CACHE(int64);
	DECLARE_CACHE(double);
	DECLARE_CACHE(StdString);
	DECLARE_CACHE(NanoTimestamp);
	DECLARE_CACHE(NanoDate);

	for (int32 i = 0; i < Parameters.Num(); ++i)
	{
		const FDatabaseValue& Value = Parameters[i];
		switch (Value.GetType())
		{
		case EDatabaseValueType::String:	CACHE_DATA(StdString, TCHAR_TO_UTF8(*Value.ToString()), ->c_str()); break;
		case EDatabaseValueType::Timestamp: CACHE_DATA(NanoTimestamp, Convert(Value.ToTimestamp()));	break;
		case EDatabaseValueType::Date:		CACHE_DATA(NanoDate,      Convert(Value.ToDate()));			break;
		case EDatabaseValueType::Uint8:		CACHE_DATA(int32,  Value);		break; // Use int32 cache as nanodbc doesn't support unsigned char.
		case EDatabaseValueType::Int32:		CACHE_DATA(int32,  Value);		break;
		case EDatabaseValueType::Int64:		CACHE_DATA(int64,  Value);		break;
		case EDatabaseValueType::Double:	CACHE_DATA(double, Value);		break;
		case EDatabaseValueType::Boolean:	CACHE_DATA(int32,  Value);		break; // Use int32 cache as nanodbc doesn't support bool.
		case EDatabaseValueType::Null: 		Statement.bind_null(i);			break;
		default:
			UE_LOG(LogDatabaseConnector, Error, TEXT("Unhandled type %d. Using NULL instead."), (int32)Value.GetType());
			Statement.bind_null(i);
		}
	}

	return nanodbc::execute(Statement);
}

static FQueryResult GetQueryResult(nanodbc::result& QueryResult, EDatabaseError& OutError)
{
	TArray<FString>						Headers;
	TArray<FColumnMetadata>				Metadata;
	TArray64<TArray<FDatabaseValue>>	Body;

	const int32 ColumnCount = (int32)QueryResult.columns();
	const int64 AffectedRowsCount = QueryResult.affected_rows();

	Headers .Reserve(ColumnCount);
	Metadata.Reserve(ColumnCount);

	if (QueryResult.affected_rows() > 0)
	{
		Body.Reserve(QueryResult.affected_rows());
	}

	for (int32 i = 0; i < ColumnCount; ++i)
	{
		Headers.Add(UTF8_TO_TCHAR(QueryResult.column_name(i).c_str()));

		FColumnMetadata& Meta = Metadata.Emplace_GetRef();

		Meta.DecimalDigits	= QueryResult.column_decimal_digits(i);
		Meta.DataTypeName	= UTF8_TO_TCHAR(QueryResult.column_datatype_name(i).c_str());
		Meta.Size			= QueryResult.column_size(i);
	}

	try
	{
		for (int32 i = 0; QueryResult.next(); ++i)
		{
			TArray<FDatabaseValue>& Row = Body.Emplace_GetRef();

			Row.Reserve(ColumnCount);

			for (int32 j = 0; j < ColumnCount; ++j)
			{
				Row.Add(Convert(QueryResult, j));
			}
		}
	}
	catch (const nanodbc::database_error& Error)
	{
		UE_LOG(LogDatabaseConnector, Error, TEXT("Failed to fetch results. Code: %d. Reason: %s"), Error.native(), UTF8_TO_TCHAR(Error.what()));

		//UE_LOG(LogTemp, Error, TEXT("Native error: %d, State: %s"), Error.native(), UTF8_TO_TCHAR(Error.state().c_str()));

		OutError = NSqlErrors::ConvertState(Error.state());;
	}
	catch (const nanodbc::index_range_error& Exception)
	{
		UE_LOG(LogDatabaseConnector, Error, TEXT("Failed to fetch results because of index error. Reason: %s"), UTF8_TO_TCHAR(Exception.what()));
	}

	return FQueryResult(MoveTemp(Headers), MoveTemp(Body), MoveTemp(Metadata), AffectedRowsCount);
}

//////////////////////////////////////////////////////////////
// FConnection

FConnection::FConnection(const FString& Url)
	: Connection(TCHAR_TO_UTF8(*Url))
	, bIsAvailable(true)
{
}

bool FConnection::TryAcquire()
{
	bool bExpected = true;
	return bIsAvailable.CompareExchange(bExpected, false);
}

void FConnection::Lock()
{
	const bool bIsConnectionAvailable = bIsAvailable.Exchange(false);
	
	check(bIsConnectionAvailable);
}

void FConnection::Unlock()
{
	const bool bWasConnectionLocked = !bIsAvailable.Exchange(true);

	check(bWasConnectionLocked);
}

bool FConnection::Connect(const FString& Dsn, const int32 Timeout)
{
	try
	{
		Connection.connect(TCHAR_TO_UTF8(*Dsn), Timeout);
	}
	catch (const nanodbc::database_error& DatabaseError)
	{
		UE_LOG(LogDatabaseConnector, Error, TEXT("Failed to connect. Code: %d. Reason: %s"),
			DatabaseError.native(), UTF8_TO_TCHAR(DatabaseError.what()));
		return false;
	}

	return true;
}

FQueryResult FConnection::Query(const FString& Sql, const FString& Dsn, const TArray<FDatabaseValue>& Parameters, EDatabaseError& OutError, int32 RecursiveCount)
{
	OutError = EDatabaseError::None;
	
	nanodbc::result QueryResult;

	{
		std::string Utf8Query = TCHAR_TO_UTF8(*Sql);

		try
		{
			nanodbc::statement Statement(Connection);

			nanodbc::prepare(Statement, Utf8Query);

			QueryResult = ExecuteStatementWithParameters(Statement, Parameters);
		}
		catch (const nanodbc::database_error& Error)
		{
			UE_LOG(LogDatabaseConnector, Error, TEXT("Failed to query database. State: %s, Reason: %s"), 
				UTF8_TO_TCHAR(Error.state().c_str()), UTF8_TO_TCHAR(Error.what()));

			OutError = NSqlErrors::ConvertState(Error.state());;
		}
	}

	if (OutError != EDatabaseError::None)
	{
		// We try to reconnect if the connection was closed.
		if (OutError == EDatabaseError::ConnectionClosed)
		{
			if (RecursiveCount < ExecuteQueryConnectionLostRetryCount)
			{
				UE_LOG(LogDatabaseConnector, Warning, TEXT("Connection lost. Trying to reconnect..."))

				if (Connect(Dsn))
				{
					UE_LOG(LogDatabaseConnector, Log, TEXT("Reconnected. Restarting query."));
				}
				else
				{
					UE_LOG(LogDatabaseConnector, Warning, TEXT("Failed to reconnect."));
				}

				return Query(Sql, Dsn, Parameters, OutError, ++RecursiveCount);
			}

			UE_LOG(LogDatabaseConnector, Error, TEXT("Query dropped has it failed to reconnect."));
		}

		return FQueryResult();
	}

	if (!QueryResult || QueryResult.columns() <= 0)
	{
		UE_LOG(LogDatabaseConnector, Log, TEXT("Query didn't return a result."));
		return FQueryResult(QueryResult.affected_rows());
	}

	return GetQueryResult(QueryResult, OutError);
}

//////////////////////////////////////////////////////////////////////
// FConnectionPool

FConnectionPool::~FConnectionPool()
{

}

EDatabaseError FConnectionPool::Create(const FString& Url, const int32 Count)
{
	try
	{
		// We no more need lock here
		// Implementation prevents concurrency.
		// i.e. can't use a pool while it is created.
		Connections.Reserve(Count);
		for (int32 i = 0; i < Count; ++i)
		{
			Connections.Emplace(MakeUnique<FConnection>(Url));
		}
	}

	catch (const nanodbc::database_error& Exception)
	{
		UE_LOG(LogDatabaseConnector, Error, TEXT("Failed to create pool. Code: %d. Reason: %s"), Exception.native(), UTF8_TO_TCHAR(Exception.what()));

		return EDatabaseError::FailedToOpenConnection;
	}

	return EDatabaseError::None;
}

FConnection& FConnectionPool::AcquireOne()
{
	for (const TUniquePtr<FConnection>& Connection : Connections)
	{
		if (Connection->TryAcquire())
		{
			return *Connection;
		}
	}

	// With the current implementation where threadCount <=> connectionCount,
	// this code is never executed.
	// We have a guard for it just in case.
	{ 
		std::unique_lock<std::mutex> Locker(Sleeper);
		SleeperCondition.wait(Locker);
	}

	return AcquireOne();
}

int32 FConnectionPool::GetPoolSize() const
{
	return Connections.Num();
}

void FConnectionPool::Reconnect(const FString& Dsn, const int32 Timeout, int32& Reconnected, int32& Skipped, int32& Failed)
{
	Reconnected = Skipped = Failed = 0;
	for (const TUniquePtr<FConnection>& Connection : Connections)
	{
		if (Connection->TryAcquire())
		{
			if (Connection->Connect(Dsn, Timeout))
			{
				++Reconnected;
			}
			else
			{
				++Failed;
			}

			Connection->Unlock();
		}
		else ++Skipped;
	}
}

//////////////////////////////////////////////////////////////////////
// FConnectionHandle


FConnectionHandle::FConnectionHandle(FConnectionPool& InPool)
	: Pool(&InPool)
{
	Connection = &Pool->AcquireOne();
}

FConnectionHandle::~FConnectionHandle()
{
	Connection->Unlock();

	Pool->SleeperCondition.notify_one();
}

FConnection& FConnectionHandle::Get()
{
	return *Connection;
}


