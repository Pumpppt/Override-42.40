#pragma once

class FortPlayerControllerAthenaHooks
{
public:
	static void Init();
	static void RestartPlayer(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* PlayerController);

private:
	static inline void (*ServerAcknowledgePossessionOriginal)(AFortPlayerControllerAthena*, APawn*);
	static void ServerAcknowledgePossession(AFortPlayerControllerAthena* PlayerController, APawn* Pawn);

	static inline void (*ServerSetClientHasFinishedLoadingOriginal)(AFortPlayerControllerAthena*, bool);
	static void ServerSetClientHasFinishedLoading(AFortPlayerControllerAthena* PlayerController, bool bInHasFinishedLoading);
};
