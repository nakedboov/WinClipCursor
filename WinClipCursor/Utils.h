#pragma once

#include <Windows.h>
#include <string>

#ifdef _DEBUG
#define DEBUG_TRACE(msg) std::cout << msg << std::endl;
#else
#define DEBUG_TRACE(msg)
#endif

void GetErrorDescription(DWORD errorCode, std::string& description);
bool CalcRequiredClipRect(HWND hwnd, RECT& rect);
bool CursorInClientArea(HWND hwnd);
HWND FindRequiredWindow(const std::wstring& className, const std::wstring& title, unsigned int attemptCount);