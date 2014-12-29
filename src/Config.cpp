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
#include "Config.h"

const TCHAR CConfig::m_szStyleSubKey[] = _T("Software\\Gnobal\\KeyboardAccess\\Style");
const TCHAR CConfig::m_szInstallFolderSubKey[] = _T("Software\\Gnobal\\KeyboardAccess\\InstallFolder");
TCHAR CConfig::m_szColorName[] = _T("color");
TCHAR CConfig::m_szBackgroundColorName[] = _T("background-color");
TCHAR CConfig::m_szFontSizeName[] = _T("font-size");
TCHAR CConfig::m_szFontWeightName[] = _T("font-weight");


HRESULT CConfig::Create()
{
	HRESULT hr = QueryStyleString();
	ATLASSERT(SUCCEEDED(hr));
	if (FAILED(hr))
		return hr;

	hr = QueryInstallFolder();
	ATLASSERT(SUCCEEDED(hr));
	if (FAILED(hr))
		return hr;

    return S_OK;
}

HRESULT CConfig::QueryInstallFolder()
{
    CRegKey keyConfig;

    LONG lResult = keyConfig.Open(HKEY_LOCAL_MACHINE, m_szInstallFolderSubKey, KEY_READ);
    ATLASSERT(lResult == ERROR_SUCCESS);
    if (lResult != ERROR_SUCCESS)
        return E_FAIL;

    TCHAR szValue[MAX_PATH] = {0};
    DWORD dwSize = sizeof(szValue)/sizeof(szValue[0]);

	lResult = keyConfig.QueryValue(szValue, NULL, &dwSize);
    ATLASSERT(lResult == ERROR_SUCCESS);
    if (lResult != ERROR_SUCCESS)
        return E_FAIL;

	keyConfig.Close();

	m_bstrInstallFolder.Append(szValue);

	return S_OK;
}

HRESULT CConfig::QueryStyleString()
{
    CRegKey keyConfig;
    LONG lResult = keyConfig.Open(HKEY_LOCAL_MACHINE, m_szStyleSubKey, KEY_READ);
    ATLASSERT(lResult == ERROR_SUCCESS);
    if (lResult != ERROR_SUCCESS)
        return E_FAIL;

    HRESULT hr = AppendStyleNameValueString(keyConfig, m_szColorName);
    if (FAILED(hr))
        return hr;

    hr = AppendStyleNameValueString(keyConfig, m_szBackgroundColorName);
    if (FAILED(hr))
        return hr;

    hr = AppendStyleNameValueString(keyConfig, m_szFontSizeName);
    if (FAILED(hr))
        return hr;
    
    hr = AppendStyleNameValueString(keyConfig, m_szFontWeightName);
    if (FAILED(hr))
        return hr;
        
    keyConfig.Close();

	return S_OK;
}


HRESULT CConfig::GetStyleString(CComBSTR& bstrStyle)
{
    bstrStyle = m_bstrStyle;
    return S_OK;
}

HRESULT CConfig::GetInstallFolder(CComBSTR& bstrInstallFolder)
{
    bstrInstallFolder = m_bstrInstallFolder;
    return S_OK;
}

HRESULT CConfig::AppendStyleNameValueString(CRegKey& keyConfig, LPTSTR lpszName)
{
    TCHAR szValue[MAX_PATH] = {0};
    DWORD dwSize = sizeof(szValue)/sizeof(szValue[0]);
    
    LONG lResult = keyConfig.QueryValue(szValue, lpszName, &dwSize);
    ATLASSERT(lResult == ERROR_SUCCESS);
    if (lResult != ERROR_SUCCESS)
        return E_FAIL;

    // No need to include empty values
    if (CComBSTR(szValue) == CComBSTR(""))
        return S_OK;

    m_bstrStyle.Append(lpszName);
    m_bstrStyle.Append(":");
    m_bstrStyle.Append(szValue);
    m_bstrStyle.Append(";");

    return S_OK;
}
