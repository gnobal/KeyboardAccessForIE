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

#ifndef _CONFIG_H__
#define _CONFIG_H__

class CConfig {
public:
    HRESULT Create();
    HRESULT GetStyleString(CComBSTR& bstrStyle);
    HRESULT GetInstallFolder(CComBSTR& bstrInstallFolder);

private:
    HRESULT AppendStyleNameValueString(CRegKey& keyConfig, LPTSTR lpszName);
	HRESULT QueryStyleString();
	HRESULT QueryInstallFolder();

private:
    CComBSTR m_bstrStyle;
    CComBSTR m_bstrInstallFolder;

private:
    static const TCHAR m_szStyleSubKey[];
    static const TCHAR m_szInstallFolderSubKey[];
    static TCHAR m_szColorName[];
    static TCHAR m_szBackgroundColorName[];
    static TCHAR m_szFontSizeName[];
    static TCHAR m_szFontWeightName[];
};


#endif
