#pragma once

class FortPlayerControllerHooks
{
public:
	static void Init();

private:
	static inline void (*ServerAttemptInteractOriginal)(UFortControllerComponent_Interaction*, AActor*, UPrimitiveComponent*, T1ByteSignedEnum<ETInteractionType>, UObject*, T1ByteSignedEnum<EInteractionBeingAttempted>, int32);
	static void ServerAttemptInteract(UFortControllerComponent_Interaction* Interaction, AActor* ReceivingActor, UPrimitiveComponent* InteractComponent, T1ByteSignedEnum<ETInteractionType> InteractType, UObject* OptionalObjectData, T1ByteSignedEnum<EInteractionBeingAttempted> InteractionBeingAttempted, int32 RequestID);
};
