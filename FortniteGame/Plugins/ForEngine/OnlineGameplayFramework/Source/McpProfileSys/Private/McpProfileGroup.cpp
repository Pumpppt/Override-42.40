#include "pch.h"
#include "FortniteGame/Plugins/ForEngine/OnlineGameplayFramework/Source/McpProfileSys/Public/McpProfileGroup.h"

void McpProfileGroupHooks::Init()
{
	Runtime::Patch(GetImageBase() + 0x13492548, { 0x18, 0xBF, 0x0F, 0xF2 }); // UMcpProfileGroup::SendRequestNow
}
