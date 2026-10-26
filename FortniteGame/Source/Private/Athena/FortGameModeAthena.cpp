#include "pch.h"
#include "FortniteGame/Source/Public/Athena/FortGameModeAthena.h"
#include "FortniteGame/Source/Public/Player/FortPlayerControllerAthena.h"
#include "FortniteGame/Source/Public/FortInventory.h"
#include "FortniteGame/Source/Public/Abilities/FortAbilitySystemComponent.h"
#include "Engine/Source/Runtime/Engine/Public/NetDriver.h"

UFortPlaylistAthena* FortGameModeAthenaHooks::LoadPlaylist()
{
	const FSoftObjectPath Path = UKismetSystemLibrary::MakeSoftObjectPath(FString(Configuration::Playlist.c_str()));
	UObject* Object = UKismetSystemLibrary::LoadAsset_Blocking(UKismetSystemLibrary::Conv_SoftObjPathToSoftObjRef(Path));

	if (!Object)
	{
		UE_LOG(LogOverrideGame, Error, TEXT("Playlist %s does not exist or failed to load"), Configuration::Playlist.c_str());
		return nullptr;
	}

	if (!Object->IsA(UFortPlaylistAthena::StaticClass()))
	{
		UE_LOG(LogOverrideGame, Error, TEXT("%s loaded as %hs, which is not a FortPlaylistAthena"), Configuration::Playlist.c_str(), Object->Class->GetName().c_str());
		return nullptr;
	}

	return static_cast<UFortPlaylistAthena*>(Object);
}

void FortGameModeAthenaHooks::ApplyPlaylist(AFortGameModeAthena* GameMode, AFortGameStateAthena* GameState)
{
	UFortPlaylistAthena* Playlist = LoadPlaylist();
	if (!Playlist)
	{
		return;
	}

	Runtime::GetVirtual<void (*)(AFortGameModeAthena*, FName)>(GameMode, 0x13A)(GameMode, Playlist->PlaylistName); // AFortGameMode::SetCurrentPlaylistName

	GameState->CurrentPlaylistInfo.BasePlaylist = Playlist;
	GameState->CurrentPlaylistInfo.OverridePlaylist = Playlist;
	GameState->CurrentPlaylistInfo.PlaylistReplicationKey++;
	GameState->CurrentPlaylistInfo.ArrayReplicationKey++;
	GameState->OnRep_CurrentPlaylistInfo();

	const std::string PlaylistName = Playlist->GetName();
	UE_LOG(LogOverrideGame, Display, TEXT("Applied playlist %hs (%hs), waiting for its data to load"), PlaylistName.c_str(), Playlist->PlaylistName.GetRawString().c_str());
	SetStatus(std::format(L"Listening on port {}, loading {} (not joinable yet)", Configuration::Port, std::wstring(PlaylistName.begin(), PlaylistName.end())));
}

bool FortGameModeAthenaHooks::ReadyToStartMatch(AFortGameModeAthena* GameMode)
{
	static const FName WaitingToStart = ToName(L"WaitingToStart");
	static bool bPlaylistApplied = false;
	static std::string WaitingFor;

	const auto Wait = [](const std::string& Reason)
	{
		if (Reason != WaitingFor)
		{
			WaitingFor = Reason;
			UE_LOG(LogOverrideGame, Display, TEXT("Match is not ready to start: %hs"), Reason.c_str());
		}
	};

	AFortGameStateAthena* GameState = static_cast<AFortGameStateAthena*>(GameMode->GameState);
	if (!GameState)
	{
		UWorld* World = UWorld::GetWorld();
		Wait(std::format("{} has no GameState (World GameState is {})", GameMode->GetName(), World && World->GameState ? World->GameState->GetName() : "null"));
		return ReadyToStartMatchOriginal(GameMode);
	}

	if (GameMode->MatchState != WaitingToStart)
	{
		Wait(std::format("MatchState is {}", GameMode->MatchState.ToString()));
		return ReadyToStartMatchOriginal(GameMode);
	}

	if (!bPlaylistApplied)
	{
		bPlaylistApplied = true;
		ApplyPlaylist(GameMode, GameState);
		return false;
	}

	if (!GameState->bPlaylistDataIsLoaded)
	{
		Wait(std::format("{} has not loaded the playlist data yet", GameState->GetName()));
		return false;
	}

	if (!GameMode->bWorldIsReady)
	{
		GameMode->bWorldIsReady = true;
		UE_LOG(LogOverrideGame, Display, TEXT("Playlist data is loaded, %hs is ready to start the match"), GameMode->GetName().c_str());
		SetStatus(std::format(L"Listening on port {}, starting the match (not joinable yet)", Configuration::Port));
	}

	return true;
}

