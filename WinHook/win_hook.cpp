// WinHook.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "../WinClipCursor/UserMsg.h"

//shared section (data shared by all process)
#pragma data_seg(".CWH")
static HWND g_hWndSrv = nullptr;
static HWND g_hWndTarget = nullptr;
#pragma data_seg()
#pragma comment(linker, "/section:.CWH,rws")

extern HINSTANCE g_hInst;

static LRESULT CALLBACK CallWndHookProc(UINT nCode, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK MouseHookProc(UINT nCode, WPARAM wParam, LPARAM lParam);
BOOL ClearWinHooks();

static HHOOK g_hCallWndHook = nullptr;
static HHOOK g_hMouseHook = nullptr;

BOOL SetWinHooks(HWND hWndSrv, HWND hWndTrg, DWORD threadId)
{
	if (g_hWndSrv != nullptr)
		return FALSE; //already hooked
	
	g_hCallWndHook = SetWindowsHookExW(WH_CALLWNDPROC, (HOOKPROC)CallWndHookProc, g_hInst, threadId);
	if (g_hCallWndHook != nullptr)
	{ 
		g_hMouseHook = SetWindowsHookExW(WH_MOUSE, (HOOKPROC)MouseHookProc, g_hInst, threadId);
		if (g_hMouseHook != nullptr)
		{
			g_hWndSrv = hWndSrv;
			g_hWndTarget = hWndTrg;
			return TRUE;
		}
		ClearWinHooks();
	}

	return FALSE;
}

BOOL ClearWinHooks()
{
	if (g_hCallWndHook != nullptr)
	{	
		BOOL res = UnhookWindowsHookEx(g_hCallWndHook);
		if (res)
			g_hCallWndHook = nullptr;
	}

	if (g_hMouseHook != nullptr)
	{	
		BOOL res = UnhookWindowsHookEx(g_hMouseHook);
		if (res)
			g_hMouseHook = nullptr;
	}

	if ((g_hCallWndHook == nullptr) && (g_hMouseHook == nullptr))
	{
		g_hWndSrv = nullptr;
		g_hWndTarget = nullptr;
		return TRUE;
	}

	return FALSE;
}

static LRESULT CALLBACK CallWndHookProc(UINT nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode < 0)
		return CallNextHookEx(nullptr, nCode, wParam, lParam);
	
	if (nCode == HC_ACTION)
	{
		PCWPSTRUCT swpStruct = (PCWPSTRUCT)lParam;

		//skip messages from childs
		if (g_hWndTarget == swpStruct->hwnd)
		{
			switch(swpStruct->message)
			{
			case WM_ACTIVATE:
				PostMessageW(g_hWndSrv, WMU_ACTIVATE, swpStruct->wParam, swpStruct->lParam);
				break;
			case WM_DESTROY:
				PostMessageW(g_hWndSrv, WMU_DESTROY, (WPARAM)swpStruct->hwnd, swpStruct->lParam);
				break;
			}
		}
	}

	return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

static LRESULT CALLBACK MouseHookProc(UINT nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode < 0)
		return CallNextHookEx(nullptr, nCode, wParam, lParam);

	if (nCode == HC_ACTION)
	{
		PMOUSEHOOKSTRUCT pmsStruct = (PMOUSEHOOKSTRUCT)lParam;

		//skip messages from childs
		if (g_hWndTarget == pmsStruct->hwnd)
		{
			switch (wParam)
			{
			case WM_LBUTTONDOWN:
				PostMessageW(g_hWndSrv, WMU_LBUTTONDOWN, wParam, lParam);
				break;
			}
		}
	}

	return CallNextHookEx(nullptr, nCode, wParam, lParam);
}