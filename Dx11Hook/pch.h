#pragma once

#include <windows.h>

#include <thread>


#include <string>
//打印调试输出
template<typename ...Args>
void OutputDebugStringEx(std::string&& format, Args&&... args)
{
	char* text = new char[1024];

	sprintf(text, format.data(), std::forward<std::remove_reference_t<Args>>(args)...);
	OutputDebugStringA(text);

	delete[] text;
	text = nullptr;
}

template<typename ...Args>
void OutputDebugStringEx(std::wstring&& format, Args&&... args)
{
	wchar_t* text = new wchar_t[1024];

	swprintf(text, format.data(), std::forward<std::remove_reference_t<Args>>(args)...);
	OutputDebugStringW(text);

	delete[] text;
	text = nullptr;
}



