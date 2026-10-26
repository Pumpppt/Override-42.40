#include "pch.h"

namespace Runtime
{
	bool HookInternal(uintptr_t Address, void* Detour, void** Original, const wchar_t* Name)
	{
		static const MH_STATUS InitStatus = MH_Initialize();
		if (InitStatus != MH_OK && InitStatus != MH_ERROR_ALREADY_INITIALIZED)
		{
			UE_LOG(LogOverride, Error, TEXT("MinHook failed to initialize (%hs)"), MH_StatusToString(InitStatus));
			return false;
		}

		void* Target = reinterpret_cast<void*>(Address);
		MH_STATUS Status = MH_CreateHook(Target, Detour, Original);
		if (Status == MH_OK)
		{
			Status = MH_EnableHook(Target);
		}

		if (Status != MH_OK)
		{
			UE_LOG(LogOverride, Error, TEXT("Failed to hook %s at 0x%llX (%hs)"), Name, Address - GetImageBase(), MH_StatusToString(Status));
			return false;
		}

		UE_LOG(LogOverride, Log, TEXT("Hooked %s (0x%llX)"), Name, Address - GetImageBase());
		return true;
	}

	bool VFTHookInternal(const void* Object, int32 Index, void* Detour, void** Original, const wchar_t* Name)
	{
		if (!Object)
		{
			UE_LOG(LogOverride, Error, TEXT("Failed to VFT hook %s, object is null"), Name);
			return false;
		}

		void** VTable = *reinterpret_cast<void** const*>(Object);
		void* Current = VTable[Index];

		if (Original)
		{
			*Original = Current;
		}

		Patch<void*>(reinterpret_cast<uintptr_t>(&VTable[Index]), Detour);

		UE_LOG(LogOverride, Log, TEXT("VFT hooked %s (vtable 0x%llX, index 0x%X, function 0x%llX)"), Name, reinterpret_cast<uintptr_t>(VTable) - GetImageBase(), Index, reinterpret_cast<uintptr_t>(Current) - GetImageBase());
		return true;
	}
}
