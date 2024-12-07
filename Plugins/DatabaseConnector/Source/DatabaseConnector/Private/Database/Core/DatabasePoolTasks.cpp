// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DatabasePoolTasks.h"

#if PLATFORM_WINDOWS
#	include "Windows/AllowWindowsPlatformTypes.h"
#endif // PLATFORM_WINDOWS

THIRD_PARTY_INCLUDES_START
#	include <nanodbc/nanodbc.h>
THIRD_PARTY_INCLUDES_END

#if PLATFORM_WINDOWS
#	include "Windows/HideWindowsPlatformTypes.h"
#endif // PLATFORM_WINDOWS

#include "Async/Async.h"

#include "Database/Core/DatabaseValueInternal.h"
#include "DatabaseConnectorModule.h"

class FDatabasePoolWorkBase : public IQueuedWork
{
public:
	FDatabasePoolWorkBase(TUniqueFunction<void()> InFunction)
		: Function(MoveTemp(InFunction))
	{}

	virtual void DoThreadedWork()
	{
		Function();

		delete this;
	}

	virtual void Abandon() {}

protected:
	TUniqueFunction<void()> Function;
};

void NDatabasePoolThread::AsyncTask(FQueuedThreadPool* const Pool, TUniqueFunction<void()> Function)
{
	Pool->AddQueuedWork(new FDatabasePoolWorkBase(MoveTemp(Function)));
}


