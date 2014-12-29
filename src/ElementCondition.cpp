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
#include "ElementCondition.h"

struct CImagesWithImageMap : public CElementCondition
{
    bool Test(const LPDISPATCH pDispatch) const 
    { 
        CComQIPtr<MSHTML::IHTMLImgElement> pImg = pDispatch;
        if (pImg == NULL)
            return false;
        
        // The following piece of code had to be removed, because many web sites
        // are mistakenly using the ismap flag for client-side maps
/*
        // If server-side map, nothing to do
        VARIANT_BOOL bIsMap = VARIANT_TRUE;
        HRESULT hr = pImg->get_isMap(&bIsMap);
        ATLASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return false;

        if (bIsMap == VARIANT_TRUE)
            return false;
*/
        CComBSTR bstrUseMap;
        HRESULT hr = pImg->get_useMap(&bstrUseMap);
        ATLASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return false;

        if (bstrUseMap.Length() != 0)
            return true;

        return false;
    }
};

struct CVisibleInputFields : public CElementCondition
{
    bool Test(const LPDISPATCH pDispatch) const 
    { 
        CComQIPtr<MSHTML::IHTMLInputElement> pField = pDispatch;
        if (pField == NULL)
            return false;
        
        CComBSTR bstrType;
        HRESULT hr = pField->get_type(&bstrType);
        ATLASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return false;

        bstrType.ToLower();
        if (bstrType == CComBSTR("hidden"))
            return false;

        return true;
    }
};


CElementCondition& CElementConditionFactory::GetAnchorCondition()
{
	static CAllElements condition;
	return condition;
}

CElementCondition& CElementConditionFactory::GetAreaCondition()
{
	static CImagesWithImageMap condition;
	return condition;
}

CElementCondition& CElementConditionFactory::GetInputCondition()
{
	static CVisibleInputFields condition;
	return condition;
}
