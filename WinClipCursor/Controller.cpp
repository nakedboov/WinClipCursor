#include "Controller.h"

#include "Utils.h"

#include <iostream>
#include <memory>

extern const unsigned int g_SleepTimeOut;
extern std::shared_ptr<Controller> g_ControllerPtr;

bool Controller::gs_ActivateClip = false;

Controller::Controller(const std::string& className, const std::string& winTitle):
	m_hWndServer(NULL),
	m_hWinHookModule(NULL),
	m_pSetWinHook(0),
	m_pClearWinHook(0),
	m_className(className),
	m_winTitle(winTitle)
{
}

Controller::Controller(HWND hWnd, const std::string& className, const std::string& winTitle):
	m_hWndServer(hWnd),
	m_hWinHookModule(NULL),
	m_pSetWinHook(0),
	m_pClearWinHook(0),
	m_className(className),
	m_winTitle(winTitle)
{
}

Controller::~Controller()
{
	if (m_pClearWinHook != 0)
		m_pClearWinHook();

	if (m_hWinHookModule != NULL)
		FreeLibrary(m_hWinHookModule);
}

/* poolling version */
void Controller::RunPoollingLoop()
{		
	while (true)
	{
		HWND activeWindow		= GetForegroundWindow();
		HWND requiredWindow		= FindRequiredWindow(m_className, m_winTitle, 5);

		if (requiredWindow == NULL)
			throw std::runtime_error("Required window not found");
		
		m_fullScreen.Init(requiredWindow);
		m_clipHelper.Init(requiredWindow);

		if (activeWindow == requiredWindow)
		{
			if (m_clipHelper.IsClipped() || !CursorInClientArea(requiredWindow))
			{
				Sleep(g_SleepTimeOut);
				continue;
			}

			if (m_fullScreen.Enter()) 
			{	
				DEBUG_TRACE("EnterFullscreen success"); 
				m_clipHelper.Clip();
				DEBUG_TRACE("Clip");
			}
			else
			{	DEBUG_TRACE("EnterFullscreen failed"); }
		}
		else
		{
			if (m_clipHelper.IsClipped())
			{
				if (m_fullScreen.Leave())
				{ DEBUG_TRACE("LeaveFullscreen success"); }
				else
				{ DEBUG_TRACE("LeaveFullscreen failed"); }

				m_clipHelper.UnClip();
				DEBUG_TRACE("UnClip");
			}

			Sleep(g_SleepTimeOut);
		}
	}
}


void Controller::SetClipHook()
{
	HWND requiredWindow		= FindRequiredWindow(m_className, m_winTitle, 5);
	if (requiredWindow == NULL)
		throw std::runtime_error("Required window not found");
		
	m_fullScreen.Init(requiredWindow);
	m_clipHelper.Init(requiredWindow);

	m_hWinHookModule = LoadLibraryA("WinHook.dll");
	if (m_hWinHookModule == NULL)
		throw std::runtime_error("WinHook.dll not found.");

	m_pSetWinHook = (SetWinHookPtr)GetProcAddress(m_hWinHookModule, "SetWinHook");
	m_pClearWinHook = (ClearWinHookPtr)GetProcAddress(m_hWinHookModule, "ClearWinHook");

	if ((m_pSetWinHook == 0) || (m_pClearWinHook == 0))
		throw std::runtime_error("Invalid WinHook.dll");

	DWORD processId = 0;
	DWORD threadId = GetWindowThreadProcessId(requiredWindow, &processId);
	
	if (!m_pSetWinHook(m_hWndServer, threadId))
	{	
		std::string description;
		GetErrorDescription(GetLastError(), description);
		
		std::string msg("Hook failed:\n");
		msg.append(description);
		throw std::runtime_error(msg);
	}
}

LRESULT CALLBACK Controller::MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_ACTIVATE) 
    { 
		switch (wParam)
		{
		case WA_ACTIVE:
			DEBUG_TRACE("WA_ACTIVE");
			gs_ActivateClip = true;
			break;
		case WA_CLICKACTIVE:
			DEBUG_TRACE("WA_CLICKACTIVE");
			gs_ActivateClip = true;
			break;
		case WA_INACTIVE:
			DEBUG_TRACE("WA_INACTIVE");
			gs_ActivateClip = false;
			if (g_ControllerPtr->ClipCursorHelper().IsClipped())
			{
				if (g_ControllerPtr->FullScreenHelper().Leave())
				{ DEBUG_TRACE("LeaveFullscreen success"); }
				else
				{ DEBUG_TRACE("LeaveFullscreen failed"); }

				g_ControllerPtr->ClipCursorHelper().UnClip();
				DEBUG_TRACE("UnClip");
			}
			break;
		}
		return 0;
	}
	else if (uMsg == WM_LBUTTONDOWN)
	{
		DEBUG_TRACE("WM_LBUTTONDOWN");
		
		if (!gs_ActivateClip)
			return 0;

		if (g_ControllerPtr->ClipCursorHelper().IsClipped())
			return 0;

		if (g_ControllerPtr->FullScreenHelper().Enter()) 
		{	
			DEBUG_TRACE("EnterFullscreen success"); 
			g_ControllerPtr->ClipCursorHelper().Clip();
			DEBUG_TRACE("Clip");
		}
		else
		{	DEBUG_TRACE("EnterFullscreen failed"); }
		
		return 0;
	}
	
    return DefWindowProc(hwnd, uMsg, wParam, lParam); 
}