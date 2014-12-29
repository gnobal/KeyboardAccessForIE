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

// ChooseLinkDlg.h : Declaration of the CChooseLinkDlg

#ifndef __CHOOSELINKDLG_H_
#define __CHOOSELINKDLG_H_

#include "resource.h"       // main symbols


class CKeyboardAccess;


/////////////////////////////////////////////////////////////////////////////
// CChooseLinkDlg
class CChooseLinkDlg : public CDialogImpl<CChooseLinkDlg>
{
public:
    CChooseLinkDlg(CKeyboardAccess* pParent);
    ~CChooseLinkDlg();
    
    enum { IDD = IDD_CHOOSE_LINK_DLG };
    
    BEGIN_MSG_MAP(CChooseLinkDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        COMMAND_ID_HANDLER(IDOK, OnOK)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_HANDLER(IDC_LINK_NUMBER, EN_CHANGE, OnChangeLinkNumber)
        COMMAND_HANDLER(IDC_DOCS, BN_CLICKED, OnClickedDocs)
    END_MSG_MAP()

    LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnChangeLinkNumber(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedDocs(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    static LRESULT CALLBACK GetMessageProc(int nCode, WPARAM wParam, LPARAM lParam);

public:
	void EnableNewWindowButton(BOOL bEnable); // Also unchecks it, if bEnable == FALSE

private:
    CKeyboardAccess* m_pParent;
    HHOOK m_hHook;
};

#endif //__EVENTSDLG_H_
