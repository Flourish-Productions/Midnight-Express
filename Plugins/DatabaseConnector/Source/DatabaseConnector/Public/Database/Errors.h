// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Errors.generated.h"


UENUM(BlueprintType)
enum class EDatabaseError : uint8
{
	None,
	InvalidPoolSize,
	FailedToOpenConnection,
	QueryFailed,
	ConnectionClosed
};


