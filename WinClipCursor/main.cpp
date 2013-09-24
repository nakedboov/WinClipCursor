#include <windows.h>

#include "Controller.h"
#include "Utils.h"

#include <iostream>
#include <string>
#include <memory>

const wchar_t* g_ClassName = L"Warcraft III";
const wchar_t* g_WindowTitle = L"Warcraft III";
extern const unsigned int g_SleepTimeOut = 500;

std::shared_ptr<Controller> g_ControllerPtr = 0;

void StartPollingVersion();
void StartHookVersion();

int main(int argc, char* argv[])
{	
	std::wcout << "start clipping the window - " << g_WindowTitle << std::endl;

	try
	{
		//StartPollingVersion();
		StartHookVersion();
	}
	catch(const std::exception& e)
	{
		std::wcout << e.what() << std::endl;
	}

	std::wcout << "end clipping the window - " << g_WindowTitle << std::endl;
	
	return 0;
}

void StartPollingVersion()
{
	g_ControllerPtr.reset(new Controller(nullptr, g_ClassName, g_WindowTitle));
	g_ControllerPtr->RunPollingLoop();
}

void StartHookVersion()
{
	const wchar_t* className = L"ClipServerWindowClass";
	WNDCLASSEXW wndclass = 
	{ 
		sizeof(WNDCLASSEX), 
		CS_DBLCLKS, 
		Controller::MainWndProc,
		0, 0, 
		GetModuleHandleW(nullptr), 
		0,
		0, 
		0,
		0, 
		className, 
		0 
	};

	if (!RegisterClassExW(&wndclass))
	{
		std::string description;
		GetErrorDescription(GetLastError(), description);
		std::cout << description << std::endl;
		return;
	}

	HWND hWndSrv = CreateWindowExW(0, className, L"ClipServer Window",
			WS_CHILDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT, HWND_MESSAGE, nullptr, GetModuleHandleW(nullptr), nullptr);
		
	if (!hWndSrv)
	{
		std::string description;
		GetErrorDescription(GetLastError(), description);
		std::cout << description << std::endl;
		return;
	}

	ShowWindow(hWndSrv, SW_HIDE); 
    UpdateWindow(hWndSrv); 

	g_ControllerPtr.reset(new Controller(hWndSrv, g_ClassName, g_WindowTitle));

	g_ControllerPtr->SetClipHook();

	MSG msg;
	BOOL fGotMessage;
	while ((fGotMessage = GetMessageW(&msg, nullptr, 0, 0)) != 0 && fGotMessage != -1)  
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}