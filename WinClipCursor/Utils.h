#pragma once

#include <Windows.h>
#include <string>

#ifdef _DEBUG
#define DEBUG_TRACE(msg) std::cout << msg << std::endl;
#else
#define DEBUG_TRACE(msg)
#endif

void GetErrorDescription(DWORD errorCode, std::string& description);
void RaiseError(const char* msgPrefix);

bool CalcRequiredClipRect(HWND hwnd, RECT& rect);
bool CursorInClientArea(HWND hwnd);
HWND FindRequiredWindow(const std::wstring& className, const std::wstring& title, unsigned int attemptCount);

namespace Permissions
{

void EnableDebugPrivileges();
bool IsCurrentProcessElevated();
bool TryToSelfElevate(HWND hwnd);
bool IsRunAsAdmin();
void EnableRequiredAccess(const wchar_t* className, const wchar_t* windowTitle);

} //namespace Permissions