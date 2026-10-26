#pragma once

inline FName ToName(const wchar_t* String)
{
	return UKismetStringLibrary::Conv_StringToName(FString(String));
}
