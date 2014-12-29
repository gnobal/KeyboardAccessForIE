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

// CKeyboardAccess.cpp : Implementation of CKeyboardAccess
#include "stdafx.h"
#include "KeyboardAccess.h"
#include "CKeyboardAccess.h"
#include "ExDispID.h"
// #include "MSHTMDID.h"

#include <strstream>
#include "ThreadSafeMap.h"
#include "ElementBehaviour.h"
#include "ElementCondition.h"

using std::string;
using std::strstream;

/////////////////////////////////////////////////////////////////////////////
// CKeyboardAccess
const char* CKeyboardAccess::m_pcElementsId = "KA";

static CThreadSafeMap<DWORD, CKeyboardAccess *> g_mapThreadBHOs;

HRESULT CKeyboardAccess::SetSite(IUnknown *pUnkSite)
{
    m_spWebBrowser2 = pUnkSite;
    if (m_spWebBrowser2 == NULL)
        return E_INVALIDARG;

    HRESULT hr = InstallKeyboardHook();
    if (FAILED(hr))
        return hr;

    hr = ManageBrowserConnection(ADVISE);
    ATLASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        UninstallKeyboardHook();
        return hr;
    }
/*    
    hr = InstallKeyboardHook2();
    if (FAILED(hr))
        return hr;
*/
    hr = m_Config.Create();
    ATLASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        ManageBrowserConnection(UNADVISE);
        UninstallKeyboardHook();
        return hr;
    }

    hr = InitSpanTagVariables();
    ATLASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        ManageBrowserConnection(UNADVISE);
        UninstallKeyboardHook();
        return hr;
    }

    return S_OK;
}

HRESULT CKeyboardAccess::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
	DISPPARAMS* pDispParams, VARIANT* pvarResult,
    EXCEPINFO*  pExcepInfo,  UINT* puArgErr)
{
    if (!pDispParams)
        return E_INVALIDARG;
    
    switch (dispidMember)
    {
    case DISPID_ONQUIT:
        OnQuit();
        break;
        
    default:
        break;
    }
    
    return S_OK;
}

HRESULT CKeyboardAccess::OnQuit()
{
    ManageBrowserConnection(UNADVISE);
    UninstallKeyboardHook();
    return S_OK;
}

HRESULT CKeyboardAccess::InstallKeyboardHook()
{
    const DWORD dwCurrThreadId = GetCurrentThreadId();
    g_mapThreadBHOs.insert(std::make_pair(dwCurrThreadId, this));
	
    m_hHook = SetWindowsHookEx( WH_KEYBOARD, 
			reinterpret_cast<HOOKPROC>(KeyboardHookProc), 
			NULL, 
			dwCurrThreadId );

	return S_OK;

}

HRESULT CKeyboardAccess::UninstallKeyboardHook()
{
    UnhookWindowsHookEx(m_hHook);
    g_mapThreadBHOs.erase(GetCurrentThreadId());

	return S_OK;
}


LRESULT CALLBACK CKeyboardAccess::KeyboardHookProc(int nCode, 
	WPARAM wParam, LPARAM lParam)
{
    CKeyboardAccess* pThis = g_mapThreadBHOs.find(GetCurrentThreadId())->second;
	// Typical start-off for any hook
	if( nCode <0 )
		return CallNextHookEx(pThis->m_hHook, nCode, wParam, lParam);

	// Process the key only once
	if ((lParam & 0x80000000) || (lParam & 0x40000000))
		return CallNextHookEx(pThis->m_hHook, nCode, wParam, lParam);

	if ((wParam==VK_ADD || wParam==VK_F12) &&
        !pThis->m_dlgChoose.IsWindow()) // Choose link window not showing
    {
        pThis->ShowUI();
    }

    return CallNextHookEx(pThis->m_hHook, nCode, wParam, lParam);
}

