// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Database/Core/ThreadPool.h"
#include "Database/Core/NanoDefinitions.h"
#include "Database/Errors.h"
#include "Database/Value.h"
#include "Database/QueryResult.h"
#include "Pool.generated.h"

class UDatabasePool;

DECLARE_DELEGATE_TwoParams (FDatabasePoolCallback,	EDatabaseError /* Error */, UDatabasePool* /* Pool */);
DECLARE_DELEGATE_TwoParams (FDatabaseQueryCallback,	EDatabaseError /* Error */, const FQueryResult& /* Results */);
DECLARE_DELEGATE_FourParams(FPoolReconnectCallback,	EDatabaseError /* Error */, int32 /* ReconnectedCount */, int32 /* SkippedCount */, int32 /* FailedCount */);

DECLARE_DYNAMIC_DELEGATE_TwoParams(FDatabasePoolDelegate,	EDatabaseError, Error, UDatabasePool*, Pool);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FDatabaseQueryDelegate,	EDatabaseError, Error, const FQueryResult&, Results);

/**
 * A pool containing clients used to communicate via ODBC to a database.
*/
UCLASS(BlueprintType)
class DATABASECONNECTOR_API UDatabasePool : public UObject
{
	GENERATED_BODY()
private:

	/**
	 * The size of our threads' stack.
	*/
	static constexpr int32 ThreadStackSize = 32768u;

	/**
	 * Priority of the threads in our pool.
	*/
	static constexpr EThreadPriority ThreadPriority = EThreadPriority::TPri_Normal;

	/**
	 * Name of the threads in our pool.
	*/
	static FString ThreadName;

public:
	/**
	 * Don't use NewObject on this class. RAII is not implemented in the constructor
	 * due to Blueprints limitations. Make sure to call `CreatePool()` instead.
	*/
	 UDatabasePool();
	~UDatabasePool();

	/**
	 * Creates a new pool asynchronously.
	 * @param DriverName	The driver to use, previously installed on your machine.
	 * @param Username		The username used to connect to your database.
	 * @param Password		The password used to connect to your database. Leave empty for none.
	 * @param Server		The URL where your database is.
	 * @param Port			The port to access the database on your server.
	 * @param Database		The name of the database to access.
	 * @param PoolSize		The size of the pool.
	 * @param Callback		Called when the pool has been created.
	*/
	static void CreatePool(const FString& DriverName, const FString& Username, const FString& Password, const FString& Server, const int32 Port, const FString& Database, const int32 PoolSize, FDatabasePoolCallback Callback);

	/**
	 * Creates a new pool synchronously. 
	 * /!\ The application will block until all the connections are established /!\
	 * Use `CreatePool()` to avoid blocking the Game Thread.
	 * @param DriverName	The driver to use, previously installed on your machine.
	 * @param Username		The username used to connect to your database.
	 * @param Password		The password used to connect to your database. Leave empty for none.
	 * @param Server		The URL where your database is.
	 * @param Port			The port to access the database on your server.
	 * @param Database		The name of the database to access.
	 * @param PoolSize		The size of the pool.
	*/
	UFUNCTION(BlueprintCallable, Category = "Database|Pool")
	static UPARAM(DisplayName = "Pool") UDatabasePool* CreatePoolSync(const FString& DriverName, const FString& Username, const FString& Password, const FString& Server, const int32 Port, const FString& Database, const int32 PoolSize, EDatabaseError& OutError);

	/**
	 * Creates a new pool asynchronously.
	 * @param DriverName	The driver to use, previously installed on your machine.
	 * @param Username		The username used to connect to your database.
	 * @param Password		The password used to connect to your database. Leave empty for none.
	 * @param Server		The URL where your database is.
	 * @param Port			The port to access the database on your server.
	 * @param Database		The name of the database to access.
	 * @param PoolSize		The size of the pool.
	 * @param Callback		Called when the pool has been created.
	*/
	UFUNCTION(BlueprintCallable, Category = "Database|Pool", Meta = (DisplayName = "Create Pool with Callback"))
	static void Blueprint_CreatePool(const FString& DriverName, const FString& Username, const FString& Password, const FString& Server, const int32 Port, const FString& Database, const int32 PoolSize, FDatabasePoolDelegate Callback);

	/**
	 * Query the database.
	 * @param Query The query string.
	 * @param Parameters The query parameters inserted into the query.
	*/
	void Query(FString Query, TArray<FDatabaseValue> Parameters, FDatabaseQueryCallback Callback);

	/**
	 * Query the database synchronously.
	 * /!\ The application will block until the query completes /!\
	 * Use `Query()` to avoid blocking the Game Thread.
	 * @param Query The query string.
	 * @param Parameters The query parameters inserted into the query.
	*/
	UFUNCTION(BlueprintCallable, Category = "Database|Pool", Meta = (AutoCreateRefTerm = "Parameters"))
	UPARAM(DisplayName = "Result") FQueryResult QuerySync(FString Query, TArray<FDatabaseValue> Parameters, EDatabaseError& OutError);

	/**
	 * Query the database.
	 * @param Query The query string.
	 * @param Parameters The query parameters inserted into the query.
	*/
	UFUNCTION(BlueprintCallable, Category = "Database|Pool", Meta = (DisplayName = "Query with Callback"))
	void Blueprint_Query(FString Query, TArray<FDatabaseValue> Parameters, FDatabaseQueryDelegate Callback);

	/**
	 * Reconnect all conections. Connections currently used will be skipped.
	 * @pram Timeout The connection timeout.
	 * @param Callback Called when all connections have been reconnected.
	*/
	void Reconnect(const int32 Timeout = 0, FPoolReconnectCallback Callback = FPoolReconnectCallback());

private:
	/**
	 * The thread pool this connection pool is going to use.
	 * We have to use a thread pool as our ODBC driver provides
	 * only a blocking synchronous interface.
	*/
	FThreadPoolPtr ThreadPool;

	/**
	 * The database connection pool.
	 * Must be thread-safe as it travels across threads.
	*/
	FConnectionPoolPtr ConnectionPool;

private:
	/**
	 * The connection DSN of this pool.
	*/
	TSharedPtr<const FString, ESPMode::ThreadSafe> ConnectionDsn;
};

