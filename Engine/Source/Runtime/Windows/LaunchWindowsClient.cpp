#include "pch.h"
#include "Engine/Source/Runtime/Engine/Public/GameViewportClient.h"

DWORD WINAPI Main(LPVOID)
{
	InitLogging(TEXT("Client"));
	SetStatus(L"Waiting for the world");

	while (!UWorld::GetWorld())
	{
		Sleep(100);
	}

	RedirectEngineLog();

	const std::string WorldName = UWorld::GetWorld()->GetName();
	UE_LOG(LogOverride, Display, TEXT("World %hs is up"), WorldName.c_str());
	SetStatus(std::format(L"In {}", std::wstring(WorldName.begin(), WorldName.end())));

	UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), FString(L"net.AllowEncryption 0"), nullptr);
	GameViewportClient::ConstructConsole();

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
