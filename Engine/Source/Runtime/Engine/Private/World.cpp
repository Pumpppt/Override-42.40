#include "pch.h"
#include "Engine/Source/Runtime/Engine/Public/World.h"
#include "Engine/Source/Runtime/Engine/Public/UnrealEngine.h"

void WorldHooks::OnFrontendReady(UWorld* World)
{
	if (ServerMapRoot.empty())
	{
		UE_LOG(LogOverrideGame, Error, TEXT("Configuration::MapToLoad must be a long package name such as /Hera_Map/Maps/Hera_V2_Terrain, got %s"), Configuration::MapToLoad.c_str());
		SetStatus(L"Configuration::MapToLoad is not a long package name");
		return;
	}

	bFrontendReady = true;

	if (!bServerMapMounted)
	{
		UE_LOG(LogOverrideGame, Display, TEXT("%hs is up, waiting for %s to be mounted before travelling to %s"), World->GetName().c_str(), ServerMapRoot.c_str(), ServerMapName.c_str());
		SetStatus(std::format(L"Waiting for {} to be mounted", ServerMapRoot));
	}

	TryStartServerTravel();
}

void WorldHooks::InsertMountPoint(void* PackagePaths, void* OutDelegates, const FString& RootPath, const FString& ContentPath)
{
	InsertMountPointOriginal(PackagePaths, OutDelegates, RootPath, ContentPath);

	if (bServerMapMounted || ServerMapRoot.empty() || _wcsicmp(RootPath.ToWString().c_str(), ServerMapRoot.c_str()) != 0)
	{
		return;
	}

	bServerMapMounted = true;
	UE_LOG(LogOverrideGame, Display, TEXT("%s is mounted from %s"), ServerMapRoot.c_str(), ContentPath.ToWString().c_str());

	TryStartServerTravel();
}

void WorldHooks::TryStartServerTravel()
{
	if (!bFrontendReady || !bServerMapMounted || bServerTravelStarted.exchange(true))
	{
		return;
	}

	UWorld* World = UWorld::GetWorld();
	GIsServer = true;

	UE_LOG(LogOverrideGame, Display, TEXT("Travelling from %hs to %s"), World->GetName().c_str(), Configuration::MapToLoad.c_str());
	SetStatus(std::format(L"Travelling to {}", ServerMapName));
	UKismetSystemLibrary::ExecuteConsoleCommand(World, FString(std::format(L"open {}", Configuration::MapToLoad).c_str()), nullptr);
}

bool WorldHooks::SetGameMode(UWorld* World, const FURL& InURL)
{
	const bool bResult = SetGameModeOriginal(World, InURL);

	const std::string WorldName = World->GetName();
	if (ServerMapName.empty() || _wcsicmp(std::wstring(WorldName.begin(), WorldName.end()).c_str(), ServerMapName.c_str()) != 0)
	{
		UE_LOG(LogOverrideNet, Log, TEXT("%hs is not the server map %s, not listening on it"), WorldName.c_str(), ServerMapName.c_str());
		return bResult;
	}

	if (!bResult)
	{
		UE_LOG(LogOverrideNet, Error, TEXT("SetGameMode failed for %hs, the server will not listen"), World->GetName().c_str());
		SetStatus(std::format(L"SetGameMode failed on {}", ServerMapName));
		return false;
	}

	Listen(World);
	return true;
}

