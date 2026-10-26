#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/Windows/WindowsPlatformMisc.h"

void WindowsPlatformMiscHooks::Init()
{
	Runtime::Patch<uint8_t>(GetImageBase() + 0x3C0E4EC, 0xC3); // FWindowsPlatformMisc::RequestExit
	Runtime::Patch<uint8_t>(GetImageBase() + 0x3C1240A, 0xC3); // FWindowsPlatformMisc::RequestExitWithStatus
}
