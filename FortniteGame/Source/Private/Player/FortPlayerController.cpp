#include "pch.h"
#include "FortniteGame/Source/Public/Player/FortPlayerController.h"

void FortPlayerControllerHooks::ServerAttemptInteract(UFortControllerComponent_Interaction* Interaction, AActor* ReceivingActor, UPrimitiveComponent* InteractComponent, T1ByteSignedEnum<ETInteractionType> InteractType, UObject* OptionalObjectData, T1ByteSignedEnum<EInteractionBeingAttempted> InteractionBeingAttempted, int32 RequestID)
{
	AActor* Owner = Interaction->GetOwner();
	APawn* Pawn = Owner && Owner->IsA(AController::StaticClass()) ? static_cast<AController*>(Owner)->Pawn : nullptr;
	if (!Pawn || !ReceivingActor)
	{
		UE_LOG(LogOverrideGame, Warning, TEXT("Interact request %d from %hs has no %hs"), RequestID, Owner ? Owner->GetName().c_str() : "owner", Pawn ? "receiving actor" : "pawn");
		return;
	}

	const bool bInteracted = Interaction->BP_ServerAttemptInteractInternal(Pawn, 512.0f, ReceivingActor, InteractComponent, InteractType, OptionalObjectData, InteractionBeingAttempted);
	UE_LOG(LogOverrideGame, Log, TEXT("%hs interact request %d on %hs: %hs"), Pawn->GetName().c_str(), RequestID, ReceivingActor->GetName().c_str(), bInteracted ? "accepted" : "rejected");
}

void FortPlayerControllerHooks::Init()
{
	UFortControllerComponent_Interaction* Interaction = UFortControllerComponent_Interaction::GetDefaultObj();

	Runtime::VFTHook(Interaction, 0xBF, ServerAttemptInteract, &ServerAttemptInteractOriginal, TEXT("UFortControllerComponent_Interaction::ServerAttemptInteract_Implementation")); // UFortControllerComponent_Interaction::ServerAttemptInteract_Implementation
}