bool WorldHooks::Listen(UWorld* World)
{
	if (World->NetDriver)
	{
		UE_LOG(LogOverrideNet, Warning, TEXT("%hs already has NetDriver %hs, not listening again"), World->GetName().c_str(), World->NetDriver->GetName().c_str());
		return false;
	}

	RouteFilteredOutClasses();

	const FName GameNetDriver = ToName(L"GameNetDriver");
	if (!UnrealEngineHooks::CreateNamedNetDriver(World, GameNetDriver, GameNetDriver))
	{
		UE_LOG(LogOverrideNet, Error, TEXT("CreateNamedNetDriver failed for %hs, check the GameNetDriver NetDriverDefinition"), World->GetName().c_str());
		return false;
	}

	UNetDriver* NetDriver = UnrealEngineHooks::FindNamedNetDriver(World, GameNetDriver);
	if (!NetDriver)
	{
		UE_LOG(LogOverrideNet, Error, TEXT("GameNetDriver was created but FindNamedNetDriver returned null for %hs"), World->GetName().c_str());
		return false;
	}

	World->NetDriver = NetDriver;
	NetDriverHooks::SetWorld(NetDriver, World);

	for (int32 i = 0; i < World->LevelCollections.Num(); i++)
	{
		FLevelCollection& Collection = World->LevelCollections[i];
		const ELevelCollectionType Type = *reinterpret_cast<const ELevelCollectionType*>(&Collection);
		if (Type == ELevelCollectionType::DynamicSourceLevels || Type == ELevelCollectionType::StaticLevels)
		{
			Collection.NetDriver = NetDriver;
		}
	}

	AWorldSettings* WorldSettings = World->K2_GetWorldSettings();
	const bool bReuseAddressAndPort = WorldSettings && WorldSettings->bReuseAddressAndPort;

	FURL ListenURL{};
	ListenURL.Port = Configuration::Port;
	ListenURL.Valid = 1;

	FString Error;
	if (!NetDriverHooks::InitListen(NetDriver, World, ListenURL, bReuseAddressAndPort, Error))
	{
		UE_LOG(LogOverrideNet, Error, TEXT("%hs failed to listen on port %d: %hs"), NetDriver->GetName().c_str(), Configuration::Port, Error.ToString().c_str());
		SetStatus(std::format(L"Failed to listen on {}", Configuration::Port));

		World->NetDriver = nullptr;
		for (int32 i = 0; i < World->LevelCollections.Num(); i++)
		{
			if (World->LevelCollections[i].NetDriver == NetDriver)
			{
				World->LevelCollections[i].NetDriver = nullptr;
			}
		}

		return false;
	}

	if (NetDriver->MaxInternetClientRate < NetDriver->MaxClientRate && NetDriver->MaxInternetClientRate > 2500)
	{
		NetDriver->MaxClientRate = NetDriver->MaxInternetClientRate;
	}

	SpawnServerStreamingLevelsVisibility(World);

	UE_LOG(LogOverrideNet, Display, TEXT("%hs (%hs) is listening on port %d for %hs as %s"), NetDriver->GetName().c_str(), NetDriver->Class->GetName().c_str(), ListenURL.Port, World->GetName().c_str(), ToString(NetDriverHooks::GetNetMode(NetDriver)));
	SetStatus(std::format(L"Listening on port {}, not joinable until the match starts", ListenURL.Port));
	return true;
}

void WorldHooks::SpawnServerStreamingLevelsVisibility(UWorld* World)
{
	if (World->ServerStreamingLevelsVisibility)
	{
		return;
	}

	CoreUObject::FTransform SpawnTransform{};
	SpawnTransform.Rotation.W = 1.0;
	SpawnTransform.Scale3D = FVector(1.0, 1.0, 1.0);

	AActor* Actor = UGameplayStatics::BeginDeferredActorSpawnFromClass(World, AServerStreamingLevelsVisibility::StaticClass(), SpawnTransform, ESpawnActorCollisionHandlingMethod::AlwaysSpawn, nullptr, ESpawnActorScaleMethod::MultiplyWithRoot);
	Actor = Actor ? UGameplayStatics::FinishSpawningActor(Actor, SpawnTransform, ESpawnActorScaleMethod::MultiplyWithRoot) : nullptr;
	if (!Actor)
	{
		UE_LOG(LogOverrideNet, Error, TEXT("Failed to spawn ServerStreamingLevelsVisibility, clients will not be allowed to make streamed levels visible"));
		return;
	}

	World->ServerStreamingLevelsVisibility = static_cast<AServerStreamingLevelsVisibility*>(Actor);
	UE_LOG(LogOverrideNet, Log, TEXT("Spawned %hs for %hs"), Actor->GetName().c_str(), World->GetName().c_str());
}

void WorldHooks::RouteFilteredOutClasses()
{
	UObjectReplicationBridgeConfig* BridgeConfig = UObjectReplicationBridgeConfig::GetDefaultObj();
	UNetObjectFilterDefinitions* FilterDefinitions = UNetObjectFilterDefinitions::GetDefaultObj();
	if (!BridgeConfig || !FilterDefinitions)
	{
		UE_LOG(LogOverrideNet, Warning, TEXT("Iris filter configs are not loaded, NotRouted classes will never replicate"));
		return;
	}

	const FName NoFilter = ToName(L"None");
	for (FObjectReplicationBridgeFilterConfig& FilterConfig : BridgeConfig->FilterConfigs)
	{
		std::string FilterClass = "no definition";
		for (const FNetObjectFilterDefinition& Definition : FilterDefinitions->NetObjectFilterDefinitions)
		{
			if (Definition.FilterName == FilterConfig.DynamicFilterName)
			{
				FilterClass = Definition.ClassName.ToString();
			}
		}

		const std::string ClassName = FilterConfig.ClassName.ToString();
		const bool bServerRouted = FilterClass.find("FilterOutNetObjectFilter") != std::string::npos || FilterClass.find("ConnectionFilter") != std::string::npos;
		const bool bReroute = bServerRouted && ClassName != "Engine.LevelScriptActor";
		UE_LOG(LogOverrideNet, Log, TEXT("Iris filter config %hs -> %hs (%hs)%hs"), ClassName.c_str(), FilterConfig.DynamicFilterName.ToString().c_str(), FilterClass.c_str(), bReroute ? ", rerouted to None" : "");

		if (bReroute)
		{
			FilterConfig.DynamicFilterName = NoFilter;
		}
	}
}

