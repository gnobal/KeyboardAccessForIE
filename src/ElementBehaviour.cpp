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

#include "stdafx.h"
#include "ExDisp.h"
#include "ElementBehaviour.h"

HRESULT CElementBehaviour::Click(const CComPtr<MSHTML::IHTMLElement>& pElement,
								 const CComQIPtr<IWebBrowser2>& pWebBrowser2,
								 const BOOL bNewWindow) const
{
	UNREFERENCED_PARAMETER(pWebBrowser2);
	UNREFERENCED_PARAMETER(bNewWindow);

	return pElement->click();
}

HRESULT CElementBehaviour::Focus(const CComPtr<MSHTML::IHTMLElement>& pElement) const
{
	CComQIPtr<MSHTML::IHTMLElement2> pElement2 = pElement;
	ATLASSERT(pElement2 != NULL);
	if (pElement2 == NULL)
		return E_POINTER;

	return pElement2->focus();
}

class CAnchorBehaviour : public CElementBehaviour {
public:
	virtual HRESULT Click(const CComPtr<MSHTML::IHTMLElement>& pElement,
						  const CComQIPtr<IWebBrowser2>& pWebBrowser2,
						  const BOOL bNewWindow) const
	{
		if (!bNewWindow)
		{
			return CElementBehaviour::Click(pElement, pWebBrowser2, bNewWindow);
		}

		CComQIPtr<MSHTML::IHTMLAnchorElement> pAnchor = pElement;
		ATLASSERT(pAnchor != NULL); // Must be an anchor, so there must be a bug
		if (pAnchor == NULL)
			return E_POINTER;

		CComBSTR bstrGoTo;
		HRESULT hr = pAnchor->get_href(&bstrGoTo);
		ATLASSERT(SUCCEEDED(hr));
		if (FAILED(hr))
			return hr;

		if (bstrGoTo == (BSTR) NULL) // No href value - can't navigate there - just click
			return CElementBehaviour::Click(pElement, pWebBrowser2, bNewWindow);

		CComBSTR bstrTargetFrame;
		hr = pAnchor->get_target(&bstrTargetFrame);
		ATLASSERT(SUCCEEDED(hr));
		if (FAILED(hr))
			return hr;
    
		CComVariant varFlags = 0L;
		if (bNewWindow)
		{
			varFlags = (long) (varFlags.lVal | navOpenInNewWindow);
		}
		hr = pWebBrowser2->Navigate(bstrGoTo, &varFlags, &(CComVariant(bstrTargetFrame)),
								    NULL, NULL);
		ATLASSERT(SUCCEEDED(hr));

		return S_OK;
	}

	virtual BOOL CanOpenNewWindow(const CComPtr<MSHTML::IHTMLElement>& pElement) const 
	{ 
		CComQIPtr<MSHTML::IHTMLAnchorElement> pAnchor = pElement;
		ATLASSERT(pAnchor != NULL); // Must be an anchor, so there must be a bug
		if (pAnchor == NULL)
			return FALSE;

		CComBSTR bstrHref;
		HRESULT hr = pAnchor->get_href(&bstrHref);
		ATLASSERT(SUCCEEDED(hr));
		if (FAILED(hr))
			return FALSE;

		if (bstrHref == (BSTR) NULL) // No href value - can't navigate there
			return FALSE;

		return TRUE;
	}
};

class CAreaBehaviour : public CElementBehaviour
{
	virtual BOOL CanOpenNewWindow(const CComPtr<MSHTML::IHTMLElement>& pElement) const 
	{ 
		CComQIPtr<MSHTML::IHTMLAreaElement> pArea = pElement;
		ATLASSERT(pArea != NULL); // Must be an area, so there must be a bug
		if (pArea == NULL)
			return FALSE;

		CComBSTR bstrHref;
		HRESULT hr = pArea->get_href(&bstrHref);
		ATLASSERT(SUCCEEDED(hr));
		if (FAILED(hr))
			return FALSE;
		
		if (bstrHref == (BSTR) NULL) // No href value - can't navigate there
			return FALSE;
		
		return TRUE;
	}
};

class CInputBehaviour : public CElementBehaviour
{
public:
	virtual HRESULT Click(const CComPtr<MSHTML::IHTMLElement>& pElement,
						  const CComQIPtr<IWebBrowser2>& pWebBrowser2,
						  const BOOL bNewWindow) const
	{
		CComQIPtr<MSHTML::IHTMLInputElement> pInput = pElement;
		ATLASSERT(pInput != NULL); // Must be an input element, so there must be a bug
		if (pInput == NULL)
			return E_POINTER;

		CComBSTR bstrType;
		HRESULT hr = pInput->get_type(&bstrType);
		ATLASSERT(SUCCEEDED(hr) && bstrType != (BSTR) NULL);
		if (FAILED(hr) || bstrType == (BSTR) NULL)
			return hr;

		hr = bstrType.ToLower();
		ATLASSERT(SUCCEEDED(hr));
		if (FAILED(hr))
			return hr;

		if (bstrType == CComBSTR("button")   ||
			bstrType == CComBSTR("checkbox") ||
			bstrType == CComBSTR("file")     ||
			bstrType == CComBSTR("image")    ||
			bstrType == CComBSTR("radio")    ||
			bstrType == CComBSTR("reset")    ||
			bstrType == CComBSTR("submit")   )
		{
			return CElementBehaviour::Click(pElement, pWebBrowser2, bNewWindow);
		}

		if (bstrType == CComBSTR("password") ||
			bstrType == CComBSTR("text")	 )
		{
			CComQIPtr<MSHTML::IHTMLElement2> pElement2 = pElement;
			ATLASSERT(pElement2 != NULL);
			if (pElement2 == NULL)
				return E_POINTER;

			return pElement2->focus();
		}

		ATLASSERT(FALSE); // This code should never be reached, because "hidden" type
						  // is never inserted into the elements collection, and there
						  // are no more types
		return E_FAIL;
	}

	virtual BOOL CanOpenNewWindow(const CComPtr<MSHTML::IHTMLElement>& pElement) const 
		{ return FALSE; }
};

CElementBehaviour& CElementBehaviourFactory::GetAnchorBehaviour()
{
	static CAnchorBehaviour behaviour;
	return behaviour;
}

CElementBehaviour& CElementBehaviourFactory::GetAreaBehaviour()
{
	static CAreaBehaviour behaviour;
	return behaviour;
}

CElementBehaviour& CElementBehaviourFactory::GetInputBehaviour()
{
	static CInputBehaviour behaviour;
	return behaviour;
}
