#pragma once

#include <Windows.h>
#include <string>

#include "ClipCursor.h"
#include "FullScreen.h"

namespace ClipStateType
{
	enum
	{
		ClipStateUnknown = 0,
		ClipStateActivate = 1,
		ClipStateDeactivate = 2
	};

	typedef unsigned int Type;
}

class Controller
{
public:

	explicit Controller(HWND hWnd, const std::wstring& className, const std::wstring& winTitle);
	~Controller();

	void RunPollingLoop();
	void SetClipHook();
	
	ClipHelper& ClipCursorHelper() { return m_clipHelper; }
	FullScreen&	FullScreenHelper() { return m_fullScreen; }

	ClipStateType::Type GetClipState();
	void SetClipState(ClipStateType::Type state);

	static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	
private:
	
	typedef BOOL (*SetWinHookPtr)(HWND hWndSrv, HWND hWndTrg, DWORD threadId);
	typedef BOOL (*ClearWinHookPtr)();

	ClipStateType::Type	m_ClipState;

	HWND				m_hWndServer;
	HMODULE				m_hWinHookModule;
	
	SetWinHookPtr		m_pSetWinHooks;
	ClearWinHookPtr		m_pClearWinHooks;

	std::wstring m_className;
	std::wstring m_winTitle;

	ClipHelper			m_clipHelper;
	FullScreen			m_fullScreen;
};