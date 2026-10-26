#include "pch.h"
#include "FortniteGame/Source/Public/FortWeaponSoundLibraryComponent.h"

void FortWeaponSoundLibraryComponentHooks::Init()
{
	Runtime::Patch(GetImageBase() + 0xF7C240, { 0x90, 0xE9 }); // UFortWeaponSoundLibraryComponent::BeginPlay
}
