// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "Database/Value.h"

THIRD_PARTY_INCLUDES_START
#	include <variant>
#	include <nanodbc/nanodbc.h>
THIRD_PARTY_INCLUDES_END

#include "Database/Core/DatabaseValueInternal.h"

DEFINE_LOG_CATEGORY(LogDatabaseValue);

const FDatabaseValue FDatabaseValue::NullValue;

FDatabaseValue::FDatabaseValue()
	: Internal(MakeUnique<FDatabaseValueInternal>())
{
	Internal->Get() = FNullValue();
}

/* static */ FDatabaseValue FDatabaseValue::Null()
{
	return FDatabaseValue();
}

FDatabaseValue::FDatabaseValue(const FDatabaseValue& Other)
	: FDatabaseValue()
{
	*this = Other;
}

FDatabaseValue::FDatabaseValue(FDatabaseValue&& Other) 
	: Internal(MoveTemp(Other.Internal))
{}

FDatabaseValue::~FDatabaseValue() = default;

FDatabaseValue& FDatabaseValue::operator=(FDatabaseValue&& Other)
{
	Internal = MoveTemp(Other.Internal);
	return *this;
}

FDatabaseValue& FDatabaseValue::operator=(const FDatabaseValue& Other)
{
	Internal->Get() = Other.Internal->Get();
	return *this;
}

FDatabaseValue::FDatabaseValue(bool bValue) : FDatabaseValue()
{
	Internal->Get() = FValue(bValue);
}

FDatabaseValue::FDatabaseValue(const FDatabaseTimestamp& Value) : FDatabaseValue()
{
	Internal->Get().operator=(Value);
}

FDatabaseValue::FDatabaseValue(uint8 Value) : FDatabaseValue()
{
	Internal->Get().operator=(FValue(Value));
}

FDatabaseValue::FDatabaseValue(int32 Value) : FDatabaseValue()
{
	Internal->Get().operator=(FValue(Value));
}

FDatabaseValue::FDatabaseValue(int64 Value) : FDatabaseValue()
{
	Internal->Get().operator=(FValue(Value));
}

FDatabaseValue::FDatabaseValue(double Value) : FDatabaseValue()
{
	Internal->Get().operator=(Value);
}

FDatabaseValue::FDatabaseValue(FString Value) : FDatabaseValue()
{
	Internal->Get().operator=(MoveTemp(Value));
}

FDatabaseValue::FDatabaseValue(const TCHAR* Value)
	: FDatabaseValue(FString(Value))
{
}

FDatabaseValue::FDatabaseValue(const FDatabaseDate& Date) : FDatabaseValue()
{
	Internal->Get() = Date;
}

FDatabaseValue::operator uint8()	const
{
	const EDatabaseValueType Type = GetType();
	switch (Type)
	{
	case EDatabaseValueType::Uint8:		return std::get<uint8>	(Internal->Get());
	case EDatabaseValueType::Boolean:	return std::get<bool>		(Internal->Get());
	case EDatabaseValueType::Int32:		
		UE_LOG(LogDatabaseValue, Warning, TEXT("Converted a database value from int32 to uint8."));
		return std::get<int32>	(Internal->Get());
	case EDatabaseValueType::Int64:		
		UE_LOG(LogDatabaseValue, Warning, TEXT("Converted a database value from int64 to uint8."));
		return std::get<int64>	(Internal->Get());
	case EDatabaseValueType::Double:	
		UE_LOG(LogDatabaseValue, Warning, TEXT("Converted a database value from double to uint8."));
		return std::get<double>	(Internal->Get());
	}
	
	UE_LOG(LogDatabaseValue, Error, TEXT("Converted a database value to uint8 but the type isn't numeric."));

	return 0;
}

