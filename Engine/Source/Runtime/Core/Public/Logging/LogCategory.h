#pragma once

#include "LogVerbosity.h"

struct FLogCategory
{
	const wchar_t* Name;
	ELogVerbosity::Type Verbosity;
};

#define DECLARE_LOG_CATEGORY_EXTERN(CategoryName, DefaultVerbosity) extern FLogCategory CategoryName;
#define DEFINE_LOG_CATEGORY(CategoryName, DefaultVerbosity) FLogCategory CategoryName{ TEXT(#CategoryName), ELogVerbosity::DefaultVerbosity };

DECLARE_LOG_CATEGORY_EXTERN(LogOverride, Log)
DECLARE_LOG_CATEGORY_EXTERN(LogOverrideNet, Log)
DECLARE_LOG_CATEGORY_EXTERN(LogOverrideGame, Log)
DECLARE_LOG_CATEGORY_EXTERN(LogOverrideInventory, Log)
DECLARE_LOG_CATEGORY_EXTERN(LogOverrideAbility, Log)
