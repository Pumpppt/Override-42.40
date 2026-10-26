#include "pch.h"
#include "FortniteGame/Source/Public/FortGameState.h"

void FortGameStateHooks::Init()
{
	Runtime::Patch<uint8>(GetImageBase() + 0x19EBC2D, 0xEB); // AFortGameState GameSessionId server analytics
}
