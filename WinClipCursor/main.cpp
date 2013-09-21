#include <windows.h>

#include "Controller.h"
#include "Utils.h"

#include <iostream>
#include <string>
#include <memory>

const char* g_ClassName = "Warcraft III";
const char* g_WindowTitle = "Warcraft III";
extern const unsigned int g_SleepTimeOut = 500;

std::shared_ptr<Controller> g_ControllerPtr = 0;

void StartPollingVersion();
void StartHookVersion();

int main(int argc, char* argv[])
{	
	std::cout << "Starting to clip the window - " << g_WindowTitle << std::endl;

	try
	{
		//StartPollingVersion();
		StartHookVersion();
	}
	catch(const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

	std::cout << "Ending to clip the window - " << g_WindowTitle << std::endl;
	
	return 0;
}

void StartPollingVersion()
{
	g_ControllerPtr.reset(new Controller(g_ClassName, g_WindowTitle));
	g_ControllerPtr->RunPoollingLoop();
}

void StartHookVersion()
{
	const char* className = "ClipServerWindowClass";
	WNDCLASSEX wndclass = 
	{ 
		sizeof(WNDCLASSEX), 
		CS_DBLCLKS, 
		Controller::MainWndProc,
		0, 0, 
		GetModuleHandle(0), 
		0,
		0, 
		0,
		0, 
		className, 
		0 
	};

	if (!RegisterClassEx(&wndclass))
	{
		std::string description;
		GetErrorDescription(GetLastError(), description);
		std::cout << description << std::endl;
		return;
	}

	HWND hWndSrv = CreateWindowEx(0, className, "ClipServer Window",
			WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, GetModuleHandle(0), 0);
		
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
	while ((fGotMessage = GetMessage(&msg, (HWND) NULL, 0, 0)) != 0 && fGotMessage != -1)  
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}