HRESULT CKeyboardAccess::ManageBrowserConnection(BROWSER_ADVISE eAdvise)
{
    CComQIPtr<IConnectionPointContainer> spCPC;
    CComPtr<IConnectionPoint> spConnectionPoint;
    
    spCPC = m_spWebBrowser2;
    if (spCPC == NULL)
        return E_POINTER;

    HRESULT hr = spCPC->FindConnectionPoint(DIID_DWebBrowserEvents2, &spConnectionPoint);
    spCPC = (IConnectionPointContainer*) NULL;
    if (FAILED(hr))
        return hr;

    if (eAdvise == ADVISE)
    {
        hr = spConnectionPoint->Advise((IDispatch*)this, &m_dwCookie);
        if (FAILED(hr))
        {
            ATLTRACE("ManageBrowserConnection(): Failed to Advise\n\n");
            return hr;
        }
    }
    else
    {
        hr = spConnectionPoint->Unadvise(m_dwCookie);
        if (FAILED(hr))
        {
            ATLTRACE("ManageBrowserConnection(): Failed to Unadvise\n\n");
            return hr;
        }
    }

    return hr;
}

HRESULT CKeyboardAccess::InitSpanTagVariables()
{
    USES_CONVERSION;

    m_strStyleFormat.append("<span id=");
    m_strStyleFormat.append(m_pcElementsId);
    m_strStyleFormat.append(" style='%s");
    CComBSTR bstrStyle;
    m_Config.GetStyleString(bstrStyle);
    m_strStyleFormat.append(OLE2A(bstrStyle));
    m_strStyleFormat.append("'>%d");
    m_strStyleFormat.append("</span>");

    return S_OK;
}


HRESULT CKeyboardAccess::ShowUI()
{
    HRESULT hr(S_OK);

    hr = AddLinkNumbering();
    if (FAILED(hr))
        return hr;

    if (m_Elements.empty()) // No links on page
        return S_OK;

    // Show choose link dialog
    HWND hwndBrowser(NULL);
    hr = m_spWebBrowser2->get_HWND((long*) &hwndBrowser);
    ATLASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

	if (!IsWindow(hwndBrowser))
		return E_POINTER;

    m_dlgChoose.Create(hwndBrowser);
    m_dlgChoose.ShowWindow(SW_SHOW);

    m_dlgChoose.SetFocus();

    return S_OK;
}

HRESULT CKeyboardAccess::AddLinkNumbering()
{
    m_Elements.clear();
    HRESULT hr(S_OK);

    FramesList lstFrames;
    AppendFrames(lstFrames);
    
    for(FramesList::iterator frameIt = lstFrames.begin();
        frameIt != lstFrames.end();
        ++frameIt)
    {
        IWebBrowser2* pCurrBrowser = *frameIt;

        CComPtr<IDispatch> pDispatch;
        hr = pCurrBrowser->get_Document(&pDispatch);
        ATLASSERT(SUCCEEDED(hr));

        pCurrBrowser->Release();
        pCurrBrowser = NULL; // Unused later
        if (FAILED(hr))
            continue; // Must continue to release all browser pointers

        CComQIPtr<MSHTML::IHTMLDocument3> pDocument = pDispatch;
		// Can be NULL if inside Windows Explorer
        if (pDocument == NULL)
            return E_POINTER;
        
        pDispatch = NULL;

        hr = AddDocumentLinkNumbering(pDocument);
        ATLASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return hr;

        pDocument = (MSHTML::IHTMLDocument3*) NULL;
    }

    return S_OK;
}

HRESULT CKeyboardAccess::AddAnchorNumbering(CComQIPtr<MSHTML::IHTMLDocument3>& pDocument3)
{
    CComPtr<MSHTML::IHTMLElementCollection> pElements;
    HRESULT hr = pDocument3->getElementsByTagName(CComBSTR("a"), &pElements);
    ATLASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;
    
    ElementsList lstAnchors;
    hr = AppendElements(pElements, lstAnchors, CElementConditionFactory::GetAnchorCondition());
    if (FAILED(hr))
        return hr;
    
    pElements = NULL;
    // m_Elements.reserve(m_Elements.size() + lstAnchors.size());
    ElementsList::iterator itAnchorsEnd = lstAnchors.end();

    const CComBSTR bstrBeforeEnd("beforeEnd");
    for (ElementsList::iterator it = lstAnchors.begin(); it != itAnchorsEnd; ++it)
    {
        const CComQIPtr<MSHTML::IHTMLElement> pCurrAnchor = *it;
        hr = AddNumberingToTag(pCurrAnchor, pCurrAnchor, bstrBeforeEnd, CElementBehaviourFactory::GetAnchorBehaviour());
        ATLASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return hr;
    }

    return S_OK;
}