APawn* FortGameModeAthenaHooks::SpawnDefaultPawnFor(AFortGameModeAthena* GameMode, AController* NewPlayer, AActor* StartSpot)
{
	if (APawn* Pawn = SpawnDefaultPawnForOriginal(GameMode, NewPlayer, StartSpot))
	{
		return Pawn;
	}

	if (!NewPlayer || !StartSpot)
	{
		UE_LOG(LogOverrideGame, Error, TEXT("SpawnDefaultPawnFor called without a %hs"), NewPlayer ? "start spot" : "controller");
		return nullptr;
	}

	const FRotator StartRotation{ 0.0, StartSpot->K2_GetActorRotation().Yaw, 0.0 };
	const CoreUObject::FTransform SpawnTransform =UKismetMathLibrary::MakeTransform(StartSpot->K2_GetActorLocation(), StartRotation, FVector(1.0, 1.0, 1.0));

	APawn* Pawn = GameMode->SpawnDefaultPawnAtTransform(NewPlayer, SpawnTransform);
	if (!Pawn)
	{
		UE_LOG(LogOverrideGame, Error, TEXT("SpawnDefaultPawnAtTransform failed for %hs at %hs"), NewPlayer->GetName().c_str(), StartSpot->GetName().c_str());
		return nullptr;
	}

	UE_LOG(LogOverrideGame, Log, TEXT("Spawned %hs for %hs at %hs"), Pawn->GetName().c_str(), NewPlayer->GetName().c_str(), StartSpot->GetName().c_str());
	return Pawn;
}

void FortGameModeAthenaHooks::HandleMatchHasStarted(AFortGameModeAthena* GameMode)
{
	HandleMatchHasStartedOriginal(GameMode);

	UE_LOG(LogOverrideGame, Display, TEXT("Match has started on %hs"), GameMode->GetName().c_str());
	NetDriverHooks::SetJoinable();

	UWorld* World = UWorld::GetWorld();
	UNetDriver* NetDriver = World ? World->NetDriver : nullptr;
	if (!NetDriver)
	{
		return;
	}

	for (int32 i = 0; i < NetDriver->ClientConnections.Num(); i++)
	{
		UNetConnection* Connection = NetDriver->ClientConnections[i];
		if (Connection && Connection->PlayerController && Connection->PlayerController->IsA(AFortPlayerControllerAthena::StaticClass()))
		{
			FortPlayerControllerAthenaHooks::RestartPlayer(GameMode, static_cast<AFortPlayerControllerAthena*>(Connection->PlayerController));
		}
	}

}

void FortGameModeAthenaHooks::UpdatePlayersLoaded(AFortGameModeAthena* GameMode)
{
	static const auto Function = reinterpret_cast<void (*)(AFortGameModeAthena*)>(GetImageBase() + 0x101098F8); // AFortGameModeAthena PlayersLoaded recount
	Function(GameMode);

	AFortGameStateAthena* GameState = static_cast<AFortGameStateAthena*>(GameMode->GameState);
	if (GameState)
	{
		UE_LOG(LogOverrideGame, Display, TEXT("%hs PlayersLoaded %d, PlayersLeft %d"), GameState->GetName().c_str(), GameState->PlayersLoaded, GameState->PlayersLeft);
	}
}

void FortGameModeAthenaHooks::HandleStartingNewPlayer(AFortGameModeAthena* GameMode, APlayerController* NewPlayer)
{
	if (NewPlayer && NewPlayer->IsA(AFortPlayerController::StaticClass()))
	{
		AFortPlayerController* PlayerController = static_cast<AFortPlayerController*>(NewPlayer);
		FortInventoryHooks::CreateWorldInventory(PlayerController);
		FortAbilitySystemComponentHooks::GiveAbilitySet(PlayerController);
	}

	HandleStartingNewPlayerOriginal(GameMode, NewPlayer);
}

void FortGameModeAthenaHooks::Init()
{
	AFortGameModeBR* GameMode = AFortGameModeBR::GetDefaultObj();

	Runtime::VFTHook(GameMode, 0xE2, SpawnDefaultPawnFor, &SpawnDefaultPawnForOriginal, TEXT("AFortGameModeAthena::SpawnDefaultPawnFor_Implementation")); // AFortGameModeAthena::SpawnDefaultPawnFor_Implementation
	Runtime::VFTHook(GameMode, 0x11E, ReadyToStartMatch, &ReadyToStartMatchOriginal, TEXT("AFortGameModeAthena::ReadyToStartMatch_Implementation")); // AFortGameModeAthena::ReadyToStartMatch_Implementation
	Runtime::VFTHook(GameMode, 0x127, HandleMatchHasStarted, &HandleMatchHasStartedOriginal, TEXT("AFortGameModeAthena::HandleMatchHasStarted")); // AFortGameModeAthena::HandleMatchHasStarted
	Runtime::VFTHook(GameMode, 0xE8, HandleStartingNewPlayer, &HandleStartingNewPlayerOriginal, TEXT("AGameModeBase::HandleStartingNewPlayer_Implementation")); // AGameModeBase::HandleStartingNewPlayer_Implementation
}