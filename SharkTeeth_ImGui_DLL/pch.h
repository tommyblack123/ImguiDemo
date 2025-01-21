// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。

#ifndef PCH_H
#define PCH_H

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件
#include <windows.h>



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



extern HWND xz_游戏窗口句柄;



#endif //PCH_H