HRESULT CKeyboardAccess::AddImageMapNumbering(CComQIPtr<MSHTML::IHTMLDocument3>& pDocument3)
{
    // Add <img> elements that use image maps
    CComQIPtr<MSHTML::IHTMLDocument2> pDocument2 = pDocument3;
    ATLASSERT(pDocument2 != NULL);
    if (pDocument2 == NULL)
        return E_POINTER;

	// Get all images
    CComPtr<MSHTML::IHTMLElementCollection> pImages;
    HRESULT hr = pDocument2->get_images(&pImages);
    ATLASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

    pDocument2 = (MSHTML::IHTMLDocument2*) NULL;

    ElementsList lstImages;
    hr = AppendElements(pImages, lstImages, CElementConditionFactory::GetAreaCondition());
    ATLASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

    pImages = NULL;
	
	// Get all image maps
	CComPtr<MSHTML::IHTMLElementCollection> pImageMaps;
    hr = pDocument3->getElementsByTagName(CComBSTR("map"), &pImageMaps);
    ATLASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

	ElementsList lstImageMaps;
	AppendElements(pImageMaps, lstImageMaps, CAllElements());
	pImageMaps = NULL;

    for (ElementsList::iterator itImages = lstImages.begin(); itImages != lstImages.end(); ++itImages)
    {
        CComQIPtr<MSHTML::IHTMLImgElement> pImg = *itImages;
        ATLASSERT(pImg != NULL);
        if (pImg == NULL)
            continue;

        CComBSTR bstrUseMap;
        hr = pImg->get_useMap(&bstrUseMap);
        ATLASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            continue;

        long lAbsoluteImgX(-1), lAbsoluteImgY(-1);
        hr = GetElementAbsolutePosition(CComQIPtr<MSHTML::IHTMLElement>(pImg), 
                                        lAbsoluteImgY, lAbsoluteImgX);
        ATLASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            continue;

		for (ElementsList::iterator itMaps = lstImageMaps.begin(); 
		     itMaps != lstImageMaps.end(); 
			 ++itMaps)
		{
            CComQIPtr<MSHTML::IHTMLMapElement> pMap = *itMaps;
            ATLASSERT(pMap != NULL);
            if (pMap == NULL)
                continue;

            CComBSTR bstrMapName;
            hr = pMap->get_name(&bstrMapName);
            ATLASSERT(SUCCEEDED(hr));
            if (FAILED(hr))
                continue;

            bstrMapName = CComBSTR("#") += bstrMapName;
            if (!(bstrMapName == bstrUseMap))
                continue;

            CComPtr<MSHTML::IHTMLAreasCollection> pAreas;
            hr = pMap->get_areas(&pAreas);
            ATLASSERT(SUCCEEDED(hr));
            if (FAILED(hr))
                break;

			ElementsList lstAreas;
			AppendElements(pAreas, lstAreas, CAllElements());
			pAreas = NULL;

			for (ElementsList::iterator itAreas = lstAreas.begin(); 
			     itAreas != lstAreas.end(); 
				 ++itAreas)
			{
                CComQIPtr<MSHTML::IHTMLAreaElement> pArea = *itAreas;
                ATLASSERT(pArea != NULL);
                if (pArea == NULL)
                    continue;

                CComBSTR bstrCoords;
                hr = pArea->get_coords(&bstrCoords);
                ATLASSERT(SUCCEEDED(hr));
                if (FAILED(hr))
                    continue;

                long lCoordX(-1), lCoordY(-1);
                const CComBSTR bstrSep(",");
                wchar_t* pcomma = wcstok(bstrCoords, bstrSep);
                if (pcomma != NULL)
                    lCoordX = _wtoi(pcomma);

                pcomma = wcstok(NULL, bstrSep);
                if (pcomma != NULL)
                    lCoordY = _wtoi(pcomma);

                string strPositionFormat = "z-index:2; position:absolute; left:%dpx; top:%dpx;";
                char szPosition[100] = {0};
                sprintf(szPosition, strPositionFormat.c_str(), lAbsoluteImgX + lCoordX, lAbsoluteImgY + lCoordY);

                AddNumberingToTag(CComQIPtr<MSHTML::IHTMLElement>(pArea), 
                                  CComQIPtr<MSHTML::IHTMLElement>(pImg), 
                                  CComBSTR("afterEnd"),
								  CElementBehaviourFactory::GetAreaBehaviour(),
                                  szPosition);
            }
        }
        
        pImageMaps = NULL;
    }

    return S_OK;
}

