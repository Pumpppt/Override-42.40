#pragma once

inline UEngine*& GEngine = *reinterpret_cast<UEngine**>(GetImageBase() + 0x1B098778); // GEngine

class UnrealEngineHooks
{
public:
	static bool CreateNamedNetDriver(UWorld* World, FName NetDriverName, FName NetDriverDefinition);
	static UNetDriver* FindNamedNetDriver(UWorld* World, FName NetDriverName);
};
