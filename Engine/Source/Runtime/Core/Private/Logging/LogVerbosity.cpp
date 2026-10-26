#include "pch.h"

namespace ELogVerbosity
{
	const wchar_t* ToString(Type Verbosity)
	{
		switch (Verbosity & VerbosityMask)
		{
		case Fatal:       return TEXT("Fatal");
		case Error:       return TEXT("Error");
		case Warning:     return TEXT("Warning");
		case Display:     return TEXT("Display");
		case Log:         return TEXT("Log");
		case Verbose:     return TEXT("Verbose");
		case VeryVerbose: return TEXT("VeryVerbose");
		default:          return TEXT("Unknown");
		}
	}

	WORD ToConsoleColor(Type Verbosity)
	{
		switch (Verbosity & VerbosityMask)
		{
		case Fatal:
		case Error:       return FOREGROUND_RED | FOREGROUND_INTENSITY;
		case Warning:     return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
		case Display:     return FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
		case Verbose:
		case VeryVerbose: return FOREGROUND_BLUE | FOREGROUND_GREEN;
		default:          return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
		}
	}
}
