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

#ifndef _THREADSAFEMAP_H_
#define _THREADSAFEMAP_H_

#include <map>
#include <utility>

using std::pair;

template<typename Key, typename HeldType>
class CThreadSafeMap 
{
public:
    typedef std::map<Key, HeldType> Impl;
    typedef Impl::iterator iterator;
    typedef Impl::value_type value_type;
    typedef Impl::size_type size_type;

    CThreadSafeMap()
    {
        InitializeCriticalSection(&m_csLock);
    }

    ~CThreadSafeMap()
    {
        DeleteCriticalSection(&m_csLock);
    }

    iterator find(const Key& key)
    {
        EnterCriticalSection(&m_csLock);
        iterator itRet = m_impl.find(key);
        LeaveCriticalSection(&m_csLock);
        
        return itRet;
    }

    pair<iterator, bool> insert(const value_type& x)
    {
        EnterCriticalSection(&m_csLock);
        pair<iterator, bool> pairRet = m_impl.insert(x);
        LeaveCriticalSection(&m_csLock);
        
        return pairRet;
    }


    size_type erase(const Key& key)
    {
        EnterCriticalSection(&m_csLock);
        size_type szRet = m_impl.erase(key);
        LeaveCriticalSection(&m_csLock);
        
        return szRet;
    }

private:
    // disallow copies
    CThreadSafeMap(const CThreadSafeMap&);
    CThreadSafeMap& operator=(const CThreadSafeMap&);

private:
    CRITICAL_SECTION m_csLock;
    Impl m_impl;
};

#endif
