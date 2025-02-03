#pragma once
#include <windows.h>
#include <iostream>
#include <string>

int CreateConsole();


void HookVtb(int64_t* vTable, int nIndex, void* NewAddr);


