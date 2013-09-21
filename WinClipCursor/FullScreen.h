#pragma once

#include <Windows.h>

class FullScreen
{
public:

	FullScreen();
	~FullScreen();

	void Init(HWND hwnd);
	bool Enter();
	bool Leave();

private:

	RECT m_origWindowRect;
	HWND m_hwnd;
	bool m_fullScreen;
};