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

#ifndef _ELEMENT_HOLDER_H__
#define _ELEMENT_HOLDER_H__

class CElementBehaviour;

class CElementHolder {
public:
	CElementHolder(const CComQIPtr<MSHTML::IHTMLElement>& pElement,
				   const CComQIPtr<IWebBrowser2>& pWebBrowser2,
				   const CElementBehaviour& cBehaviour)
	: m_pElement(pElement), m_pWebBrowser2(&pWebBrowser2), m_Behaviour(&cBehaviour)
	{
	}

	HRESULT Click(const BOOL bNewWindow);
	HRESULT Focus() const;
	BOOL CanOpenNewWindow() const;

private:
	CComPtr<MSHTML::IHTMLElement> m_pElement;
	const CComQIPtr<IWebBrowser2>* m_pWebBrowser2;
	const CElementBehaviour* m_Behaviour;
};

#endif

