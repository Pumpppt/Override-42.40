#include "pch.h"
#include "FortniteGame/Source/Public/FortInventory.h"

void FortInventoryHooks::CreateWorldInventory(AFortPlayerController* PlayerController)
{
	if (PlayerController->WorldInventory.GetObjectRef())
	{
		return;
	}

	UClass* InventoryClass = PlayerController->WorldInventoryClass.Get();
	if (!InventoryClass)
	{
		UE_LOG(LogOverrideInventory, Error, TEXT("%hs has no WorldInventoryClass, its ClientRestart will never finish"), PlayerController->GetName().c_str());
		return;
	}

	CoreUObject::FTransform SpawnTransform{};
	SpawnTransform.Rotation.W = 1.0;
	SpawnTransform.Scale3D = FVector(1.0, 1.0, 1.0);

	AActor* Inventory = UGameplayStatics::BeginDeferredActorSpawnFromClass(PlayerController, InventoryClass, SpawnTransform, ESpawnActorCollisionHandlingMethod::AlwaysSpawn, PlayerController, ESpawnActorScaleMethod::MultiplyWithRoot);
	Inventory = Inventory ? UGameplayStatics::FinishSpawningActor(Inventory, SpawnTransform, ESpawnActorScaleMethod::MultiplyWithRoot) : nullptr;
	if (!Inventory)
	{
		UE_LOG(LogOverrideInventory, Error, TEXT("Failed to spawn %hs for %hs"), InventoryClass->GetName().c_str(), PlayerController->GetName().c_str());
		return;
	}

	void* Interface = reinterpret_cast<uint8*>(Inventory) + 0x270; // AFortInventory::IFortInventoryInterface
	PlayerController->WorldInventory.ObjectPointer = Inventory;
	PlayerController->WorldInventory.InterfacePointer = Interface;
	PlayerController->ForceNetUpdate();

	UE_LOG(LogOverrideInventory, Display, TEXT("Spawned %hs as the WorldInventory of %hs (interface vtable 0x%llX)"), Inventory->GetName().c_str(), PlayerController->GetName().c_str(), *reinterpret_cast<uintptr_t*>(Interface) - GetImageBase());
}
