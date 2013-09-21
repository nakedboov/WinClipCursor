#pragma once

#include <Windows.h>

class ClipHelper
{
public:

	ClipHelper();
	~ClipHelper();

	void Init(HWND hwnd);

	bool Clip();
	bool UnClip();
	bool IsClipped();
	
private:

	bool	m_clipped;
	HWND	m_hwnd;

	RECT m_currentClipArea;
	RECT m_prevClipArea;       
};