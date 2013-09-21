// WinHook.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

//shared section (data shared by all process)
#pragma data_seg(".CWH")
static HHOOK g_hCallWndHook = NULL;
static HHOOK g_hMouseHook = NULL;
static HWND g_hWndSrv = NULL;
#pragma data_seg()
#pragma comment(linker, "/section:.CWH,rws")

extern HINSTANCE g_hInst;

static LRESULT CALLBACK CallWndHookProc(UINT nCode, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK MouseHookProc(UINT nCode, WPARAM wParam, LPARAM lParam);
BOOL ClearWinHook();

BOOL SetWinHook(HWND hWnd, DWORD threadId)
{
	if (g_hWndSrv != NULL)
		return FALSE; //already hooked
	
	g_hCallWndHook = SetWindowsHookEx(WH_CALLWNDPROC, (HOOKPROC)CallWndHookProc, g_hInst, threadId);
	if (g_hCallWndHook != NULL)
	{ 
		g_hMouseHook = SetWindowsHookEx(WH_MOUSE, (HOOKPROC)MouseHookProc, g_hInst, threadId);
		if (g_hMouseHook != NULL)
		{
			g_hWndSrv = hWnd;
			return TRUE;
		}
		ClearWinHook();
	}

	return FALSE;
}

BOOL ClearWinHook()
{
	BOOL res = TRUE;

	if (g_hCallWndHook != NULL)
	{	
		res = UnhookWindowsHookEx(g_hCallWndHook);
		if (res)
			g_hCallWndHook = NULL;
	}

	if (g_hMouseHook != NULL)
	{	
		res = UnhookWindowsHookEx(g_hMouseHook);
		if (res)
			g_hMouseHook = NULL;
	}

	return res;
}

static LRESULT CALLBACK CallWndHookProc(UINT nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode < 0)
		return CallNextHookEx(g_hCallWndHook, nCode, wParam, lParam);
	
	if (nCode == HC_ACTION)
	{
		PCWPSTRUCT swpStruct = (PCWPSTRUCT)lParam;
		switch(swpStruct->message)
		{
		case WM_ACTIVATE:
			PostMessage(g_hWndSrv, WM_ACTIVATE, swpStruct->wParam, swpStruct->lParam);
			break;
		}
	}

	return CallNextHookEx(g_hCallWndHook, nCode, wParam, lParam);
}

static LRESULT CALLBACK MouseHookProc(UINT nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode < 0)
		return CallNextHookEx(g_hCallWndHook, nCode, wParam, lParam);

	if (nCode == HC_ACTION)
	{
		switch (wParam)
		{
		case WM_LBUTTONDOWN:
			PostMessage(g_hWndSrv, WM_LBUTTONDOWN, wParam, lParam);
			break;
		}
	}

	return CallNextHookEx(g_hCallWndHook, nCode, wParam, lParam);
}