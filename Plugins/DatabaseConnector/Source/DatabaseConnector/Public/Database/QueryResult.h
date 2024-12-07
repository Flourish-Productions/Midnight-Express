// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Database/Value.h"
#include "QueryResult.generated.h"

USTRUCT(BlueprintType)
struct DATABASECONNECTOR_API FColumnMetadata
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database|Metadata")
	int32 DecimalDigits = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database|Metadata")
	FString DataTypeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database|Metadata")
	int32 Size = 0;
};

/**
 * The query of a result.
 * Holds a pointer to a shared dataset. Cheap to copy and Thread-safe.
*/
USTRUCT(BlueprintType)
struct DATABASECONNECTOR_API FQueryResult
{
	GENERATED_BODY()

public:
	FQueryResult();
	~FQueryResult();

	FQueryResult(uint64 AffectedRows);

	/* Initializer constructor. Create internally a shared result. */
	FQueryResult(TArray<FString> Headers, TArray64<TArray<FDatabaseValue>> Values, TArray<FColumnMetadata> Metadata, uint64 AffectedRows);

	/* Copy constructor. It is cheap, whatever the resultset size is. */
	FQueryResult(const FQueryResult&);

	/* Copy operator. It is cheap, whatever the resultset size is. */
	FQueryResult& operator=(const FQueryResult&);

	FQueryResult(FQueryResult&&);
	FQueryResult& operator=(FQueryResult&&);

	/**
	 * Gets a value by column name. 
	 * Access cost is O(ColumnCount) so it is O(1) on a fixed table.
	 * @param ColumnName The column to get the value from.
	 * @param RowIndex The row index to get the value from.
	 * @return The value at the specified location.
	*/
	const FDatabaseValue& Get(const FString& ColumnName, const int64 RowIndex) const;

	/**
	 * Gets a value by column index.
	 * Access cost is O(1) (faster than get by column name).
	 * @param ColumnName The column to get the value from.
	 * @param RowIndex The row index to get the value from.
	 * @return The value at the specified location.
	*/
	const FDatabaseValue& Get(const int32& ColumnIndex, const int64 RowIndex) const;

	/**
	 * Gets the columns.
	 * @return The columns.
	*/
	const TArray<FString>& GetColumns() const;

	/**
	 * Gets a row.
	 * Access cost is O(1).
	 * @return The row or nullptr if invalid index.
	*/
	const TArray<FDatabaseValue>* GetRow(const int64 RowIndex) const;

	/**
	 * Dumps the data nicely in the output log.
	*/
	void LogDump() const;

	/**
	 * Gets the number of rows.
	 * @return The number of rows.
	*/
	int64 GetRowCount() const;

	/**
	 * Gets the number of columns.
	 * @return The number of columns.
	*/
	int32 GetColumnCount() const;

	/**
	 * Gets the columns' metadata.
	 * @return The columns' metadata.
	*/
	const TArray<FColumnMetadata>& GetColumnsMetadata() const;

	/**
	 * Gets a column's metadata.
	 * @param ColumnIndex The column index to get.
	 * @return Gets a column's metadata or nullptr if invalid index.
	*/
	const FColumnMetadata* GetColumnMetadata(const int32 ColumnIndex) const;

	/**
	 * Gets a column's metadata.
	 * @param ColumnName The column's name to get.
	 * @return Gets a column's metadata or nullptr if not found.
	*/
	const FColumnMetadata* GetColumnMetadata(const FString ColumnName) const;

	/**
	 * Gets the number of rows affected by the query.
	 * @return The number of rows affected by the query.
	*/
	int64 GetAffectedRows() const;

private:
	TSharedPtr<const struct FQueryResultInternal, ESPMode::ThreadSafe> Internal;
};
