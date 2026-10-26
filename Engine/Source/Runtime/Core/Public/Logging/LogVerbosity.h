#pragma once

namespace ELogVerbosity
{
	enum Type : uint8
	{
		NoLogging = 0,
		Fatal,
		Error,
		Warning,
		Display,
		Log,
		Verbose,
		VeryVerbose,

		VerbosityMask = 0xF
	};

	const wchar_t* ToString(Type Verbosity);
	WORD ToConsoleColor(Type Verbosity);
}