HRESULT CKeyboardAccess::AddInputFieldNumbering(CComQIPtr<MSHTML::IHTMLDocument3>& pDocument3)
{
    CComPtr<MSHTML::IHTMLElementCollection> pElements;
    HRESULT hr = pDocument3->getElementsByTagName(CComBSTR("input"), &pElements);
    ATLASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;
    
    ElementsList lstInputs;
    hr = AppendElements(pElements, lstInputs, CElementConditionFactory::GetInputCondition());
    if (FAILED(hr))
        return hr;

    pElements = NULL;
    ElementsList::iterator itInputsEnd = lstInputs.end();

    const CComBSTR bstrAfterEnd("afterEnd");
    for (ElementsList::iterator it = lstInputs.begin(); it != itInputsEnd; ++it)
    {
        const CComQIPtr<MSHTML::IHTMLElement> pCurrInput = *it;
        hr = AddNumberingToTag(pCurrInput, pCurrInput, bstrAfterEnd, CElementBehaviourFactory::GetInputBehaviour());
        ATLASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return hr;
    }

    return S_OK;
}


HRESULT CKeyboardAccess::AddDocumentLinkNumbering(CComQIPtr<MSHTML::IHTMLDocument3>& pDocument3)
{
    HRESULT hr = AddAnchorNumbering(pDocument3);
    ATLASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

    hr = AddInputFieldNumbering(pDocument3);
    ATLASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

    hr = AddImageMapNumbering(pDocument3);
    ATLASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

    return S_OK;
}

HRESULT CKeyboardAccess::GetElementAbsolutePosition(CComQIPtr<MSHTML::IHTMLElement>& pElement,
                                              long& lTop,
                                              long& lLeft)
{
    ATLASSERT(pElement != NULL);
    if (pElement == NULL)
        return E_POINTER;

    lTop = lLeft = 0;

    CComPtr<MSHTML::IHTMLElement> pParent = pElement;

    do 
    {
        long lParentOffsetLeft(-1), lParentOffsetTop(-1);
        HRESULT hr = pParent->get_offsetLeft(&lParentOffsetLeft);
        ATLASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return hr;

        hr = pParent->get_offsetTop(&lParentOffsetTop);
        ATLASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return hr;

        lLeft += lParentOffsetLeft;
        lTop += lParentOffsetTop;

        CComPtr<MSHTML::IHTMLElement> pPrevParent = pParent;
        pParent = NULL;
        hr = pPrevParent->get_offsetParent(&pParent);
        ATLASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return hr;
    } while (pParent != NULL);
 
    return S_OK;
}

// strCurrLinkNum = "<font style='font-weight:bolder; color:yellow; background-color:black;'>" + strCurrLinkNum + "</font>";
// strCurrLinkNum = "<span id=KeyboardAccess style='position:relative; z-index=1; left:-30px; color:yellow; background-color:black;'>" + strCurrLinkNum + "</span>";

