// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include <variant>
#include "Database/Value.h"

struct FNullValue {};

using FValue = std::variant
<
	FNullValue, bool, uint8, int32, int64, double, FString, FDatabaseTimestamp, FDatabaseDate
>;

class FDatabaseValueInternal
{
public:
	inline FValue& Get() { return Value; }

private:
	FValue Value;
};

