#include "pch.h"
#include "FortniteGame/Source/Public/FortGameSession.h"

void FortGameSessionHooks::Init()
{
	Runtime::Patch(GetImageBase() + 0xFC224FA, { 0xB8, 0x01, 0x00, 0x00, 0x00, 0xC3 }); // AFortGameSession reservation check
	Runtime::Patch(GetImageBase() + 0x6C3C2D0, { 0xB8, 0x01, 0x00, 0x00, 0x00, 0xC3 }); // AFortGameSession auth ticket check
}
