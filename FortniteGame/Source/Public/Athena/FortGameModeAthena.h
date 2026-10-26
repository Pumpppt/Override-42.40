#pragma once

class FortGameModeAthenaHooks
{
public:
	static void Init();
	static UFortPlaylistAthena* LoadPlaylist();
	static void UpdatePlayersLoaded(AFortGameModeAthena* GameMode);

private:
	static inline void (*HandleStartingNewPlayerOriginal)(AFortGameModeAthena*, APlayerController*);
	static void HandleStartingNewPlayer(AFortGameModeAthena* GameMode, APlayerController* NewPlayer);

	static inline bool (*ReadyToStartMatchOriginal)(AFortGameModeAthena*);
	static bool ReadyToStartMatch(AFortGameModeAthena* GameMode);

	static inline APawn* (*SpawnDefaultPawnForOriginal)(AFortGameModeAthena*, AController*, AActor*);
	static APawn* SpawnDefaultPawnFor(AFortGameModeAthena* GameMode, AController* NewPlayer, AActor* StartSpot);

	static inline void (*HandleMatchHasStartedOriginal)(AFortGameModeAthena*);
	static void HandleMatchHasStarted(AFortGameModeAthena* GameMode);

	static void ApplyPlaylist(AFortGameModeAthena* GameMode, AFortGameStateAthena* GameState);
};
