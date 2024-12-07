// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if PLATFORM_WINDOWS
#	include "Windows/AllowWindowsPlatformTypes.h"
#endif 

THIRD_PARTY_INCLUDES_START
#	include <nanodbc/nanodbc.h>
THIRD_PARTY_INCLUDES_END

#if PLATFORM_WINDOWS
#	include "Windows/HideWindowsPlatformTypes.h"
#endif // PLATFORM_WINDOWS

#include <list>
#include <mutex>
#include <condition_variable>

#include "Database/Value.h"
#include "Database/Errors.h"
#include "Database/QueryResult.h"

class FConnection
{
public:
	FConnection(const FString& Url);

	FConnection(const FConnection&) = delete;
	FConnection& operator=(const FConnection&) = delete;

	bool TryAcquire();
	void Lock();
	void Unlock();

	FQueryResult Query(const FString& Sql, const FString& Dsn, const TArray<FDatabaseValue> & Parameters, EDatabaseError& OutError, int32 RecursiveCount = 0);

	bool Connect(const FString& Dsn, const int32 Timeout = 0);

private:
	nanodbc::connection Connection;
	TAtomic<bool> bIsAvailable;
};

class FConnectionPool
{
private:
	friend class FConnectionHandle;
public:
	 FConnectionPool() = default;
	~FConnectionPool();

	FConnectionPool(const FConnectionPool&) = delete;
	FConnectionPool operator=(const FConnectionPool&) = delete;

	EDatabaseError Create(const FString& Url, const int32 Count);

	int32 GetPoolSize() const;

	void Reconnect(const FString& Dsn, const int32 Timeout, int32& Reconnected, int32& Skipped, int32& Failed);

private:
	FConnection& AcquireOne();

private:
	TArray<TUniquePtr<FConnection>> Connections;

	FCriticalSection Section;

	std::mutex Sleeper;
	std::condition_variable SleeperCondition;
};

class FConnectionHandle
{
public:
	FConnectionHandle(FConnectionPool& InPool);

	FConnectionHandle(const FConnectionHandle&) = delete;
	FConnectionHandle& operator=(const FConnectionHandle&) = delete;

	FConnection& Get();

	~FConnectionHandle();

private:
	FConnection* Connection;
	FConnectionPool* const Pool;
};

