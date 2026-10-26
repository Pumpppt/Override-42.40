#pragma once

#include "Engine/Source/Runtime/Engine/Public/NetDriver.h"

enum class ELevelCollectionType : uint8
{
	DynamicSourceLevels,
	DynamicDuplicatedLevels,
	StaticLevels
};

class WorldHooks
{
public:
	static void Init();
	static void OnFrontendReady(UWorld* World);
	static bool Listen(UWorld* World);

private:
	static inline std::wstring ServerMapName;
	static inline std::wstring ServerMapRoot;
	static inline std::atomic<bool> bFrontendReady = false;
	static inline std::atomic<bool> bServerMapMounted = false;
	static inline std::atomic<bool> bServerTravelStarted = false;

	static inline bool (*SetGameModeOriginal)(UWorld*, const FURL&);
	static bool SetGameMode(UWorld* World, const FURL& InURL);

	static inline void (*InsertMountPointOriginal)(void*, void*, const FString&, const FString&);
	static void InsertMountPoint(void* PackagePaths, void* OutDelegates, const FString& RootPath, const FString& ContentPath);

	static inline ENetMode (*AttemptDeriveFromURLOriginal)(UWorld*);
	static ENetMode AttemptDeriveFromURL(UWorld* World);

	static void TryStartServerTravel();
	static void SpawnServerStreamingLevelsVisibility(UWorld* World);
	static void RouteFilteredOutClasses();
	static void PatchInlinedNetModes();
};
