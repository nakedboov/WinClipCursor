#include "Utils.h"

#include <ShellAPI.h>

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

void RaiseError(const char* msgPrefix)
{
	std::string description;
	GetErrorDescription(::GetLastError(), description);
	
	std::string msg(msgPrefix);
	msg.append(description);
	
	throw std::runtime_error(msg);
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

namespace Permissions
{
	void EnableDebugPrivileges()
	{
		HANDLE              hToken;
		LUID                SeDebugNameValue;
		TOKEN_PRIVILEGES    TokenPrivileges;

		if (!::OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
			RaiseError("Couldn't OpenProcessToken:\n");
		
		if (!::LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &SeDebugNameValue))
		{
			::CloseHandle(hToken);
			RaiseError("Couldn't LookupPrivilegeValue:\n");
		}

		TokenPrivileges.PrivilegeCount              = 1;
		TokenPrivileges.Privileges[0].Luid          = SeDebugNameValue;
		TokenPrivileges.Privileges[0].Attributes    = SE_PRIVILEGE_ENABLED;

		if (!::AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
		{ 
			::CloseHandle(hToken);
			RaiseError("Couldn't AdjustTokenPrivileges:\n");
		}

		::CloseHandle(hToken);
	}

	bool TryToSelfElevate(HWND hwnd)
	{
		HRESULT hRes = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (hRes != S_OK)
			throw std::runtime_error("Cannot initialize COM library.");

		wchar_t szPath[MAX_PATH]; 
        if (::GetModuleFileNameW(NULL, szPath, ARRAYSIZE(szPath)) == 0)
			RaiseError("TryToSelfElevate:\n");

        SHELLEXECUTEINFOW sei = { sizeof(sei) };
		sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_UNICODE | SEE_MASK_NOASYNC | SEE_MASK_NO_CONSOLE | /*SEE_MASK_NOCLOSEPROCESS |*/ SEE_MASK_HMONITOR;
        sei.lpVerb = L"runas"; 
        sei.lpFile = szPath; 
		sei.nShow = SW_SHOW;
		sei.hMonitor = ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY); 

        if (!ShellExecuteExW(&sei)) 
        { 
            DWORD dwError = GetLastError(); 
            if (dwError != ERROR_CANCELLED)
				RaiseError("TryToSelfElevate:\n");
           
            // The user refused the elevation. 
			return false;
		}
		
		//if (sei.hProcess == nullptr)
		//	throw std::runtime_error("Cannot start new process.");

		//::WaitForSingleObject(sei.hProcess, INFINITE);

		return true;
	}

	bool IsCurrentProcessElevated() 
	{
		HANDLE hToken = NULL;
		if (!::OpenProcessToken(::GetCurrentProcess( ), TOKEN_QUERY, &hToken)) 
			RaiseError("Couldn't OpenProcessToken:\n");

		TOKEN_ELEVATION Elevation;
		DWORD cbSize = 0;
		if (!::GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) 
		{
			::CloseHandle(hToken);
			RaiseError("Couldn't GetTokenInformation:\n");
		}
				
		::CloseHandle(hToken);

		return (Elevation.TokenIsElevated != 0);
	}

	bool IsRunAsAdmin()
	{
		BOOL ret = TRUE;
		SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
		PSID AdministratorsGroup; 
		ret = ::AllocateAndInitializeSid(
					&NtAuthority,
					2,
					SECURITY_BUILTIN_DOMAIN_RID,
					DOMAIN_ALIAS_RID_ADMINS,
					0, 0, 0, 0, 0, 0,
					&AdministratorsGroup); 
		if (!ret)
			RaiseError("Couldn't AllocateAndInitializeSid:\n");
		
		BOOL check = FALSE;
		if (!::CheckTokenMembership(nullptr, AdministratorsGroup, &check))
		{
			FreeSid(AdministratorsGroup); 
			RaiseError("Couldn't CheckTokenMembership:\n");
		}

		FreeSid(AdministratorsGroup); 
		
		return (check ? true : false);
	}

	void EnableRequiredAccess(const wchar_t* className, const wchar_t* windowTitle)
	{
		HWND requiredWindow		= FindRequiredWindow(className, windowTitle, 5);
		if (requiredWindow == nullptr)
			throw std::runtime_error("Required window not found");

		// check required access to the target process 
		// (because UIPI: The thread of a process can send messages only to message queues of threads in processes of lesser or equal integrity level.)
		// and hooking mechanism uses special message (WM_TIMER) for injecting of dll
		LRESULT res = ::SendMessageW(requiredWindow, WM_TIMER, 0, 0); //ignore result
		DWORD lastError = ::GetLastError();
		if (lastError == ERROR_ACCESS_DENIED)
		{	
				
			OSVERSIONINFOW osver = { sizeof(osver) }; 
			if (::GetVersionExW(&osver) && osver.dwMajorVersion >= 6)	//under Vista ++ 
			{
				if (Permissions::IsCurrentProcessElevated())
					throw std::runtime_error("Not enough rights for controlling target process. Try run this tool as admin manually.");
				
				if (Permissions::TryToSelfElevate(requiredWindow))
					throw std::runtime_error("By user request new instance is runned as elevated process.");
			}
				
			if (!Permissions::IsRunAsAdmin())
				throw std::runtime_error("Not enough rights for controlling target process. Try run this tool as admin manually.");
		}
	}
}