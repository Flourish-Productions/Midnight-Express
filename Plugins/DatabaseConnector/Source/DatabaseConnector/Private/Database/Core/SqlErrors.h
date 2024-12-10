// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include <string>

#include "Database/Errors.h"

/* SQL error codes */
#define SQL_ERROR_CONNECTION_NOT_OPEN		"0800"

namespace NSqlErrors
{
	EDatabaseError ConvertState(const std::string State);
};


