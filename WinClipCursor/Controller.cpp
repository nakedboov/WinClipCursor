#include "Controller.h"

#include "Utils.h"
#include "UserMsg.h"

#include <iostream>
#include <memory>

extern const unsigned int g_SleepTimeOut;
extern std::shared_ptr<Controller> g_ControllerPtr;

Controller::Controller(HWND hWnd, const std::wstring& className, const std::wstring& winTitle):
	m_ClipState(ClipStateType::ClipStateUnknown),
	m_hWndServer(hWnd),
	m_hWinHookModule(nullptr),
	m_pSetWinHooks(nullptr),
	m_pClearWinHooks(nullptr),
	m_className(className),
	m_winTitle(winTitle)
{
}

Controller::~Controller()
{
	if (m_pClearWinHooks != nullptr)
		m_pClearWinHooks();

	if (m_hWinHookModule != nullptr)
		FreeLibrary(m_hWinHookModule);
}

/* polling version */
void Controller::RunPollingLoop()
{		
	while (true)
	{
		HWND activeWindow		= GetForegroundWindow();
		HWND requiredWindow		= FindRequiredWindow(m_className, m_winTitle, 5);

		if (requiredWindow == nullptr)
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
	if (requiredWindow == nullptr)
		throw std::runtime_error("Required window not found");
		
	m_fullScreen.Init(requiredWindow);
	m_clipHelper.Init(requiredWindow);

	m_hWinHookModule = LoadLibraryW(L"WinHook.dll");
	if (m_hWinHookModule == nullptr)
		throw std::runtime_error("WinHook.dll not found.");

	m_pSetWinHooks = (SetWinHookPtr)GetProcAddress(m_hWinHookModule, "SetWinHooks");
	m_pClearWinHooks = (ClearWinHookPtr)GetProcAddress(m_hWinHookModule, "ClearWinHooks");

	if ((m_pSetWinHooks == nullptr) || (m_pClearWinHooks == nullptr))
		throw std::runtime_error("Invalid WinHook.dll");

	DWORD processId = 0;
	DWORD threadId = GetWindowThreadProcessId(requiredWindow, &processId);
	
	if (!m_pSetWinHooks(m_hWndServer, requiredWindow, threadId))
	{	
		std::string description;
		GetErrorDescription(GetLastError(), description);
		
		std::string msg("Hook failed:\n");
		msg.append(description);
		throw std::runtime_error(msg);
	}
}

ClipStateType::Type Controller::GetClipState()
{
	return m_ClipState;
}

void Controller::SetClipState(ClipStateType::Type state)
{
	m_ClipState = state;
}

LRESULT CALLBACK Controller::MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WMU_ACTIVATE) 
    { 
		switch (wParam)
		{
		case WA_ACTIVE:
			DEBUG_TRACE("WA_ACTIVE");
			g_ControllerPtr->SetClipState(ClipStateType::ClipStateActivate);
			break;
		case WA_CLICKACTIVE:
			DEBUG_TRACE("WA_CLICKACTIVE");
			g_ControllerPtr->SetClipState(ClipStateType::ClipStateActivate);
			break;
		case WA_INACTIVE:
			DEBUG_TRACE("WA_INACTIVE");
			g_ControllerPtr->SetClipState(ClipStateType::ClipStateDeactivate);
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
	else if (uMsg == WMU_LBUTTONDOWN)
	{
		DEBUG_TRACE("WM_LBUTTONDOWN");
		
		if (g_ControllerPtr->GetClipState() == ClipStateType::ClipStateDeactivate)
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
	else if (uMsg == WMU_DESTROY)
	{
		DEBUG_TRACE("WM_DESTROY");
		PostQuitMessage(0); 
		return 0;
	}
	
    return DefWindowProc(hwnd, uMsg, wParam, lParam); 
}