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
#include "ElementHolder.h"
#include "ElementBehaviour.h"

HRESULT CElementHolder::Click(const BOOL bNewWindow)
{
	return m_Behaviour->Click(m_pElement, *m_pWebBrowser2, bNewWindow);
}

BOOL CElementHolder::CanOpenNewWindow() const
{
	return m_Behaviour->CanOpenNewWindow(m_pElement);
}

HRESULT CElementHolder::Focus() const
{
	return m_Behaviour->Focus(m_pElement);
}
