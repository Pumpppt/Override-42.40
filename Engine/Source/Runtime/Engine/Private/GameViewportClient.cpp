#include "pch.h"
#include "Engine/Source/Runtime/Engine/Public/GameViewportClient.h"
#include "Engine/Source/Runtime/Engine/Public/UnrealEngine.h"

void GameViewportClient::ConstructConsole()
{
	UGameViewportClient* Viewport = GEngine ? GEngine->GameViewport : nullptr;
	if (!Viewport)
	{
		UE_LOG(LogOverride, Error, TEXT("No game viewport to construct the console on"));
		return;
	}

	if (Viewport->ViewportConsole)
	{
		UE_LOG(LogOverride, Log, TEXT("%hs already has console %hs"), Viewport->GetName().c_str(), Viewport->ViewportConsole->GetName().c_str());
		return;
	}

	UObject* Console = UGameplayStatics::SpawnObject(GEngine->ConsoleClass.Get(), Viewport);
	if (!Console)
	{
		UE_LOG(LogOverride, Error, TEXT("Failed to construct %hs on %hs"), GEngine->ConsoleClass.Get() ? GEngine->ConsoleClass.Get()->GetName().c_str() : "a null ConsoleClass", Viewport->GetName().c_str());
		return;
	}

	Viewport->ViewportConsole = static_cast<UConsole*>(Console);

	const TArray<FKey>& ConsoleKeys = UInputSettings::GetDefaultObj()->ConsoleKeys;
	std::string KeyNames;
	for (int32 i = 0; i < ConsoleKeys.Num(); i++)
	{
		KeyNames += (i ? ", " : "") + ConsoleKeys[i].KeyName.ToString();
	}

	if (KeyNames.empty())
	{
		UE_LOG(LogOverride, Warning, TEXT("Constructed %hs on %hs, but InputSettings has no ConsoleKeys bound"), Console->GetName().c_str(), Viewport->GetName().c_str());
		return;
	}

	UE_LOG(LogOverride, Display, TEXT("Constructed %hs on %hs, open it with %hs"), Console->GetName().c_str(), Viewport->GetName().c_str(), KeyNames.c_str());
}
