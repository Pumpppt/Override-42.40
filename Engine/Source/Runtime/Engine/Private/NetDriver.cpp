#include "pch.h"
#include "Engine/Source/Runtime/Engine/Public/NetDriver.h"

void NetDriverHooks::Init()
{
	Runtime::Patch(GetImageBase() + 0xA1936D8, { 0x31, 0xC0, 0x0F, 0x1F, 0x44, 0x00, 0x00 }); // UNetDriver::GetNetMode
	Runtime::Patch(GetImageBase() + 0x441FE80, { 0x0F, 0x1F, 0x44, 0x00, 0x00 }); // UNetDriver::TickFlush
	Runtime::Patch(GetImageBase() + 0x441FE93, { 0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00 }); // UNetDriver::TickFlush
	Runtime::Patch(GetImageBase() + 0x441FEA1, { 0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00 }); // UNetDriver::TickFlush

	Runtime::Hook(GetImageBase() + 0x441FC3A, TickFlush, &TickFlushOriginal, TEXT("UNetDriver::TickFlush")); // UNetDriver::TickFlush
}

ENetMode NetDriverHooks::GetNetMode(UNetDriver* NetDriver)
{
	static const auto Function = reinterpret_cast<ENetMode (*)(UNetDriver*)>(GetImageBase() + 0xA19367A); // UNetDriver::GetNetMode
	return Function(NetDriver);
}

void NetDriverHooks::SetWorld(UNetDriver* NetDriver, UWorld* World)
{
	Runtime::GetVirtual<void (*)(UNetDriver*, UWorld*)>(NetDriver, 0x7A)(NetDriver, World); // UNetDriver::SetWorld
}

bool NetDriverHooks::InitListen(UNetDriver* NetDriver, UWorld* World, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error)
{
	void* Notify = reinterpret_cast<uint8*>(World) + 0x28; // UWorld::FNetworkNotify
	return Runtime::GetVirtual<bool (*)(UNetDriver*, void*, FURL&, bool, FString&)>(NetDriver, 0x55)(NetDriver, Notify, ListenURL, bReuseAddressAndPort, Error); // UNetDriver::InitListen
}

void NetDriverHooks::TickFlush(UNetDriver* NetDriver, float DeltaSeconds)
{
	if (NetDriver->World && NetDriver->World->NetDriver == NetDriver)
	{
		SendClientMoveAdjustments(NetDriver);
		UpdateJoinableStatus(NetDriver);
	}

	TickFlushOriginal(NetDriver, DeltaSeconds);
}

void NetDriverHooks::SetJoinable()
{
	bJoinable = true;
}

void NetDriverHooks::UpdateJoinableStatus(UNetDriver* NetDriver)
{
	static int32 LastInGame = -1;
	static int32 LastLoading = -1;

	if (!bJoinable)
	{
		return;
	}

	int32 InGame = 0;
	int32 Loading = 0;
	for (int32 i = 0; i < NetDriver->ClientConnections.Num(); i++)
	{
		UNetConnection* Connection = NetDriver->ClientConnections[i];
		if (!Connection)
		{
			continue;
		}

		if (Connection->PlayerController && Connection->PlayerController->Pawn)
		{
			InGame++;
		}
		else
		{
			Loading++;
		}
	}

	if (InGame == LastInGame && Loading == LastLoading)
	{
		return;
	}

	LastInGame = InGame;
	LastLoading = Loading;

	UE_LOG(LogOverrideNet, Display, TEXT("Joinable on port %d: %d in game, %d loading in"), Configuration::Port, InGame, Loading);
	SetStatus(std::format(L"Joinable on port {} | {} in game, {} loading in", Configuration::Port, InGame, Loading));
}

void NetDriverHooks::SendClientMoveAdjustments(UNetDriver* NetDriver)
{
	for (int32 i = 0; i < NetDriver->ClientConnections.Num(); i++)
	{
		UNetConnection* Connection = NetDriver->ClientConnections[i];
		if (!Connection || !Connection->ViewTarget)
		{
			continue;
		}

		SendClientAdjustment(Connection->PlayerController);

		for (int32 j = 0; j < Connection->Children.Num(); j++)
		{
			if (UChildConnection* Child = Connection->Children[j])
			{
				SendClientAdjustment(Child->PlayerController);
			}
		}
	}
}

void NetDriverHooks::SendClientAdjustment(APlayerController* PlayerController)
{
	if (!PlayerController || !PlayerController->Pawn || PlayerController->AcknowledgedPawn != PlayerController->Pawn || !PlayerController->Pawn->IsA(ACharacter::StaticClass()))
	{
		return;
	}

	UCharacterMovementComponent* Movement = static_cast<ACharacter*>(PlayerController->Pawn)->CharacterMovement;
	if (!Movement)
	{
		return;
	}

	void* NetworkPrediction = reinterpret_cast<uint8*>(Movement) + 0x190; // UCharacterMovementComponent::INetworkPredictionInterface
	Runtime::GetVirtual<void (*)(void*)>(NetworkPrediction, 0x2)(NetworkPrediction); // INetworkPredictionInterface::SendClientAdjustment
}
