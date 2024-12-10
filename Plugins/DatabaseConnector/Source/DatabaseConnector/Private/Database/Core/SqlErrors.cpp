// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "Database/Core/SqlErrors.h"

#include "DatabaseConnectorModule.h"

EDatabaseError NSqlErrors::ConvertState(const std::string State)
{
	if (State == SQL_ERROR_CONNECTION_NOT_OPEN)
	{
		return EDatabaseError::ConnectionClosed;
	}

	UE_LOG(LogDatabaseConnector, Warning, TEXT("Unknown type: %s"), UTF8_TO_TCHAR(State.c_str()));

	return EDatabaseError::QueryFailed;
}
