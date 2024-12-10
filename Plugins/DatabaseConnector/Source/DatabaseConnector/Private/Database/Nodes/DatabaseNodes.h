// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Database/Value.h"
#include "Database/Pool.h"
#include "Database/QueryResult.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DatabaseNodes.generated.h"

UCLASS()
class UDatabaseConnectorBlueprintLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Value", meta = (CompactNodeTitle = "->", BlueprintAutocast))
	static FDatabaseValue FromDouble(double Value) { return Value; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Value", meta = (CompactNodeTitle = "->", BlueprintAutocast))
	static FDatabaseValue FromTimestamp(const FDatabaseTimestamp& Value) { return Value; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Value", meta = (CompactNodeTitle = "->", BlueprintAutocast))
	static FDatabaseValue FromDate(const FDatabaseDate Value) { return Value; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Value", meta = (CompactNodeTitle = "->", BlueprintAutocast))
	static FDatabaseValue FromUint8(uint8 Value) { return Value; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Value", meta = (CompactNodeTitle = "->", BlueprintAutocast))
	static FDatabaseValue FromBool(bool bValue) { return bValue; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Value", meta = (CompactNodeTitle = "->", BlueprintAutocast))
	static FDatabaseValue FromInt32(int32 Value) { return Value; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Value", meta = (CompactNodeTitle = "->", BlueprintAutocast))
	static FDatabaseValue FromInt64(int64 Value) { return Value; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Value", meta = (CompactNodeTitle = "->", BlueprintAutocast))
	static FDatabaseValue FromFloat(float Value) { return Value; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Value", meta = (CompactNodeTitle = "->", BlueprintAutocast))
	static FDatabaseValue FromString(const FString& Value) { return Value; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Value", meta = (CompactNodeTitle = "NULL"))
	static FDatabaseValue FromNull() { return FDatabaseValue::Null(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Value", meta = (CompactNodeTitle = "->", BlueprintAutocast))
	static FDatabaseDate ToDate(UPARAM(ref) const FDatabaseValue& Value) { return Value; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Value", meta = (CompactNodeTitle = "->", BlueprintAutocast))
	static FDatabaseTimestamp ToTimestamp(UPARAM(ref) const FDatabaseValue& Value) { return Value; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Value", meta = (CompactNodeTitle = "->", BlueprintAutocast))
	static FString ToString(UPARAM(ref) const FDatabaseValue& Value) { return Value; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Value", meta = (CompactNodeTitle = "->", BlueprintAutocast))
	static int32 ToInt32(UPARAM(ref) const FDatabaseValue& Value) { return Value; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Value", meta = (CompactNodeTitle = "->", BlueprintAutocast))
	static int64 ToInt64(UPARAM(ref) const FDatabaseValue& Value) { return Value; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Value", meta = (CompactNodeTitle = "->", BlueprintAutocast))
	static float ToFloat(UPARAM(ref) const FDatabaseValue& Value) { return Value; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Value", meta = (CompactNodeTitle = "->", BlueprintAutocast))
	static double ToDouble(UPARAM(ref) const FDatabaseValue& Value) { return Value; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Value", meta = (CompactNodeTitle = "NULL", BlueprintAutocast))
	static bool IsNull(UPARAM(ref) const FDatabaseValue& Value) { return Value.IsNull(); }
	
	/**
	 * Gets a value by column name.
	 * Access cost is O(ColumnCount) so it is O(1) on a fixed table.
	 * @param ColumnName The column to get the value from.
	 * @param RowIndex The row index to get the value from.
	 * @return The value at the specified location.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Query")
	static UPARAM(DisplayName = "Value") FDatabaseValue GetByColumnName(UPARAM(ref) const FQueryResult& QueryResult, const FString& ColumnName, int64 RowIndex);

	/**
	 * Gets a value by column index.
	 * Access cost is O(1) (faster than get by column name).
	 * @param ColumnName The column to get the value from.
	 * @param RowIndex The row index to get the value from.
	 * @return The value at the specified location.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Query")
	static UPARAM(DisplayName = "Value") FDatabaseValue GetByColumnIndex(UPARAM(ref) const FQueryResult& QueryResult, int32 ColumnIndex, int64 RowIndex);

	/**
	 * Gets the columns.
	 * @return The columns.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Query")
	static UPARAM(DisplayName = "Columns") TArray<FString> GetColumns(UPARAM(ref) const FQueryResult& QueryResult);

	/**
	 * Gets a row.
	 * Access cost is O(1).
	 * @return The row or nullptr if invalid index.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Query")
	static UPARAM(DisplayName = "Row") TArray<FDatabaseValue> GetRow(UPARAM(ref) const FQueryResult& QueryResult, const int64 RowIndex);

	/**
	 * Dumps the data nicely in the output log.
	*/
	UFUNCTION(BlueprintCallable, Category = "Database|Query")
	static void LogDump(UPARAM(ref) const FQueryResult& QueryResult);

	/**
	 * Gets the number of rows.
	 * @return The number of rows.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Query")
	static UPARAM(DisplayName = "Row Count") int64 GetRowCount(UPARAM(ref) const FQueryResult& QueryResult);

	/**
	 * Gets the number of columns.
	 * @return The number of columns.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Query")
	static UPARAM(DisplayName = "Row Count") int32 GetColumnCount(UPARAM(ref) const FQueryResult& QueryResult);

	/**
	 * Gets the columns' metadata.
	 * @return The columns' metadata.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Query")
	static UPARAM(DisplayName = "Metdata") TArray<FColumnMetadata> GetColumnsMetadata(UPARAM(ref) const FQueryResult& QueryResult);

	/**
	 * Gets a column's metadata.
	 * @param ColumnIndex The column index to get.
	 * @return Gets a column's metadata or nullptr if invalid index.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Query")
	static UPARAM(DisplayName = "Metdata") FColumnMetadata GetColumnMetadataByIndex(UPARAM(ref) const FQueryResult& QueryResult, int32 ColumnIndex);

	/**
	 * Gets a column's metadata.
	 * @param ColumnName The column's name to get.
	 * @return Gets a column's metadata or nullptr if not found.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Query")
	static UPARAM(DisplayName = "Metdata") FColumnMetadata GetColumnMetadataByName(UPARAM(ref) const FQueryResult& QueryResult, FString ColumnName);

	/**
	 * Gets the current date.
	 * @return The current date.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Date", meta = (CompactNodeTitle = "NOW", BlueprintAutocast))
	static UPARAM(DisplayName = "Now") FDatabaseDate Date_Now() { return FDatabaseDate::Now(); }
	
	/**
	 * Gets the current timestamp.
	 * @return The current timestamp.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Date", meta = (CompactNodeTitle = "NOW", BlueprintAutocast))
	static UPARAM(DisplayName = "Now") FDatabaseTimestamp Timestamp_Now() { return FDatabaseTimestamp::Now(); }


	/**
	 * Gets the number of rows affected by the query.
	 * @param QueryResult The query result to get the affected rows from.
	 * @return The number of rows affected by the query.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Database|Query")
	static UPARAM(DisplayName = "Affected Rows") int64 GetAffectedRows(UPARAM(ref) const FQueryResult& QueryResult);
	
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams  (FPoolDynMultCallback, UDatabasePool*, Pool, EDatabaseError, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams  (FPoolQueryDynMultCallback, const FQueryResult&, Result, EDatabaseError, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams (FPoolReconnectedDynMultCallback, int32, Reconnected, int32, Skipped, int32, Failed, EDatabaseError, Error);

UCLASS()
class UCreatePoolProxy final : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FPoolDynMultCallback OnPoolCreated;

	UPROPERTY(BlueprintAssignable)
	FPoolDynMultCallback OnError;

public:

	/**
	 * Creates a new pool asynchronously.
	 * @param DriverName	The driver to use, previously installed on your machine.
	 * @param Username		The username used to connect to your database.
	 * @param Password		The password used to connect to your database. Leave empty for none.
	 * @param Server		The URL where your database is.
	 * @param Port			The port to access the database on your server.
	 * @param Database		The name of the database to access.
	 * @param PoolSize		The size of the pool to create (i.e. the number of possible simultaneous connections).
	*/
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Database|Pool")
	static UCreatePoolProxy* CreatePool
	(
		const FString& DriverName,
		const FString& Username,
		const FString& Password,
		const FString& Server,
		const int32 Port		= 3306,
		const FString& Database = TEXT(""),
		const int32 PoolSize    = 5
	);

	virtual void Activate();

private:
	UFUNCTION()
	void OnTaskFinished(EDatabaseError Error, UDatabasePool* Pool);

private:
	FString DriverName;
	FString Username;
	FString Password;
	FString Server;
	int32 Port;
	FString Database;
	int32 PoolSize;
};

UCLASS()
class UQueryPoolProxy final : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FPoolQueryDynMultCallback Done;

	UPROPERTY(BlueprintAssignable)
	FPoolQueryDynMultCallback Failed;

public:
	/**
	 * Query the database.
	 * @param Query The query string.
	 * @param Parameters The query parameters inserted into the query.
	*/
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", AutoCreateRefTerm = "Parameters"), Category = "Database|Pool")
	static UQueryPoolProxy* Query(UDatabasePool* Pool, const FString& Query, TArray<FDatabaseValue> Parameters);

	virtual void Activate();

private:
	UFUNCTION()
	void OnTaskOver(EDatabaseError Error, const FQueryResult& Result);

private:
	UPROPERTY()
	UDatabasePool* Pool;

	FString QueryStr;
	TArray<FDatabaseValue> Parameters;
};

UCLASS()
class UReconnectPoolProxy final : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FPoolReconnectedDynMultCallback Done;

	UPROPERTY(BlueprintAssignable)
	FPoolReconnectedDynMultCallback Failed;

public:

	/**
	 * Reconnect all conections. Connections currently used will be skipped.
	 * @pram Timeout The connection timeout.
	 * @param Callback Called when all connections have been reconnected.
	*/
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", AutoCreateRefTerm = "Parameters"), Category = "Database|Pool")
	static UReconnectPoolProxy* Reconnect(UDatabasePool* Pool, const int32 Timeout = 0);

	virtual void Activate();

private:
	UFUNCTION()
	void OnTaskOver(EDatabaseError Error, int32 Reconnected, int32 Skipped, int32 Fail);

private:
	UPROPERTY()
	UDatabasePool* Pool;

	int32 Timeout;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDynMultRow, const TArray<FDatabaseValue>&, Row);

UCLASS()
class UForEachQueryRowProxy final : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;

	/**
	 * Iterates over the result.
	 * @param Result The result to iterate over.
	*/
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", AutoCreateRefTerm = ""), Category = "Database|Pool")
	static UForEachQueryRowProxy* ForEachRow(const FQueryResult& Result);

	/**
	 * This pin is fired once for every row in the result.
	*/
	UPROPERTY(BlueprintAssignable)
	FDynMultRow LoopBody;

private:
	FQueryResult Result;
};
