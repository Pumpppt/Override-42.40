#include "pch.h"
#include "FortniteGame/Source/Public/Abilities/FortAbilitySystemComponent.h"

void FortAbilitySystemComponentHooks::GiveAbilitySet(AFortPlayerController* PlayerController)
{
	AFortPlayerState* PlayerState = static_cast<AFortPlayerState*>(PlayerController->PlayerState);
	UFortAbilitySystemComponent* AbilitySystemComponent = PlayerState ? PlayerState->AbilitySystemComponent : nullptr;
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogOverrideAbility, Error, TEXT("%hs has no ability system component on its player state"), PlayerController->GetName().c_str());
		return;
	}

	UFortAbilitySet* AbilitySet = PlayerController->PreloadedPlayerAbilitySetBR;
	if (!AbilitySet)
	{
		UObject* Object = UKismetSystemLibrary::LoadAsset_Blocking(reinterpret_cast<TSoftObjectPtr<UObject>&>(PlayerController->PlayerAbilitySetBR));
		AbilitySet = Object && Object->IsA(UFortAbilitySet::StaticClass()) ? static_cast<UFortAbilitySet*>(Object) : nullptr;
	}

	if (!AbilitySet)
	{
		UE_LOG(LogOverrideAbility, Error, TEXT("PlayerAbilitySetBR of %hs did not load, no abilities given"), PlayerController->GetName().c_str());
		return;
	}

	int32 Abilities = 0;
	for (TSubclassOf<UFortGameplayAbility>& Ability : AbilitySet->GameplayAbilities)
	{
		if (UClass* AbilityClass = Ability.Get())
		{
			AbilitySystemComponent->K2_GiveAbility(AbilityClass, 1, -1);
			Abilities++;
		}
	}

	int32 Effects = 0;
	for (FGameplayEffectApplicationInfoHard& Effect : AbilitySet->GrantedGameplayEffects)
	{
		if (Effect.GameplayEffect.Get())
		{
			AbilitySystemComponent->BP_ApplyGameplayEffectToSelf(Effect.GameplayEffect, Effect.Level, AbilitySystemComponent->MakeEffectContext());
			Effects++;
		}
	}

	UE_LOG(LogOverrideAbility, Display, TEXT("Gave %hs %d abilities and %d effects from %hs, %hs now has %d activatable abilities"), PlayerController->GetName().c_str(), Abilities, Effects, AbilitySet->GetName().c_str(), AbilitySystemComponent->GetName().c_str(), AbilitySystemComponent->ActivatableAbilities.Items.Num());
}
