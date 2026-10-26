#pragma once

enum ENetMode : int32
{
	NM_Standalone,
	NM_DedicatedServer,
	NM_ListenServer,
	NM_Client,
	NM_MAX
};

inline const TCHAR* ToString(ENetMode NetMode)
{
	switch (NetMode)
	{
	case NM_Standalone: return TEXT("Standalone");
	case NM_DedicatedServer: return TEXT("DedicatedServer");
	case NM_ListenServer: return TEXT("ListenServer");
	case NM_Client: return TEXT("Client");
	default: return TEXT("Invalid");
	}
}

struct FURL
{
	FString Protocol;
	FString Host;
	int32 Port;
	int32 Valid;
	FString Map;
	FString RedirectURL;
	TArray<FString> Op;
	FString Portal;
};

class NetDriverHooks
{
public:
	static void Init();
	static ENetMode GetNetMode(UNetDriver* NetDriver);
	static void SetWorld(UNetDriver* NetDriver, UWorld* World);
	static bool InitListen(UNetDriver* NetDriver, UWorld* World, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error);
	static void SetJoinable();

private:
	static inline std::atomic<bool> bJoinable = false;

	static inline void (*TickFlushOriginal)(UNetDriver*, float);
	static void TickFlush(UNetDriver* NetDriver, float DeltaSeconds);

	static void SendClientMoveAdjustments(UNetDriver* NetDriver);
	static void SendClientAdjustment(APlayerController* PlayerController);
	static void UpdateJoinableStatus(UNetDriver* NetDriver);
};
