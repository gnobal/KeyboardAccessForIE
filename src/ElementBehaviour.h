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

#ifndef _ELEMENT_BEHAVIOUR_H__
#define _ELEMENT_BEHAVIOUR_H__

class CElementBehaviour {
public:
	virtual ~CElementBehaviour() {}
	
	virtual HRESULT Click(const CComPtr<MSHTML::IHTMLElement>& pElement, 
						  const CComQIPtr<IWebBrowser2>& pWebBrowser2,
						  const BOOL bNewWindow) const;
	virtual HRESULT Focus(const CComPtr<MSHTML::IHTMLElement>& pElement) const;

	virtual BOOL CanOpenNewWindow(const CComPtr<MSHTML::IHTMLElement>& pElement) const =0;

protected:
	// HRESULT DoSimpleClick(const CComPtr<MSHTML::IHTMLElement>& pElement);
};

class CElementBehaviourFactory {
public:
	static CElementBehaviour& GetAnchorBehaviour();
	static CElementBehaviour& GetAreaBehaviour();
	static CElementBehaviour& GetInputBehaviour();
};

#endif
