#include "pch.h"
#include "Engine/Source/Runtime/Engine/Public/UnrealEngine.h"

bool UnrealEngineHooks::CreateNamedNetDriver(UWorld* World, FName NetDriverName, FName NetDriverDefinition)
{
	static const auto Function = reinterpret_cast<bool (*)(UEngine*, UWorld*, FName, FName)>(GetImageBase() + 0xB9F687A); // UEngine::CreateNamedNetDriver
	return Function(GEngine, World, NetDriverName, NetDriverDefinition);
}

UNetDriver* UnrealEngineHooks::FindNamedNetDriver(UWorld* World, FName NetDriverName)
{
	static const auto Function = reinterpret_cast<UNetDriver* (*)(UEngine*, UWorld*, FName)>(GetImageBase() + 0x45E5AC2); // UEngine::FindNamedNetDriver
	return Function(GEngine, World, NetDriverName);
}
