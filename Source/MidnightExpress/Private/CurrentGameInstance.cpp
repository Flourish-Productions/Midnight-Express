// Fill out your copyright notice in the Description page of Project Settings.


#include "CurrentGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"

void UCurrentGameInstance::Init()
{
	Super::Init();
    
	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get())
	{
		if (IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface())
		{
			Sessions->AddOnSessionUserInviteAcceptedDelegate_Handle(
				FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UCurrentGameInstance::OnSteamInviteAccepted)
			);
		}
	}
}

void UCurrentGameInstance::OnSteamInviteAccepted(bool bWasSuccessful, int32 ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& SearchResult)
{
	if (bWasSuccessful && SearchResult.IsValid())
	{
		IOnlineSessionPtr Sessions = IOnlineSubsystem::Get()->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->JoinSession(0, NAME_GameSession, SearchResult);
		}
	}
}
