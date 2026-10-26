#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <algorithm>
#include <atomic>
#include <cstdarg>
#include <cstdint>
#include <format>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Engine/Source/ThirdParty/minhook/MinHook.h"
#include "Engine/Source/ThirdParty/SDK/SDK.hpp"

#pragma comment(lib, "Engine/Source/ThirdParty/minhook/minhook.lib")

using namespace SDK;

namespace Configuration
{
	inline std::wstring Playlist = L"/BRPlaylists/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo";
	inline std::wstring MapToLoad = L"/Hera_Map/Maps/Hera_V2_Terrain";
	inline int32 Port = 7777;
}

#include "Engine/Source/Runtime/Core/Public/HAL/Runtime.h"
#include "Engine/Source/Runtime/Core/Public/CoreGlobals.h"
#include "Engine/Source/Runtime/Core/Public/UObject/NameTypes.h"
#include "Engine/Source/Runtime/Core/Public/Logging/LogMacros.h"
