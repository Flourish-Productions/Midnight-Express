// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Value.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDatabaseValue, Log, All);

UENUM(BlueprintType)
enum class EDatabaseValueType : uint8
{
	Null,
	Boolean,
	Uint8,
	Int32,
	Int64,
	Double		UMETA(DisplayName = "Float"),
	String,
	Timestamp,
	Date
};

USTRUCT(BlueprintType)
struct DATABASECONNECTOR_API FDatabaseTimestamp
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database|Timestamp", meta = (ClampMin = "0", ClampMax = "9999", UIMin = "0", UIMax = "9999"))
	int32 Year;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database|Timestamp", meta = (ClampMin = "1", ClampMax = "12", UIMin = "1", UIMax = "12"))
	uint8 Month;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database|Timestamp", meta = (ClampMin = "1", ClampMax = "31", UIMin = "1", UIMax = "31"))
	uint8 Day;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database|Timestamp", meta = (ClampMin = "0", ClampMax = "23", UIMin = "1", UIMax = "12"))
	uint8 Hour;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database|Timestamp", meta = (ClampMin = "0", ClampMax = "59", UIMin = "0", UIMax = "59"))
	uint8 Minute;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database|Timestamp", meta = (ClampMin = "0", ClampMax = "59", UIMin = "0", UIMax = "59"))
	uint8 Second;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database|Timestamp", meta = (ClampMin = "0", ClampMax = "1000", UIMin = "0", UIMax = "1000"))
	int32 Fract;

public:
	static FDatabaseTimestamp Now();
};

USTRUCT(BlueprintType)
struct DATABASECONNECTOR_API FDatabaseDate
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database|Date", meta = (ClampMin = "0", ClampMax = "9999", UIMin = "0", UIMax = "9999"))
	int32 Year;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database|Date", meta = (ClampMin = "1", ClampMax = "12", UIMin = "1", UIMax = "12"))
	uint8 Month;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database|Date", meta = (ClampMin = "1", ClampMax = "31", UIMin = "1", UIMax = "31"))
	uint8 Day;

public:
	static FDatabaseDate Now();
};

USTRUCT(BlueprintType)
struct DATABASECONNECTOR_API FDatabaseValue
{
	GENERATED_BODY()
public:
	FDatabaseValue();
	
	FDatabaseValue(bool);
	FDatabaseValue(uint8);
	FDatabaseValue(int32);
	FDatabaseValue(int64);
	FDatabaseValue(double);
	FDatabaseValue(FString);
	FDatabaseValue(const TCHAR*);
	FDatabaseValue(const FDatabaseTimestamp&);
	FDatabaseValue(const FDatabaseDate&);

	FDatabaseValue(const FDatabaseValue&);
	FDatabaseValue(FDatabaseValue&&);

	~FDatabaseValue();

	FDatabaseValue& operator=(FDatabaseValue&&);
	FDatabaseValue& operator=(const FDatabaseValue&);

	static FDatabaseValue Null();

	static const FDatabaseValue NullValue;

	operator uint8()	const;
	operator int32()	const;
	operator int64()	const;
	operator double()	const;
	operator float()	const;
	operator FString()	const;

	operator FDatabaseTimestamp()	const;
	operator FDatabaseDate()		const;

	FORCEINLINE bool	IsNull()   const { return GetType() == EDatabaseValueType::Null; }
	FORCEINLINE uint8	ToUint8()  const { return *this; }
	FORCEINLINE int32   ToInt32()  const { return *this; }
	FORCEINLINE int64   ToInt64()  const { return *this; }
	FORCEINLINE double  ToDouble() const { return *this; }
	
	FString ToString(bool bWarnConversion = true) const;

	FORCEINLINE FDatabaseTimestamp	ToTimestamp()	const { return *this; }
	FORCEINLINE FDatabaseDate		ToDate()		const { return *this; }

	EDatabaseValueType GetType() const;

private:
	TUniquePtr<class FDatabaseValueInternal> Internal;

private:
	friend class FDatabasePoolQueryTask;
	friend class UDatabasePool;
};