FDatabaseValue::operator int32()	const
{
	const EDatabaseValueType Type = GetType();
	switch (Type)
	{
	case EDatabaseValueType::Uint8:		return std::get<uint8>(Internal->Get());
	case EDatabaseValueType::Boolean:	return std::get<bool> (Internal->Get());
	case EDatabaseValueType::Int32:		return std::get<int32>(Internal->Get());
	case EDatabaseValueType::Int64:
		UE_LOG(LogDatabaseValue, Warning, TEXT("Converted a database value from int64 to int32."));
		return std::get<int64>(Internal->Get());
	case EDatabaseValueType::Double:
		UE_LOG(LogDatabaseValue, Warning, TEXT("Converted a database value from double to int32."));
		return std::get<double>(Internal->Get());
	}

	UE_LOG(LogDatabaseValue, Error, TEXT("Converted a database value to int32 but the type isn't numeric."));

	return 0;
}

FDatabaseValue::operator int64()	const
{
	const EDatabaseValueType Type = GetType();
	switch (Type)
	{
	case EDatabaseValueType::Uint8:		return std::get<uint8>(Internal->Get());
	case EDatabaseValueType::Boolean:	return std::get<bool> (Internal->Get());
	case EDatabaseValueType::Int32:		return std::get<int32>(Internal->Get());
	case EDatabaseValueType::Int64:		return std::get<int64>(Internal->Get());
	case EDatabaseValueType::Double:
		UE_LOG(LogDatabaseValue, Warning, TEXT("Converted a database value from double to int64."));
		return std::get<double>(Internal->Get());
	}

	UE_LOG(LogDatabaseValue, Error, TEXT("Converted a database value to int64 but the type isn't numeric."));

	return 0;
}

FDatabaseValue::operator float()	const
{
	return (float)double(*this);
}

FDatabaseValue::operator double()	const
{
	const EDatabaseValueType Type = GetType();
	switch (Type)
	{
	case EDatabaseValueType::Uint8:		return std::get<uint8> (Internal->Get());
	case EDatabaseValueType::Boolean:	return std::get<bool>  (Internal->Get());
	case EDatabaseValueType::Int32:		return std::get<int32> (Internal->Get());
	case EDatabaseValueType::Int64:		return std::get<int64> (Internal->Get());
	case EDatabaseValueType::Double:	return std::get<double>(Internal->Get());
	}

	UE_LOG(LogDatabaseValue, Error, TEXT("Converted a database value to double but the type isn't numeric."));

	return 0.;
}

FDatabaseValue::operator FString()	const
{
	return ToString();
}

FString FDatabaseValue::ToString(bool bWarn) const
{
	const EDatabaseValueType Type = GetType();

	switch (Type)
	{
	case EDatabaseValueType::Uint8:		
		if (bWarn) UE_LOG(LogDatabaseValue, Warning, TEXT("Converted a database value from uint8 to FString."));
		return FString::Printf(TEXT("%f"), std::get<uint8>(Internal->Get()));
	case EDatabaseValueType::Boolean:	
		if (bWarn) UE_LOG(LogDatabaseValue, Warning, TEXT("Converted a database value from boolean to FString."));
		return FString::Printf(TEXT("%s"), std::get<bool>(Internal->Get()) ? TEXT("true") : TEXT("false"));
	case EDatabaseValueType::Int32:		
		if (bWarn) UE_LOG(LogDatabaseValue, Warning, TEXT("Converted a database value from int32 to FString."));
		return FString::Printf(TEXT("%d"), std::get<int32>(Internal->Get()));
	case EDatabaseValueType::Int64:		
		if (bWarn) UE_LOG(LogDatabaseValue, Warning, TEXT("Converted a database value from int64 to FString."));
		return FString::Printf(TEXT("%lld"), std::get<int64>(Internal->Get()));
	case EDatabaseValueType::Double:
		if (bWarn) UE_LOG(LogDatabaseValue, Warning, TEXT("Converted a database value from double to FString."));
		return FString::Printf(TEXT("%f"), std::get<double>(Internal->Get()));
	case EDatabaseValueType::Timestamp:
	{
		if (bWarn) UE_LOG(LogDatabaseValue, Warning, TEXT("Converted a database value from double to FDatabaseTimestamp."));
		const FDatabaseTimestamp Timestamp = std::get<FDatabaseTimestamp>(Internal->Get());
		return FString::Printf(TEXT("%04d-%02d-%02d %02d:%02d:%02d"),
			Timestamp.Year, Timestamp.Month, Timestamp.Day, Timestamp.Hour, Timestamp.Minute, Timestamp.Second);
	}
	case EDatabaseValueType::Date:
	{
		if (bWarn) UE_LOG(LogDatabaseValue, Warning, TEXT("Converted a database value from double to FDatabaseTimestamp."));
		const FDatabaseDate Timestamp = std::get<FDatabaseDate>(Internal->Get());
		return FString::Printf(TEXT("%04d-%02d-%02d"),
			Timestamp.Year, Timestamp.Month, Timestamp.Day);
	}
	case EDatabaseValueType::String:
		return std::get<FString>(Internal->Get());
	case EDatabaseValueType::Null:
		return TEXT("NULL");
	}

	UE_LOG(LogDatabaseValue, Error, TEXT("Converted a database value to FString but the type isn't convertible."));

	return TEXT("");
}

