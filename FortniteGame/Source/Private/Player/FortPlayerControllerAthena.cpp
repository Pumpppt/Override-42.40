#include "pch.h"
#include "FortniteGame/Source/Public/Player/FortPlayerControllerAthena.h"
#include "FortniteGame/Source/Public/Athena/FortGameModeAthena.h"



void FortPlayerControllerAthenaHooks::RestartPlayer(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* PlayerController)
{
	if (!PlayerController->bHasClientFinishedLoading || PlayerController->Pawn)
	{
		return;
	}

	GameMode->RestartPlayer(PlayerController);

	if (!PlayerController->Pawn)
	{
		UE_LOG(LogOverrideGame, Error, TEXT("RestartPlayer did not give %hs a pawn"), PlayerController->GetName().c_str());
		return;
	}

	UE_LOG(LogOverrideGame, Display, TEXT("Restarted %hs with %hs"), PlayerController->GetName().c_str(), PlayerController->Pawn->GetName().c_str());
}

void FortPlayerControllerAthenaHooks::ServerAcknowledgePossession(AFortPlayerControllerAthena* PlayerController, APawn* Pawn)
{
	PlayerController->AcknowledgedPawn = Pawn;
	UE_LOG(LogOverrideGame, Log, TEXT("%hs acknowledged possession of %hs"), PlayerController->GetName().c_str(), Pawn ? Pawn->GetName().c_str() : "None");
}

void FortPlayerControllerAthenaHooks::ServerSetClientHasFinishedLoading(AFortPlayerControllerAthena* PlayerController, bool bInHasFinishedLoading)
{
	PlayerController->bHasClientFinishedLoading = bInHasFinishedLoading;
	UE_LOG(LogOverrideGame, Display, TEXT("%hs client finished loading: %hs"), PlayerController->GetName().c_str(), bInHasFinishedLoading ? "true" : "false");

	if (!bInHasFinishedLoading)
	{
		return;
	}

	UWorld* World = UWorld::GetWorld();
	AGameModeBase* GameMode = World ? World->AuthorityGameMode : nullptr;
	if (!GameMode || !GameMode->IsA(AFortGameModeAthena::StaticClass()))
	{
		UE_LOG(LogOverrideGame, Warning, TEXT("No Athena game mode to restart %hs with"), PlayerController->GetName().c_str());
		return;
	}

	AFortGameModeAthena* AthenaGameMode = static_cast<AFortGameModeAthena*>(GameMode);
	FortGameModeAthenaHooks::UpdatePlayersLoaded(AthenaGameMode);

	if (!AthenaGameMode->HasMatchStarted())
	{
		UE_LOG(LogOverrideGame, Log, TEXT("%hs will be restarted when the match starts"), PlayerController->GetName().c_str());
		return;
	}

	RestartPlayer(AthenaGameMode, PlayerController);
}

void FortPlayerControllerAthenaHooks::Init()
{
	AFortPlayerControllerAthena* PlayerController = AFortPlayerControllerAthena::GetDefaultObj();

	Runtime::VFTHook(PlayerController, 0x12B, ServerAcknowledgePossession, &ServerAcknowledgePossessionOriginal, TEXT("APlayerController::ServerAcknowledgePossession_Implementation")); // APlayerController::ServerAcknowledgePossession_Implementation
	Runtime::VFTHook(PlayerController, 0x27D, ServerSetClientHasFinishedLoading, &ServerSetClientHasFinishedLoadingOriginal, TEXT("AFortPlayerController::ServerSetClientHasFinishedLoading_Implementation")); // AFortPlayerController::ServerSetClientHasFinishedLoading_Implementation
}