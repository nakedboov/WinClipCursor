#include "FullScreen.h"

#include <assert.h>

FullScreen::FullScreen() :
	m_hwnd(NULL),
	m_fullScreen(false)
{
	SecureZeroMemory(&m_origWindowRect, sizeof(m_origWindowRect));
}

FullScreen::~FullScreen()
{
	if (m_fullScreen && m_hwnd != NULL)
		Leave();
}

void FullScreen::Init(HWND hwnd)
{
	m_hwnd = hwnd;
}

bool FullScreen::Enter()
{
	if (m_fullScreen)
		return true;

	assert(m_hwnd);
	if (m_hwnd == NULL)
		return false;
	
	HMONITOR hmon = MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi = { sizeof(mi) };

	if (!GetMonitorInfo(hmon, &mi)) 
		return false;

	if (!GetWindowRect(m_hwnd, &m_origWindowRect))
	{
		SecureZeroMemory(&m_origWindowRect, sizeof(m_origWindowRect));
		return false;
	}

	if (!SetWindowPos(m_hwnd, HWND_TOPMOST, 
					   mi.rcMonitor.left,
					   mi.rcMonitor.top,
					   mi.rcMonitor.right - mi.rcMonitor.left,
					   mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_SHOWWINDOW))
		return false;

	m_fullScreen = true;
	
	return true;
}

bool FullScreen::Leave()
{
	if (!m_fullScreen)
		return true;
	
	if (!SetWindowPos(m_hwnd, HWND_NOTOPMOST, m_origWindowRect.left, m_origWindowRect.top, 
								m_origWindowRect.right - m_origWindowRect.left, 
								m_origWindowRect.bottom - m_origWindowRect.top, SWP_SHOWWINDOW))
		return false;
	
	m_fullScreen = false;

    return true;
}