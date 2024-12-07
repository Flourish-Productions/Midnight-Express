// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "Database/QueryResult.h"

#include "DatabaseConnectorModule.h"

#include <sstream>

struct FQueryResultInternal
{
public:
	FQueryResultInternal() = default;
	FQueryResultInternal(uint64 InAffectedRows)
		: AffectedRows(InAffectedRows) 
	{}
	FQueryResultInternal(TArray<FString>&& InHeaders, TArray64<TArray<FDatabaseValue>>&& InValues, TArray<FColumnMetadata>&& InMetadata, uint64 InAffectedRows)
		: AffectedRows(InAffectedRows)
		, Headers(MoveTemp(InHeaders))
		, Metadata(MoveTemp(InMetadata))
		, Values(MoveTemp(InValues))
	{}

	uint64 AffectedRows;
	TArray<FString> Headers;
	TArray<FColumnMetadata> Metadata;
	TArray64<TArray<FDatabaseValue>> Values;
};

FQueryResult::FQueryResult(TArray<FString> Headers, TArray64<TArray<FDatabaseValue>> Values, TArray<FColumnMetadata> Metadata, uint64 AffectedRows)
	: Internal(MakeShared<FQueryResultInternal, ESPMode::ThreadSafe>(
		MoveTemp(Headers), MoveTemp(Values), MoveTemp(Metadata), AffectedRows))
{
}

FQueryResult::FQueryResult()
	: Internal(MakeShared<FQueryResultInternal, ESPMode::ThreadSafe>())
{}

FQueryResult::~FQueryResult() = default;

FQueryResult::FQueryResult(uint64 AffectedRows)
	:	Internal(MakeShared<FQueryResultInternal, ESPMode::ThreadSafe>(AffectedRows))
{

}

FQueryResult::FQueryResult(const FQueryResult & Other)
	: Internal(Other.Internal)
{
}

FQueryResult::FQueryResult(FQueryResult && Other)
	: Internal(MoveTemp(Other.Internal))
{
}

FQueryResult& FQueryResult::operator=(const FQueryResult & Other)
{
	Internal = Other.Internal;
	return *this;
}

FQueryResult& FQueryResult::operator=(FQueryResult && Other)
{
	Internal = MoveTemp(Other.Internal);
	return *this;
}

int64 FQueryResult::GetAffectedRows() const
{
	return (int64)Internal->AffectedRows;
}

const TArray<FString>& FQueryResult::GetColumns() const
{
	return Internal->Headers;
}

const TArray<FDatabaseValue>* FQueryResult::GetRow(const int64 RowIndex) const
{
	return Internal->Values.IsValidIndex(RowIndex) ? &Internal->Values[RowIndex] : nullptr;
}

int64 FQueryResult::GetRowCount() const
{
	return Internal->Values.Num();
}

int32 FQueryResult::GetColumnCount() const
{
	return Internal->Headers.Num();
}

const FDatabaseValue& FQueryResult::Get(const FString & ColumnName, const int64 RowIndex) const
{
	int32 ColumnIndex = -1;
	if (Internal->Headers.Find(ColumnName, ColumnIndex))
	{
		return Get(ColumnIndex, RowIndex);
	}

	UE_LOG(LogDatabaseConnector, Warning, TEXT("Column `%s` not found."), *ColumnName);

	return FDatabaseValue::NullValue;
}

const FDatabaseValue& FQueryResult::Get(const int32 & ColumnIndex, const int64 RowIndex) const
{
	if (Internal->Values.IsValidIndex(RowIndex))
	{
		const TArray<FDatabaseValue>& Row = Internal->Values[RowIndex];
		if (Row.IsValidIndex(ColumnIndex))
		{
			return Row[ColumnIndex];
		}
	}

	UE_LOG(LogDatabaseConnector, Warning, TEXT("Failed to find column %d row %d. Dataset is of size %d/%d."),
		ColumnIndex, RowIndex, Internal->Headers.Num(), Internal->Values.Num());

	return FDatabaseValue::NullValue;
}

const TArray<FColumnMetadata>& FQueryResult::GetColumnsMetadata() const
{
	return Internal->Metadata;
}

const FColumnMetadata* FQueryResult::GetColumnMetadata(const int32 ColumnIndex) const
{
	if (Internal->Metadata.IsValidIndex(ColumnIndex))
	{
		return &Internal->Metadata[ColumnIndex];
	}

	return nullptr;
}

const FColumnMetadata* FQueryResult::GetColumnMetadata(const FString ColumnName) const
{
	const int32 Index = Internal->Headers.Find(ColumnName);

	return GetColumnMetadata(Index);
}

void FQueryResult::LogDump() const
{
	const TArray<FString>&					Headers		= Internal->Headers;
	const TArray64<TArray<FDatabaseValue>>& Values		= Internal->Values;
	const TArray<FColumnMetadata>&			Metadata	= Internal->Metadata;

	if (Headers.Num() <= 0)
	{
		UE_LOG(LogDatabaseConnector, Log, TEXT(" -> Start FQueryResult dump: Result is empty."));
		return;
	}

	const auto GetColumnSize = [&](const int32 Index) -> int32
	{
		const int32 Size = Metadata[Index].Size;
		// Trim large data.
		return Size > 55 ? 20 : Size;
	};

	std::string Separator;

	{
		int32 TableLength = 2;
		for (int32 i = 0; i < Metadata.Num(); ++i)
		{
			TableLength += 3 + GetColumnSize(i);
		}

		Separator.resize(TableLength, '-');
		Separator[0] = ' ';
	}

	std::stringstream Output;

	if (Values.Num() > 0)
	{
		{
			Output << Separator << "\n | ";

			for (int32 i = 0; i < Headers.Num(); ++i)
			{
				const int32 Size = GetColumnSize(i);
				Output << TCHAR_TO_UTF8(*Headers[i].Left(Size).RightPad(Size)) << " | ";
			}

			Output << "\n" << Separator << "\n";
		}

		for (const TArray<FDatabaseValue>& Row : Values)
		{
			Output << " | ";
			for (int32 i = 0; i < Row.Num(); ++i)
			{
				const int32 Size = GetColumnSize(i);
				Output << TCHAR_TO_UTF8(*Row[i].ToString(false).Left(Size).RightPad(Size)) << " | ";
			}
			Output << "\n";
		}

		Output << Separator;
	}

	UE_LOG(LogDatabaseConnector, Log,
		TEXT(" -> Start FQueryResult dump (%d rows, %d columns):\n")
		TEXT(" \n")
		TEXT("%s\n \n"), Values.Num(), Headers.Num(), UTF8_TO_TCHAR(Output.str().c_str())
	);

}