//        strNextElementId = "<span id=KeyboardAccess style='color:yellow; background-color:black;'>" + strNextElementId + "</span>";

HRESULT CKeyboardAccess::AddNumberingToTag(const CComQIPtr<MSHTML::IHTMLElement>& pAddElement,
                                     const CComQIPtr<MSHTML::IHTMLElement>& pCodeElement,
                                     const CComBSTR& bstrWhere,
									 const CElementBehaviour& behaviour,
                                     const string& strAdditionalStyle)
{
    static char szTag[512] = {0};

    ATLASSERT(pAddElement != NULL && pCodeElement != NULL);
    if (pAddElement == NULL || pCodeElement == NULL)
        return E_POINTER;

    sprintf(szTag, m_strStyleFormat.c_str(), strAdditionalStyle.c_str(), m_Elements.size());

    HRESULT hr = pCodeElement->insertAdjacentHTML(bstrWhere, CComBSTR(szTag));
    ATLASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

    m_Elements.push_back(CElementHolder(pAddElement, m_spWebBrowser2, behaviour));

    return S_OK;
}

HRESULT CKeyboardAccess::OnUIOk(long lLinkNumber, BOOL bNewWindow)
{
    HRESULT hr = RemoveLinkNumbering();
    if (FAILED(hr))
        return hr;

    if (lLinkNumber >= m_Elements.size())
    {
        // m_Elements.clear();
        return S_OK;
    }

	return m_Elements[lLinkNumber].Click(bNewWindow);
}

HRESULT CKeyboardAccess::OnUICancel()
{
    m_Elements.clear();
    HRESULT hr = RemoveLinkNumbering();
    if (FAILED(hr))
        return hr;

    return S_OK;
}

HRESULT CKeyboardAccess::OnUIShowDocs()
{
	CComVariant varFlags = (long) navOpenInNewWindow;
	CComBSTR bstrDocs;
	m_Config.GetInstallFolder(bstrDocs);
	bstrDocs.Append(_T("\\KeyboardAccessDocs.html"));
	HRESULT hr = m_spWebBrowser2->Navigate(bstrDocs, &varFlags, NULL, NULL, NULL);
	ATLASSERT(SUCCEEDED(hr));
	if (FAILED(hr))
		return hr;

	return S_OK;
}


HRESULT CKeyboardAccess::RemoveLinkNumbering()
{
    HRESULT hr(S_OK);

    FramesList lstFrames;
    AppendFrames(lstFrames);

    ElementsList lstAllElements;
    for(FramesList::iterator frameIt = lstFrames.begin();
        frameIt != lstFrames.end();
        ++frameIt)
    {
        IWebBrowser2* pCurrBrowser = *frameIt;
        
        CComPtr<IDispatch> pDispatch;
        hr = pCurrBrowser->get_Document(&pDispatch);
        ATLASSERT(SUCCEEDED(hr));
        
        pCurrBrowser->Release();
        pCurrBrowser = NULL; 
        
        CComQIPtr<MSHTML::IHTMLDocument3> pDocument = pDispatch;
        ATLASSERT(pDocument != NULL);
        if (pDocument == NULL)
            return E_POINTER;
        
        CComPtr<MSHTML::IHTMLElementCollection> pMarkedLinks;
        hr = pDocument->getElementsByName(CComBSTR(m_pcElementsId), &pMarkedLinks);
        if (FAILED(hr))
            return hr;
        
        ElementsList lstMarkedElements;
        hr = AppendElements(pMarkedLinks, lstMarkedElements, CAllElements());
        if (FAILED(hr))
            return hr;
        
        for (ElementsList::iterator it = lstMarkedElements.begin(); 
        it != lstMarkedElements.end();
        ++it)
        {
            CComQIPtr<MSHTML::IHTMLElement> pCurrElement = *it;
            ATLASSERT(pCurrElement != NULL);
            pCurrElement->put_outerHTML(_bstr_t(""));
        }
        
    }

    return S_OK;
}

