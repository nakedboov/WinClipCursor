#pragma once

#include <Windows.h>
#include <string>

#include "ClipCursor.h"
#include "FullScreen.h"

class Controller
{
public:

	explicit Controller(HWND hWnd, const std::wstring& className, const std::wstring& winTitle);
	~Controller();

	void RunPollingLoop();
	void SetClipHook();

	ClipHelper& ClipCursorHelper() { return m_clipHelper; }
	FullScreen&	FullScreenHelper() { return m_fullScreen; }

	static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static bool gs_ActivateClip;

private:
	
	typedef BOOL (*SetWinHookPtr)(HWND hWnd, DWORD threadId);
	typedef BOOL (*ClearWinHookPtr)();

	HWND				m_hWndServer;
	HMODULE				m_hWinHookModule;
	
	SetWinHookPtr		m_pSetWinHooks;
	ClearWinHookPtr		m_pClearWinHooks;

	std::wstring m_className;
	std::wstring m_winTitle;

	ClipHelper			m_clipHelper;
	FullScreen			m_fullScreen;
};