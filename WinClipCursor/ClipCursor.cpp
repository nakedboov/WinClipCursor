#include "ClipCursor.h"
#include "Utils.h"


#include <iostream>

ClipHelper::ClipHelper() :
	m_clipped(false),
	m_hwnd(NULL)
{
	SecureZeroMemory(&m_currentClipArea, sizeof(m_currentClipArea));
	SecureZeroMemory(&m_prevClipArea, sizeof(m_prevClipArea));
}

ClipHelper::~ClipHelper()
{
	if (IsClipped())
		UnClip();
}

bool ClipHelper::IsClipped()
{
	return m_clipped;
}

void ClipHelper::Init(HWND hwnd)
{
	m_hwnd = hwnd;
}

bool ClipHelper::Clip()
{
	if (m_hwnd == NULL)
		return false;

	if (m_clipped)
		return true;

	if (!::GetClipCursor(&m_prevClipArea))
	{
		std::string description;
		DWORD lastError = GetLastError();
		GetErrorDescription(lastError, description);
		throw std::runtime_error(description);
	}

	RECT rect;
	if (!CalcRequiredClipRect(m_hwnd, rect))
		return false;

	if (!::ClipCursor(&rect))
	{
		std::string description;
		DWORD lastError = GetLastError();
		GetErrorDescription(lastError, description);
		throw std::runtime_error(description);
	}

	m_currentClipArea = rect;
	m_clipped = true;

	return true;
}

bool ClipHelper::UnClip()
{
	if (!m_clipped)
		return true;

	if (!::ClipCursor(&m_prevClipArea))
	{
		std::string description;
		DWORD lastError = GetLastError();
		GetErrorDescription(lastError, description);
		throw std::runtime_error(description);
	}
		
	SecureZeroMemory(&m_currentClipArea, sizeof(m_currentClipArea));
	SecureZeroMemory(&m_prevClipArea, sizeof(m_prevClipArea));

	m_clipped = false;

	return true;
}