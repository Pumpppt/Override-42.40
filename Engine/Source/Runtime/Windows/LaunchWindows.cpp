#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/Windows/WindowsPlatformMisc.h"
#include "Engine/Source/Runtime/Engine/Public/NetDriver.h"
#include "Engine/Source/Runtime/Engine/Public/World.h"
#include "FortniteGame/Source/Public/FortGameSession.h"
#include "FortniteGame/Source/Public/FortGameState.h"
#include "FortniteGame/Source/Public/FortWeaponSoundLibraryComponent.h"
#include "FortniteGame/Plugins/ForEngine/OnlineGameplayFramework/Source/McpProfileSys/Public/McpProfileGroup.h"
#include "FortniteGame/Source/Public/Athena/FortGameModeAthena.h"
#include "FortniteGame/Source/Public/Player/FortPlayerControllerAthena.h"
#include "FortniteGame/Source/Public/Player/FortPlayerController.h"

DWORD WINAPI Main(LPVOID)
{
	InitLogging(TEXT("Server"));
	SetStatus(L"Waiting for the Frontend");

	WorldHooks::Init();
	RedirectEngineLog();
	WindowsPlatformMiscHooks::Init();
	FortGameSessionHooks::Init();
	FortGameStateHooks::Init();
	FortWeaponSoundLibraryComponentHooks::Init();
	McpProfileGroupHooks::Init();
	NetDriverHooks::Init();

	UWorld* World = UWorld::GetWorld();
	while (!World || !World->AuthorityGameMode)
	{
		Sleep(100);
		World = UWorld::GetWorld();
	}

	FortGameModeAthenaHooks::Init();
	FortPlayerControllerAthenaHooks::Init();
	FortPlayerControllerHooks::Init();

	UE_LOG(LogOverride, Display, TEXT("World %hs is up with %hs"), World->GetName().c_str(), World->AuthorityGameMode->GetName().c_str());
	UKismetSystemLibrary::ExecuteConsoleCommand(World, FString(L"net.AllowEncryption 0"), nullptr);
	WorldHooks::OnFrontendReady(World);

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		CreateThread(nullptr, 0, Main, nullptr, 0, nullptr);
	}

	return TRUE;
}
