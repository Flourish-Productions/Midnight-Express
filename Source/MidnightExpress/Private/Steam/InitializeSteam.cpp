// Fill out your copyright notice in the Description page of Project Settings.


#include "Steam/InitializeSteam.h"
//#include <steam/steam_api.h>

InitializeSteam::InitializeSteam()
{
}

bool InitializeSteam::_InitializeSteam()
{
	// if (!SteamAPI_Init())
	// {
	// 	std::cerr << "Steam API initialization failed!" << std::endl;
	// 	return false;
	// }
	// return true;
	return true;
}

void InitializeSteam::_ShutdownSteam()
{
	//SteamAPI_Shutdown();
}