ENetMode WorldHooks::AttemptDeriveFromURL(UWorld* World)
{
	if (World->NetDriver)
	{
		return NetDriverHooks::GetNetMode(World->NetDriver);
	}

	return AttemptDeriveFromURLOriginal(World);
}

void WorldHooks::PatchInlinedNetModes()
{
	const auto IsNetDriverCheck = [](const uint8* Code)
	{
		return (Code[0] == 0x48 || Code[0] == 0x49) && Code[1] == 0x83 && (Code[2] & 0xF8) == 0x78 && Code[2] != 0x7C && Code[3] == 0x48 && Code[4] == 0x00;
	};

	const auto IsStackMove = [](const uint8* Code)
	{
		return (Code[0] == 0x48 || Code[0] == 0x4C) && (Code[1] == 0x89 || Code[1] == 0x8B) && (Code[2] & 0xC7) == 0x44 && Code[3] == 0x24;
	};

	const auto IsRegisterMove = [](const uint8* Code)
	{
		return (Code[0] & 0xF0) == 0x40 && Code[1] == 0x89 && Code[2] >= 0xC0;
	};

	const auto IsDemoNetDriverLoad = [](const uint8* Code)
	{
		if ((Code[0] & 0xFA) != 0x48 || Code[1] != 0x8B || (Code[2] & 0xC0) != 0x80)
		{
			return false;
		}

		const uint8* Displacement = (Code[2] & 0x07) != 0x04 ? Code + 3 : Code[3] == 0x24 ? Code + 4 : nullptr;
		return Displacement && *reinterpret_cast<const uint32*>(Displacement) == 0x160;
	};

	const auto DosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(GetImageBase());
	const auto NtHeaders = reinterpret_cast<const IMAGE_NT_HEADERS*>(GetImageBase() + DosHeader->e_lfanew);
	const IMAGE_SECTION_HEADER* Text = IMAGE_FIRST_SECTION(NtHeaders);
	uint8* Start = reinterpret_cast<uint8*>(GetImageBase() + Text->VirtualAddress);
	uint8* End = Start + Text->Misc.VirtualSize - 32;

	const uint8* LocalPlayerSpawnCheck = reinterpret_cast<const uint8*>(GetImageBase() + 0x194CDA2); // ULocalPlayer::SpawnPlayActor

	int32 Patched = 0;
	for (uint8* Code = Start; Code < End; Code++)
	{
		if (Code == LocalPlayerSpawnCheck || !IsNetDriverCheck(Code))
		{
			continue;
		}

		uint8* Branch = Code + 5;
		for (int32 i = 0; i < 3 && IsStackMove(Branch); i++)
		{
			Branch += 5;
		}

		const bool bShort = Branch[0] == 0x75;
		if (!bShort && (Branch[0] != 0x0F || Branch[1] != 0x85))
		{
			continue;
		}

		uint8* FallThrough = Branch + (bShort ? 2 : 6);
		for (int32 i = 0; i < 2 && IsRegisterMove(FallThrough); i++)
		{
			FallThrough += 3;
		}

		if (!IsDemoNetDriverLoad(FallThrough))
		{
			continue;
		}

		Runtime::Patch<uint8>(reinterpret_cast<uintptr_t>(bShort ? Branch : Branch + 1), bShort ? 0x70 : 0x80);
		Patched++;
	}

	UE_LOG(LogOverrideNet, Display, TEXT("Routed %d inlined UWorld net mode checks to UWorld::AttemptDeriveFromURL"), Patched);
}

void WorldHooks::Init()
{
	const std::wstring& Map = Configuration::MapToLoad;
	if (Map.starts_with(L'/') && Map.find(L'/', 1) != std::wstring::npos)
	{
		ServerMapRoot = Map.substr(0, Map.find(L'/', 1) + 1);
		ServerMapName = Map.substr(Map.find_last_of(L'/') + 1);
	}

	Runtime::Patch(GetImageBase() + 0x5B8D85, { 0x90, 0x90 }); // ULevelStreaming::ShouldClientUseMakingVisibleTransactionRequest

	Runtime::Hook(GetImageBase() + 0x9C1E9CE, InsertMountPoint, &InsertMountPointOriginal, TEXT("FLongPackagePathsSingleton::InsertMountPoint")); // FLongPackagePathsSingleton::InsertMountPoint
	Runtime::Hook(GetImageBase() + 0x45B1F68, SetGameMode, &SetGameModeOriginal, TEXT("UWorld::SetGameMode")); // UWorld::SetGameMode
	Runtime::Hook(GetImageBase() + 0xA79482, AttemptDeriveFromURL, &AttemptDeriveFromURLOriginal, TEXT("UWorld::AttemptDeriveFromURL")); // UWorld::AttemptDeriveFromURL

	PatchInlinedNetModes();
}