FDatabaseValue::operator FDatabaseTimestamp() const
{
	if (GetType() == EDatabaseValueType::Timestamp)
	{
		return std::get<FDatabaseTimestamp>(Internal->Get());
	}

	UE_LOG(LogDatabaseValue, Error, TEXT("Converted a database value to FDatabaseTimestamp but the type isn't convertible."));

	return FDatabaseTimestamp();
}

FDatabaseValue::operator FDatabaseDate()	const
{
	if (GetType() == EDatabaseValueType::Date)
	{
		return std::get<FDatabaseDate>(Internal->Get());
	}

	UE_LOG(LogDatabaseValue, Error, TEXT("Converted a database value to FDatabaseDate but the type isn't convertible."));

	return FDatabaseDate();
}

EDatabaseValueType FDatabaseValue::GetType() const
{
	class FVariantTypeVisitor
	{
	public:
		EDatabaseValueType Type;
	public:
#		define MakeVisitorFunc(InType, Value)					\
			void operator()(const InType&) { Type = Value; }	
		MakeVisitorFunc(uint8,				EDatabaseValueType::Uint8);
		MakeVisitorFunc(int32,				EDatabaseValueType::Int32);
		MakeVisitorFunc(int64,				EDatabaseValueType::Int64);
		MakeVisitorFunc(double,				EDatabaseValueType::Double);
		MakeVisitorFunc(FString,			EDatabaseValueType::String);
		MakeVisitorFunc(FNullValue,			EDatabaseValueType::Null);
		MakeVisitorFunc(FDatabaseDate,		EDatabaseValueType::Date);
		MakeVisitorFunc(FDatabaseTimestamp,	EDatabaseValueType::Timestamp);
#		undef MakeVisitorFunc
	} Visitor;

	std::visit(Visitor, Internal->Get());

	return Visitor.Type;
}

FDatabaseDate FDatabaseDate::Now()
{
	const FDateTime Current = FDateTime::Now();
	
	FDatabaseDate Date;

	Date.Year		= Current.GetYear();
	Date.Month		= Current.GetMonth();
	Date.Day		= Current.GetDay();

	return Date;
}

FDatabaseTimestamp FDatabaseTimestamp::Now()
{
	const FDateTime Current = FDateTime::Now();
	
	FDatabaseTimestamp Timestamp;

	Timestamp.Year		= Current.GetYear();
	Timestamp.Month		= Current.GetMonth();
	Timestamp.Day		= Current.GetDay();
	Timestamp.Hour		= Current.GetHour();
	Timestamp.Minute	= Current.GetMinute();
	Timestamp.Second	= Current.GetSecond();
	Timestamp.Fract	    = Current.GetMillisecond();

	return Timestamp;
}


