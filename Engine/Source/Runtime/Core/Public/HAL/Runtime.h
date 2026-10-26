#pragma once

inline uintptr_t GetImageBase()
{
	static const uintptr_t ImageBase = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
	return ImageBase;
}

namespace Runtime
{
	template<typename T>
	void Patch(uintptr_t Address, T Value)
	{
		DWORD OldProtect;
		VirtualProtect(reinterpret_cast<void*>(Address), sizeof(T), PAGE_EXECUTE_READWRITE, &OldProtect);
		*reinterpret_cast<T*>(Address) = Value;
		VirtualProtect(reinterpret_cast<void*>(Address), sizeof(T), OldProtect, &OldProtect);
		FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(Address), sizeof(T));
	}

	inline void Patch(uintptr_t Address, std::initializer_list<uint8> Bytes)
	{
		DWORD OldProtect;
		VirtualProtect(reinterpret_cast<void*>(Address), Bytes.size(), PAGE_EXECUTE_READWRITE, &OldProtect);
		memcpy(reinterpret_cast<void*>(Address), Bytes.begin(), Bytes.size());
		VirtualProtect(reinterpret_cast<void*>(Address), Bytes.size(), OldProtect, &OldProtect);
		FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(Address), Bytes.size());
	}

	bool HookInternal(uintptr_t Address, void* Detour, void** Original, const wchar_t* Name);
	bool VFTHookInternal(const void* Object, int32 Index, void* Detour, void** Original, const wchar_t* Name);

	template<typename FuncType>
	bool Hook(uintptr_t Address, FuncType Detour, FuncType* Original, const wchar_t* Name)
	{
		return HookInternal(Address, reinterpret_cast<void*>(Detour), reinterpret_cast<void**>(Original), Name);
	}

	template<typename FuncType>
	bool VFTHook(const void* Object, int32 Index, FuncType Detour, FuncType* Original, const wchar_t* Name)
	{
		return VFTHookInternal(Object, Index, reinterpret_cast<void*>(Detour), reinterpret_cast<void**>(Original), Name);
	}

	template<typename FuncType>
	FuncType GetVirtual(const void* Object, int32 Index)
	{
		return reinterpret_cast<FuncType>((*reinterpret_cast<void** const*>(Object))[Index]);
	}
}
