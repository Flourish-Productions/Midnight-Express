// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DatabaseNodes.h"

FDatabaseValue UDatabaseConnectorBlueprintLibrary::GetByColumnName(const FQueryResult& QueryResult, const FString& Column, int64 RowIndex)
{
	return QueryResult.Get(Column, RowIndex);
}

FDatabaseValue UDatabaseConnectorBlueprintLibrary::GetByColumnIndex(UPARAM(ref) const FQueryResult& QueryResult, int32 ColumnIndex, int64 RowIndex)
{
	return QueryResult.Get(ColumnIndex, RowIndex);
}

TArray<FString> UDatabaseConnectorBlueprintLibrary::GetColumns(UPARAM(ref) const FQueryResult& QueryResult)
{
	return QueryResult.GetColumns();
}

int32 UDatabaseConnectorBlueprintLibrary::GetColumnCount(UPARAM(ref) const FQueryResult& QueryResult)
{
	return QueryResult.GetColumnCount();
}

int64 UDatabaseConnectorBlueprintLibrary::GetRowCount(UPARAM(ref) const FQueryResult& QueryResult)
{
	return QueryResult.GetRowCount();
}

TArray<FDatabaseValue> UDatabaseConnectorBlueprintLibrary::GetRow(UPARAM(ref) const FQueryResult& QueryResult, const int64 RowIndex)
{
	using FValueArray = TArray<FDatabaseValue>;

	const FValueArray* const Row = QueryResult.GetRow(RowIndex);

	return Row ? *Row : FValueArray();
}

TArray<FColumnMetadata> UDatabaseConnectorBlueprintLibrary::GetColumnsMetadata(const FQueryResult& QueryResult)
{
	return QueryResult.GetColumnsMetadata();
}

FColumnMetadata UDatabaseConnectorBlueprintLibrary::GetColumnMetadataByIndex(const FQueryResult& QueryResult, const int32 ColumnIndex)
{
	const FColumnMetadata* const Meta = QueryResult.GetColumnMetadata(ColumnIndex);
	return Meta ? *Meta : FColumnMetadata();
}

FColumnMetadata UDatabaseConnectorBlueprintLibrary::GetColumnMetadataByName(const FQueryResult& QueryResult, const FString ColumnName)
{
	const FColumnMetadata* const Meta = QueryResult.GetColumnMetadata(ColumnName);
	return Meta ? *Meta : FColumnMetadata();
}

int64 UDatabaseConnectorBlueprintLibrary::GetAffectedRows(UPARAM(ref) const FQueryResult& QueryResult)
{
	return QueryResult.GetAffectedRows();
}

void UDatabaseConnectorBlueprintLibrary::LogDump(UPARAM(ref) const FQueryResult& QueryResult)
{
	QueryResult.LogDump();
}

UCreatePoolProxy* UCreatePoolProxy::CreatePool(
	const FString &DriverName,
	const FString &Username,
	const FString &Password,
	const FString &Server,
	const int32 Port,
	const FString &Database,
	const int32 PoolSize
)
{
	ThisClass* const Proxy = NewObject<ThisClass>();

	Proxy->DriverName	= DriverName;
	Proxy->Username		= Username;
	Proxy->Password		= Password;
	Proxy->Server		= Server;
	Proxy->Database		= Database;
	Proxy->PoolSize		= PoolSize;
	Proxy->Port			= Port;

	return Proxy;
}

void UCreatePoolProxy::Activate()
{
	UDatabasePool::CreatePool(
		MoveTemp(DriverName), MoveTemp(Username), MoveTemp(Password), MoveTemp(Server), Port, MoveTemp(Database), PoolSize,
		FDatabasePoolCallback::CreateUObject(this, &UCreatePoolProxy::OnTaskFinished));
}

void UCreatePoolProxy::OnTaskFinished(EDatabaseError Error, UDatabasePool* Pool)
{
	(Error == EDatabaseError::None ? OnPoolCreated : OnError).Broadcast(Pool, Error);
	SetReadyToDestroy();
}


UQueryPoolProxy* UQueryPoolProxy::Query(UDatabasePool* Pool, const FString& Query, TArray<FDatabaseValue> Parameters)
{
	ThisClass* const Proxy = NewObject<ThisClass>();

	Proxy->Pool			= Pool;
	Proxy->QueryStr		= Query;
	Proxy->Parameters	= MoveTemp(Parameters);

	return Proxy;
}

void UQueryPoolProxy::Activate()
{
	if (!Pool)
	{
		OnTaskOver(EDatabaseError::FailedToOpenConnection, {});
		return;
	}

	Pool->Query(MoveTemp(QueryStr), MoveTemp(Parameters), FDatabaseQueryCallback::CreateUObject(this, &UQueryPoolProxy::OnTaskOver));
}

void UQueryPoolProxy::OnTaskOver(EDatabaseError Error, const FQueryResult& Result)
{
	(Error == EDatabaseError::None ? Done : Failed).Broadcast(Result, Error);
	SetReadyToDestroy();
}

UReconnectPoolProxy* UReconnectPoolProxy::Reconnect(UDatabasePool* Pool, const int32 Timeout)
{
	ThisClass* const Proxy = NewObject<ThisClass>();

	Proxy->Pool		= Pool;
	Proxy->Timeout	= Timeout;

	return Proxy;
}

void UReconnectPoolProxy::Activate()
{
	if (!Pool)
	{
		OnTaskOver(EDatabaseError::FailedToOpenConnection, 0, 0, 0);
		return;
	}

	Pool->Reconnect(Timeout, FPoolReconnectCallback::CreateUObject(this, &ThisClass::OnTaskOver));
}

void UReconnectPoolProxy::OnTaskOver(EDatabaseError Error, int32 Reconnected, int32 Skipped, int32 Fail)
{
	(Error == EDatabaseError::None ? Done : Failed).Broadcast(Reconnected, Skipped, Fail, Error);

	SetReadyToDestroy(); 
}

UForEachQueryRowProxy* UForEachQueryRowProxy::ForEachRow(const FQueryResult& Result)
{
	ThisClass* const Proxy = NewObject<ThisClass>();

	Proxy->Result = Result;

	return Proxy;
}

void UForEachQueryRowProxy::Activate()
{
	if (LoopBody.IsBound())
	{
		const int64 RowCount = Result.GetRowCount();
		for (int64 i = 0; i < RowCount; ++i)
		{
			const TArray<FDatabaseValue>* const Row = Result.GetRow(i);

			LoopBody.Broadcast(*Row);
		}
	}

	SetReadyToDestroy();
}
