/*
 *	Copyright (C) 2002, 2003 Amit Schreiber <gnobal@yahoo.com>
 *
 *	This file is part of KeyboardAccess for IE.
 *
 *	KeyboardAccess for IE is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	KeyboardAccess for IE is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with KeyboardAccess for IE; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// ChooseLinkDlg.cpp : Implementation of CChooseLinkDlg
#include "stdafx.h"
#include "ChooseLinkDlg.h"

#include "KeyboardAccess.h"
#include "CKeyboardAccess.h"
#include "ThreadSafeMap.h"

static CThreadSafeMap<DWORD, CChooseLinkDlg*> g_mapThreadDlgs;
/////////////////////////////////////////////////////////////////////////////
// CChooseLinkDlg

CChooseLinkDlg::CChooseLinkDlg(CKeyboardAccess* pParent) : m_pParent(pParent), m_hHook(NULL) //, m_wndEdit("Edit", this, 1)
{
}

CChooseLinkDlg::~CChooseLinkDlg()
{
}

LRESULT CChooseLinkDlg::OnOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    long lLinkNumber = ::GetDlgItemInt(m_hWnd, IDC_LINK_NUMBER, NULL, FALSE);
    BOOL bNewWindow = ::IsDlgButtonChecked(m_hWnd, IDC_NEW_WINDOW);

	m_pParent->OnUIOk(lLinkNumber, bNewWindow);
    
    DestroyWindow();
    return 0;
}

LRESULT CChooseLinkDlg::OnCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    m_pParent->OnUICancel();
    DestroyWindow();
    return 0;
}

LRESULT CChooseLinkDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    const DWORD dwCurrThreadId = GetCurrentThreadId();
    g_mapThreadDlgs.insert(std::make_pair(dwCurrThreadId, this));

    m_hHook = ::SetWindowsHookEx(
        WH_GETMESSAGE,
        GetMessageProc,
        _Module.m_hInst,
        dwCurrThreadId);

    CenterWindow();
    return 0;
}



LRESULT CChooseLinkDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    UnhookWindowsHookEx(m_hHook);
    g_mapThreadDlgs.erase(GetCurrentThreadId());

    return 0;
}

LRESULT CChooseLinkDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    DestroyWindow();
    return 0;
}

LRESULT CChooseLinkDlg::OnChangeLinkNumber(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    long lLinkNumber = ::GetDlgItemInt(m_hWnd, IDC_LINK_NUMBER, NULL, FALSE);
	m_pParent->OnUIChangeLinkNumber(lLinkNumber);
	
    return 0;
}

LRESULT CChooseLinkDlg::OnClickedDocs(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_pParent->OnUIShowDocs();
	return 0;
}

void CChooseLinkDlg::EnableNewWindowButton(BOOL bEnable)
{
	HWND hwndNewWindow = ::GetDlgItem(m_hWnd, IDC_NEW_WINDOW);
	if (!bEnable)
	{
		::CheckDlgButton(m_hWnd, IDC_NEW_WINDOW, BST_UNCHECKED);
	}
	::EnableWindow(hwndNewWindow, bEnable);

    return;
}


LRESULT CALLBACK CChooseLinkDlg::GetMessageProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    CChooseLinkDlg* pThis = g_mapThreadDlgs.find(GetCurrentThreadId())->second;

    // If this is a keystrokes message, translate it in controls'
    // PreTranslateMessage().
    LPMSG lpMsg = (LPMSG) lParam;
    if( nCode >= 0 && 
        PM_REMOVE == wParam &&
        lpMsg->message >= WM_KEYFIRST && 
        lpMsg->message <= WM_KEYLAST &&
        ::IsDialogMessage(pThis->m_hWnd, lpMsg))
    {
        if (lpMsg->message == WM_SYSKEYDOWN) 
        {
            if (lpMsg->wParam == VK_UP || lpMsg->wParam == VK_DOWN && 
                lpMsg->lParam & 0x20000000)
            {
                pThis->m_pParent->OnUIBrowserControl(lpMsg->wParam);
            }
        }

        // The value returned from this hookproc is ignored, and it cannot
        // be used to tell Windows the message has been handled. To avoid
        // further processing, convert the message to WM_NULL before
        // returning.
        lpMsg->hwnd = NULL;
        lpMsg->message = WM_NULL;
        lpMsg->lParam = 0L;
        lpMsg->wParam = 0;
  }

    // Passes the hook information to the next hook procedure in
    // the current hook chain.
    return ::CallNextHookEx(pThis->m_hHook, nCode, wParam, lParam);
}
