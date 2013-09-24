#include "Utils.h"

void GetErrorDescription(DWORD errorCode, std::string& description)
{
	LPWSTR lpMsgBuf = nullptr;
   
	DWORD count = FormatMessageW(
					FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
					nullptr,
					errorCode,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPWSTR)&lpMsgBuf,
					0, nullptr );

	if (count != 0)
	{
		char dst[255] = {0};
		CharToOemBuffW(lpMsgBuf, dst, sizeof(dst));
		description.assign(dst, count);
	}
	else
	{
		description.assign("cannt retrieve error description");
	}

	if (lpMsgBuf != 0)
		LocalFree(lpMsgBuf);
}


bool CalcRequiredClipRect(HWND hwnd, RECT& rect)
{
	std::string description;

	if (!GetClientRect(hwnd, &rect))
	{
		DWORD lastError = GetLastError();
		GetErrorDescription(lastError, description);
		throw std::runtime_error(description);
	}

	POINT pt1 = { rect.left, rect.top };
	POINT pt2 = { rect.right, rect.bottom };

	if (!ClientToScreen(hwnd, &pt1))
		return false;
	
	if (!ClientToScreen(hwnd, &pt2))
		return false;

	if (!SetRect(&rect, pt1.x, pt1.y, pt2.x, pt2.y))
		return false;

	return true;
}

bool CursorInClientArea(HWND hwnd)
{
	POINT cursorPos;
	if (!GetCursorPos(&cursorPos))
		return false;
	
	RECT rc;
	if (!GetClientRect(hwnd, &rc))
		return false;

	if (!ScreenToClient(hwnd, &cursorPos))
		return false;

	return (PtInRect(&rc, cursorPos)) ? true : false;
}

extern const unsigned int g_SleepTimeOut;
HWND FindRequiredWindow(const std::wstring& className, const std::wstring& title, unsigned int attemptCount)
{
	HWND requiredWindow = NULL;

	if (className.empty() && title.empty())
		return NULL;

	while (attemptCount-- > 0)
	{
		requiredWindow	= FindWindowW(className.c_str(), title.c_str());
		if (requiredWindow != NULL)
			break;
						
		Sleep(g_SleepTimeOut);
	}

	return requiredWindow;
}