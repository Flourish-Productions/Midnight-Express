// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "Database/Pool.h"

#if PLATFORM_WINDOWS
#	include "Windows/AllowWindowsPlatformTypes.h"
#endif 

THIRD_PARTY_INCLUDES_START
#	include <nanodbc/nanodbc.h>
THIRD_PARTY_INCLUDES_END

#if PLATFORM_WINDOWS
#	include "Windows/HideWindowsPlatformTypes.h"
#endif // PLATFORM_WINDOWS

#include "Core/OdbClient.h"
#include "Core/DatabasePoolTasks.h"
#include "Core/DatabaseValueInternal.h"
#include "Core/SqlTypes.h"
#include "Database/Core/SqlErrors.h"

#include "UObject/StrongObjectPtr.h"
#include "Misc/QueuedThreadPool.h"
#include "Async/Async.h"

#include "DatabaseConnectorModule.h"

#include "Runtime/Launch/Resources/Version.h"

#define LAMBDA_MOVE_TEMP(Var) Var = MoveTemp(Var)

#define START_THREAD_POOL_EXECUTION(...)						\
	NDatabasePoolThread::AsyncTask(ThreadPool.Get(),			\
	[															\
		ConnectionPool		 = (this->ConnectionPool),			\
		ConnectionDsn		 = (this->ConnectionDsn)			\
		, ## __VA_ARGS__										\
	]() mutable -> void											\
	{

#define END_THREAD_POOL_EXECUTION(...) })

#define START_THREAD_EXECUTION(ThreadName, ...)					\
	AsyncTask(ThreadName, [										\
		__VA_ARGS__												\
	]() mutable -> void											\
	{


#define END_THREAD_EXECUTION() })


FString UDatabasePool::ThreadName = TEXT("DatabaseConnector_Pool");

UDatabasePool::UDatabasePool()
	: ThreadPool    (FQueuedThreadPool::Allocate())
	, ConnectionPool(nullptr)
{
}

UDatabasePool::~UDatabasePool() = default;

void UDatabasePool::Blueprint_CreatePool(const FString& DriverName, const FString& Username, const FString& Password, const FString& Server, const int32 Port, const FString& Database, const int32 PoolSize, FDatabasePoolDelegate Callback)
{
	CreatePool(DriverName, Username, Password, Server, Port, Database, PoolSize, FDatabasePoolCallback::CreateLambda([Callback = MoveTemp(Callback)](EDatabaseError Error, UDatabasePool* Pool) -> void
	{
		Callback.ExecuteIfBound(Error, Pool);
	}));
}

UDatabasePool* UDatabasePool::CreatePoolSync(const FString& DriverName, const FString& Username, const FString& Password, const FString& Server, const int32 Port, const FString& Database, const int32 PoolSize, EDatabaseError& OutError)
{
	// TODO: Factorize with CreatePool to avoid duplicate code.

	if (PoolSize <= 0)
	{
		ensureMsgf(PoolSize > 0, TEXT("Pool size must be strictly greater than 0. Provided %d."), PoolSize);

		return nullptr;
	}

	FString Url = FString::Printf(TEXT("DRIVER=%s;UID=%s;PORT=%d;DATABASE=%s;SERVER=%s;TCPIP=1;"),
		*DriverName, *Username, Port, *Database, *Server);

	UE_LOG(LogDatabaseConnector, Log, TEXT("Creating pool of size %d with parameters {%s}, with%s password."),
		PoolSize, *Url, Password.IsEmpty() ? TEXT("out") : TEXT(""));

	// We don't want to print the password to logs so we add it afterward.
	if (!Password.IsEmpty())
	{
		Url += TEXT("PWD=") + Password;
	}

	FConnectionPoolPtr ConPool = MakeShared<FConnectionPool, ESPMode::ThreadSafe>();

	OutError = ConPool->Create(Url, PoolSize);

	if (OutError == EDatabaseError::None)
	{
		UDatabasePool* const Pool = NewObject<UDatabasePool>();

		Pool->ConnectionPool = ConPool;
		Pool->ConnectionDsn = MakeShared<FString, ESPMode::ThreadSafe>(MoveTemp(Url));

		const bool bCreatedPool = Pool->ThreadPool->Create(PoolSize, ThreadStackSize, ThreadPriority
#if ENGINE_MAJOR_VERSION > 4 || (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 26)
			, *ThreadName
#endif
		);

		UE_LOG(LogDatabaseConnector, Log, TEXT("Database Pool created."));

		ensureMsgf(bCreatedPool, TEXT("Failed to create Thread Pool."));

		return Pool;
	}

	return nullptr;
}

void UDatabasePool::CreatePool
(
	const FString& DriverName, const FString& Username, const FString& Password, const FString& Server,
	const int32 Port, const FString& Database, const int32 PoolSize, FDatabasePoolCallback Callback
)
{
	if (!Callback.IsBound())
	{
		return;
	}

	if (PoolSize <= 0)
	{
		ensureMsgf(PoolSize > 0, TEXT("Pool size must be strictly greater than 0. Provided %d."), PoolSize);

		Callback.ExecuteIfBound(EDatabaseError::InvalidPoolSize, nullptr);
		return;
	}

	FString Url = FString::Printf(TEXT("DRIVER=%s;UID=%s;PORT=%d;DATABASE=%s;SERVER=%s;TCPIP=1;"),
		*DriverName, *Username, Port, *Database, *Server);

	UE_LOG(LogDatabaseConnector, Log, TEXT("Creating pool of size %d with parameters {%s}, with%s password."), 
		PoolSize, *Url, Password.IsEmpty() ? TEXT("out") : TEXT(""));

	// We don't want to print the password to logs so we add it afterward.
	if (!Password.IsEmpty())
	{
		Url += TEXT("PWD=") + Password;
	}

	// We can't use our thread pool yet as it gets created later on game thread.
	START_THREAD_EXECUTION(ENamedThreads::AnyBackgroundThreadNormalTask,
		LAMBDA_MOVE_TEMP(Url), LAMBDA_MOVE_TEMP(Callback), PoolSize);

	FConnectionPoolPtr ConPool = MakeShared<FConnectionPool, ESPMode::ThreadSafe>();

	const EDatabaseError Error = ConPool->Create(Url, PoolSize);

	// Go back to game thread to create the UObject pool.
	START_THREAD_EXECUTION(ENamedThreads::GameThread,
		LAMBDA_MOVE_TEMP(ConPool), PoolSize, Error, LAMBDA_MOVE_TEMP(Callback), LAMBDA_MOVE_TEMP(Url));

	if (Error == EDatabaseError::None)
	{
		UDatabasePool* const Pool = NewObject<UDatabasePool>();

		Pool->ConnectionPool = ConPool;
		Pool->ConnectionDsn  = MakeShared<FString, ESPMode::ThreadSafe>(MoveTemp(Url));

		const bool bCreatedPool = Pool->ThreadPool->Create(PoolSize, ThreadStackSize, ThreadPriority
#if ENGINE_MAJOR_VERSION > 4 || (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 26)
			, *ThreadName
#endif
		);

		UE_LOG(LogDatabaseConnector, Log, TEXT("Database Pool created."));
		
		ensureMsgf(bCreatedPool, TEXT("Failed to create Thread Pool."));

		Callback.ExecuteIfBound(Error, Pool);
	}
	else
	{
		Callback.ExecuteIfBound(Error, nullptr);
	}
	
	END_THREAD_EXECUTION(); // Game Thread

	END_THREAD_EXECUTION(); // Background Normal Pri Thread
}

void UDatabasePool::Blueprint_Query(FString Query, TArray<FDatabaseValue> Parameters, FDatabaseQueryDelegate Callback)
{
	UDatabasePool::Query(MoveTemp(Query), MoveTemp(Parameters), FDatabaseQueryCallback::CreateLambda([Callback = MoveTemp(Callback)](EDatabaseError Error, const FQueryResult& Results) -> void
	{
		Callback.ExecuteIfBound(Error, Results);
	}));
}

FQueryResult UDatabasePool::QuerySync(FString Query, TArray<FDatabaseValue> Parameters, EDatabaseError& OutError)
{
	FQueryResult   Result;

	{
		FConnectionHandle Handle(*ConnectionPool);

		FConnection& Connection = Handle.Get();

		Result = Connection.Query(Query, *ConnectionDsn, Parameters, OutError);
	}

	return Result;
}

void UDatabasePool::Query(FString Query, TArray<FDatabaseValue> Parameters, FDatabaseQueryCallback Callback)
{
	START_THREAD_POOL_EXECUTION(LAMBDA_MOVE_TEMP(Query), LAMBDA_MOVE_TEMP(Parameters), LAMBDA_MOVE_TEMP(Callback));

	EDatabaseError Error;
	FQueryResult   Result;

	{
		FConnectionHandle Handle(*ConnectionPool);

		FConnection& Connection = Handle.Get();

		Result = Connection.Query(Query, *ConnectionDsn, Parameters, Error);
	}

	// Go back to Game Thread for our callback.
	START_THREAD_EXECUTION(ENamedThreads::GameThread, Error, LAMBDA_MOVE_TEMP(Result), LAMBDA_MOVE_TEMP(Callback));

	Callback.ExecuteIfBound(Error, Result);
	
	END_THREAD_EXECUTION(); // Game Thread.

	END_THREAD_POOL_EXECUTION();
}

void UDatabasePool::Reconnect(const int32 Timeout, FPoolReconnectCallback Callback)
{
	START_THREAD_POOL_EXECUTION(LAMBDA_MOVE_TEMP(Callback), Timeout);

	int32 ReconnectedCount	= 0;
	int32 SkippedCount		= 0;
	int32 FailedCount		= 0;

	UE_LOG(LogDatabaseConnector, Log, TEXT("Reconnecting pool to database."));

	ConnectionPool->Reconnect(*ConnectionDsn, Timeout, ReconnectedCount, SkippedCount, FailedCount);

	UE_LOG(LogDatabaseConnector, Log, TEXT("Reconnection to database result: %d reconnected, %d skipped, %d failed.")
		, ReconnectedCount, SkippedCount, FailedCount);

	if (Callback.IsBound())
	{
		START_THREAD_EXECUTION(ENamedThreads::GameThread, FailedCount, ReconnectedCount, SkippedCount, LAMBDA_MOVE_TEMP(Callback));

		Callback.ExecuteIfBound
		(
			ReconnectedCount == 0 ? EDatabaseError::FailedToOpenConnection : EDatabaseError::None,
			ReconnectedCount, SkippedCount, FailedCount
		);

		END_THREAD_EXECUTION(); // Game Thread.
	}

	END_THREAD_POOL_EXECUTION();
}
