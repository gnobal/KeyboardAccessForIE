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

// CKeyboardAccess.h : Declaration of the CKeyboardAccess

#ifndef __KeyboardAccess_H_
#define __KeyboardAccess_H_

#include "resource.h"       // main symbols
#include "ExDisp.h"
#include "ChooseLinkDlg.h"
#include "Config.h"
#include "ElementHolder.h"
#include <vector>
#include <list>

struct CElementCondition;

/////////////////////////////////////////////////////////////////////////////
// CKeyboardAccess
class ATL_NO_VTABLE CKeyboardAccess : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CKeyboardAccess, &CLSID_KeyboardAccess>,
	public IObjectWithSiteImpl<CKeyboardAccess>,
	public IDispatchImpl<IKeyboardAccess, &IID_IKeyboardAccess, &LIBID_KEYBOARDACCESSLib>
{
public:
    CKeyboardAccess() : m_dlgChoose(this), m_hHook(NULL), m_dwCookie(0)
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_KEYBOARDACCESS)
DECLARE_NOT_AGGREGATABLE(CKeyboardAccess)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CKeyboardAccess)
	COM_INTERFACE_ENTRY(IKeyboardAccess)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_IMPL(IObjectWithSite)
END_COM_MAP()

// IObjectWithSite
public:
	STDMETHOD(SetSite)(IUnknown *pUnkSite);

// IDispatch
public:    
	STDMETHOD(Invoke)(DISPID, REFIID, LCID, WORD, DISPPARAMS*, 
		              VARIANT*, EXCEPINFO*, UINT*);

public:
    HRESULT OnUIOk(long lLinkNumber, BOOL bNewWihndow);
    HRESULT OnUICancel();
    HRESULT OnUIBrowserControl(WPARAM wVirtualKey);
	HRESULT OnUIChangeLinkNumber(long lLinkNumber);
	HRESULT OnUIShowDocs();
    BOOL IsNewWindowPossible(long lLinkNumber);

private:
    typedef enum { ADVISE, UNADVISE } BROWSER_ADVISE;

    HRESULT ManageBrowserConnection(BROWSER_ADVISE eAdvise);
    HRESULT InstallKeyboardHook();
    // HRESULT InstallKeyboardHook2();
    HRESULT UninstallKeyboardHook();
    static LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);

private:
    CComQIPtr<IWebBrowser2> m_spWebBrowser2;
    HHOOK m_hHook;
    DWORD m_dwCookie;

private:
    typedef std::list<LPDISPATCH> ElementsList;
    typedef std::list<IWebBrowser2*> FramesList;
    struct ElementCondition;

	template <typename Col>
	HRESULT AppendElements(Col& pCollection,
                          ElementsList& lstTarget, const CElementCondition& condition)
	{
		HRESULT hr(S_OK);

		long lNumItems(-1);
		hr = pCollection->get_length(&lNumItems);
		ATLASSERT(SUCCEEDED(hr));
		if (FAILED(hr))
			return hr;

		LPDISPATCH pDispatch(NULL);
		for (long i=0; i<lNumItems; ++i)
		{
			CComVariant index = i;
			hr = pCollection->item(index, index, &pDispatch);
			ATLASSERT(SUCCEEDED(hr));
			if (FAILED(hr))
				return hr;

			if (condition.Test(pDispatch))
				lstTarget.push_back(pDispatch);
		}

		return S_OK;
	}

/*
		HRESULT AppendElements(CComPtr<MSHTML::IHTMLElementCollection>& pCollection,
                           ElementsList& lstTarget, const CElementCondition&);
*/
	HRESULT OnQuit();
    HRESULT InitSpanTagVariables();
    HRESULT ShowUI();
    HRESULT RemoveLinkNumbering();
    HRESULT AddLinkNumbering();
    HRESULT AddDocumentLinkNumbering(CComQIPtr<MSHTML::IHTMLDocument3>& pDocument);
    HRESULT AddAnchorNumbering(CComQIPtr<MSHTML::IHTMLDocument3>& pDocument3);
    HRESULT AddImageMapNumbering(CComQIPtr<MSHTML::IHTMLDocument3>& pDocument3);
    HRESULT AddInputFieldNumbering(CComQIPtr<MSHTML::IHTMLDocument3>& pDocument3);
    HRESULT AddNumberingToTag(const CComQIPtr<MSHTML::IHTMLElement>& pElement,
                              const CComQIPtr<MSHTML::IHTMLElement>& pCodeElement,
                              const CComBSTR& bstrWhere,
							  const CElementBehaviour& behaviour,
                              const std::string& strAdditionalStyle = std::string(""));

    HRESULT GetElementAbsolutePosition(CComQIPtr<MSHTML::IHTMLElement>& pElement,
                                       long& lTop,
                                       long& lLeft);

    HRESULT AppendFrames(FramesList& lstBrowsers);
    static HRESULT AppendFramesRec(CComQIPtr<IWebBrowser2>& pBrowser, FramesList& lstBrowsers);
private:
    CChooseLinkDlg m_dlgChoose;
    std::vector<CElementHolder> m_Elements;
    CConfig m_Config;
    std::string m_strStyleFormat;
    static const char* m_pcElementsId;
};

#endif //__KeyboardAccess_H_
