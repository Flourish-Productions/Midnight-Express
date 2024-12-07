// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Database/Value.h"
#include "Database/Errors.h"
#include "Database/Core/NanoDefinitions.h"
#include "Database/Core/ThreadPool.h"
#include "Misc/IQueuedWork.h"

namespace NDatabasePoolThread
{
	void AsyncTask(FQueuedThreadPool* const Pool, TUniqueFunction<void()> Function);
};
