#pragma once

#include "LogCategory.h"

void InitLogging(const wchar_t* ModuleName);
void RedirectEngineLog();
void LogMessage(const FLogCategory& Category, ELogVerbosity::Type Verbosity, const wchar_t* Format, ...);
void LogLine(const wchar_t* Category, ELogVerbosity::Type Verbosity, const wchar_t* Message);
void SetStatus(const std::wstring& Status);

#define UE_LOG(CategoryName, VerbosityName, Format, ...) \
	do \
	{ \
		if (ELogVerbosity::VerbosityName <= CategoryName.Verbosity) \
		{ \
			LogMessage(CategoryName, ELogVerbosity::VerbosityName, Format, ##__VA_ARGS__); \
		} \
	} while (0)
