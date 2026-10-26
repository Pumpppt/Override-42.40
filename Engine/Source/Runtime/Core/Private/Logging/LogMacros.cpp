#include "pch.h"
#include <share.h>

namespace
{
	std::mutex LogMutex;
	HANDLE ConsoleOutput = nullptr;
	FILE* LogFile = nullptr;
	std::wstring ConsoleTitle;
	std::wstring CurrentStatus;

	std::wstring GetCategoryName(const FName& Category)
	{
		static std::mutex CacheMutex;
		static std::unordered_map<int32, std::wstring> Cache;

		std::scoped_lock Lock(CacheMutex);

		auto It = Cache.find(Category.ComparisonIndex);
		if (It == Cache.end())
		{
			const std::string Name = Category.ToString();
			It = Cache.emplace(Category.ComparisonIndex, std::wstring(Name.begin(), Name.end())).first;
		}

		return It->second;
	}

	void (*SerializeOriginal)(void*, const wchar_t*, uint8, const FName&, double);
	void Serialize(void* OutputDevice, const wchar_t* Data, uint8 Verbosity, const FName& Category, double Time)
	{
		thread_local bool bSerializing = false;
		if (bSerializing)
		{
			SerializeOriginal(OutputDevice, Data, Verbosity, Category, Time);
			return;
		}

		bSerializing = true;
		SerializeOriginal(OutputDevice, Data, Verbosity, Category, Time);
		bSerializing = false;

		static void* MirroredDevice = OutputDevice;
		if (OutputDevice == MirroredDevice && Data && *Data)
		{
			LogLine(GetCategoryName(Category).c_str(), static_cast<ELogVerbosity::Type>(Verbosity & ELogVerbosity::VerbosityMask), Data);
		}
	}
}

void InitLogging(const wchar_t* ModuleName)
{
	HMODULE Module = nullptr;
	GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCWSTR>(&InitLogging), &Module);

	wchar_t ModulePath[MAX_PATH]{};
	GetModuleFileNameW(Module, ModulePath, MAX_PATH);

	std::wstring LogPath = ModulePath;
	LogPath.erase(LogPath.find_last_of(L"\\/") + 1);
	LogPath += std::format(L"Override-{}.log", ModuleName);

	AllocConsole();
	ConsoleOutput = CreateFileW(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
	if (ConsoleOutput == INVALID_HANDLE_VALUE)
	{
		ConsoleOutput = nullptr;
	}

	LogFile = _wfsopen(LogPath.c_str(), L"w, ccs=UTF-8", _SH_DENYNO);

	ConsoleTitle = std::format(L"Override {}", ModuleName);
	SetConsoleTitleW(ConsoleTitle.c_str());

	UE_LOG(LogOverride, Display, TEXT("Logging to %s"), LogPath.c_str());
}

void RedirectEngineLog()
{
	Runtime::Hook(GetImageBase() + 0x607C8, Serialize, &SerializeOriginal, TEXT("FOutputDeviceFile::Serialize")); // FOutputDeviceFile::Serialize
}

void LogMessage(const FLogCategory& Category, ELogVerbosity::Type Verbosity, const wchar_t* Format, ...)
{
	va_list Args;
	va_start(Args, Format);

	va_list ArgsCopy;
	va_copy(ArgsCopy, Args);
	const int32 Length = _vscwprintf(Format, ArgsCopy);
	va_end(ArgsCopy);

	std::wstring Message(Length > 0 ? Length : 0, L'\0');
	if (Length > 0)
	{
		vswprintf_s(Message.data(), Length + 1, Format, Args);
	}

	va_end(Args);

	LogLine(Category.Name, Verbosity, Message.c_str());
}

void LogLine(const wchar_t* Category, ELogVerbosity::Type Verbosity, const wchar_t* Message)
{
	SYSTEMTIME Time;
	GetLocalTime(&Time);

	const bool bShowVerbosity = Verbosity != ELogVerbosity::Log && Verbosity != ELogVerbosity::Display;
	const std::wstring Line = std::format(L"[{:02}:{:02}:{:02}.{:03}] {}: {}{}{}\n",
		Time.wHour, Time.wMinute, Time.wSecond, Time.wMilliseconds,
		Category, bShowVerbosity ? ELogVerbosity::ToString(Verbosity) : L"", bShowVerbosity ? L": " : L"", Message);

	std::scoped_lock Lock(LogMutex);

	if (ConsoleOutput)
	{
		SetConsoleTextAttribute(ConsoleOutput, ELogVerbosity::ToConsoleColor(Verbosity));
		WriteConsoleW(ConsoleOutput, Line.c_str(), static_cast<DWORD>(Line.size()), nullptr, nullptr);
		SetConsoleTextAttribute(ConsoleOutput, ELogVerbosity::ToConsoleColor(ELogVerbosity::Log));
	}

	if (LogFile)
	{
		fputws(Line.c_str(), LogFile);
		fflush(LogFile);
	}
}

void SetStatus(const std::wstring& Status)
{
	std::scoped_lock Lock(LogMutex);

	if (Status == CurrentStatus)
	{
		return;
	}

	CurrentStatus = Status;
	SetConsoleTitleW(std::format(L"{} | {}", ConsoleTitle, Status).c_str());
}
