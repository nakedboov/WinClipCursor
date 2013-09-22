#pragma once

#include <Windows.h>
#include <string>

#include "ClipCursor.h"
#include "FullScreen.h"

class Controller
{
public:

	explicit Controller(const std::string& className, const std::string& winTitle);
	explicit Controller(HWND hWnd, const std::string& className, const std::string& winTitle);
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
	
	SetWinHookPtr		m_pSetWinHook;
	ClearWinHookPtr		m_pClearWinHook;

	std::string m_className;
	std::string m_winTitle;

	ClipHelper			m_clipHelper;
	FullScreen			m_fullScreen;
};