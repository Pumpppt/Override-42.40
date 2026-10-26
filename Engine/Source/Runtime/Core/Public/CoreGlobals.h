#pragma once

inline bool& GIsClient = *reinterpret_cast<bool*>(GetImageBase() + 0x1AF3CBA8); // GIsClient
inline bool& GIsServer = *reinterpret_cast<bool*>(GetImageBase() + 0x1AF31FD0); // GIsServer