HRESULT CKeyboardAccess::AppendFrames(FramesList& lstBrowsers)
{
    // Make a copy of the main window because it will be released
    // and we don't want our member pointer destroyed
    CComQIPtr<IWebBrowser2> pMainWindow = m_spWebBrowser2; 

    HRESULT hr = AppendFramesRec(pMainWindow, lstBrowsers);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

HRESULT CKeyboardAccess::AppendFramesRec(CComQIPtr<IWebBrowser2>& pBrowser, FramesList& lstBrowsers)
{

    CComPtr<IDispatch> pDispatch;
    HRESULT hr = pBrowser->get_Document(&pDispatch);
    if (FAILED(hr))
        return hr;
    
    lstBrowsers.push_back(pBrowser.Detach());

    CComPtr<IOleContainer> pContainer;
    hr = pDispatch->QueryInterface(&pContainer);
	// Can fail if inside Windows Explorer
	if (FAILED(hr))
		return hr;

    pDispatch = NULL;

    CComPtr<IEnumUnknown> pEnumerator;
    hr = pContainer->EnumObjects(OLECONTF_EMBEDDINGS, &pEnumerator); // See MS Q196340
    ATLASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

    pContainer = NULL;

    CComPtr<IUnknown> pUnk;
    ULONG uFetched(0);
    for (UINT i = 0; S_OK == pEnumerator->Next(1, &pUnk, &uFetched); i++)
    {
        CComQIPtr<IWebBrowser2> pFrameBrowserFrame = pUnk;
        pUnk = NULL;
        if (pFrameBrowserFrame == NULL)
            continue;

        hr = AppendFramesRec(pFrameBrowserFrame, lstBrowsers);
        if (FAILED(hr))
            continue;
    }

    return S_OK;
}

HRESULT CKeyboardAccess::OnUIBrowserControl(WPARAM wVirtualKey)
{
    CComPtr<IDispatch> pDispatch;
    HRESULT hr = m_spWebBrowser2->get_Document(&pDispatch);
    ATLASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

    CComQIPtr<MSHTML::IHTMLDocument2> pDocument = pDispatch;
    ATLASSERT(pDocument != NULL);
    if (pDocument == NULL)
        return E_POINTER;
    
    pDispatch = NULL;

    CComPtr<MSHTML::IHTMLWindow2> pWindow;
    hr = pDocument->get_parentWindow(&pWindow);
    ATLASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

    pDocument = (MSHTML::IHTMLDocument2*) NULL;
    
    long lScrollAmount = 40;
    if (wVirtualKey == VK_UP)
        lScrollAmount *= -1;

    pWindow->scrollBy(0, lScrollAmount);

    return S_OK;
}

HRESULT CKeyboardAccess::OnUIChangeLinkNumber(long lLinkNumber)
{
    if (lLinkNumber >= m_Elements.size())
    {
		m_dlgChoose.EnableNewWindowButton(FALSE);
        return S_OK;
    }

	const CElementHolder& element = m_Elements[lLinkNumber];
	m_dlgChoose.EnableNewWindowButton(element.CanOpenNewWindow());
	
	return S_OK;
}


/*
HRESULT CKeyboardAccess::InstallKeyboardHook2()
{
    CComPtr<IDispatch> pDisp;
    HRESULT hr = m_spWebBrowser2->get_Document(&pDisp);
    ATLASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

    CComQIPtr<MSHTML::IHTMLDocument3> pDocument;
    pDocument = pDisp;
    pDisp = NULL;
    ATLASSERT(pDocument != NULL);
    if (pDocument == NULL)
        return E_POINTER;

    VARIANT_BOOL varResult = VARIANT_FALSE;
    hr = pDocument->attachEvent(CComBSTR(_T("onclick")), static_cast<IDispatch*>(this), 
                                &varResult);
    ATLASSERT(SUCCEEDED(hr));
    ATLASSERT(varResult == VARIANT_TRUE);
    if (FAILED(hr) || varResult == VARIANT_FALSE)
        return E_FAIL;

    return S_OK;
}

*/