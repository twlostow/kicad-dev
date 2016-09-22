/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 CERN
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef __VIEW_CACHE_H
#define __VIEW_CACHE_H

#include <vector>
#include <boost/unordered/unordered_map.hpp>
#include <boost/range/adaptor/map.hpp>

#include <math/box2.h>
#include <gal/definitions.h>

namespace KIGFX {
class GAL_GROUP;
class VIEW_CACHE;
class VIEW_ITEM_NG;
class VIEW_BASE;
class VIEW_RTREE_NG;

#define VI_VISIBLE      0x1
#define VI_HIDDEN       0x2
#define VI_SELECTABLE   0x04
#define VI_MASKED       0x08
#define VI_CACHEABLE    0x10
#define VI_DIRTY        0x20

class VIEW_CACHE_ENTRY
{
public:
    VIEW_CACHE_ENTRY() :
        m_cachedGroups( NULL ),
        m_numCachedGroups( 0 ),
        m_owner( NULL ),
        m_entry (NULL),
        m_flags( VI_VISIBLE | VI_DIRTY ) {
    }

    ~VIEW_CACHE_ENTRY() {
        if( m_cachedGroups )
            delete m_cachedGroups;
    }

    void SetEntry (VIEW_RTREE_ENTRY*ent){m_entry = ent; }

    inline void SetGroup( int idx, int group )
    {
        allocateGroups( idx + 1 );
        m_cachedGroups[idx] = group;
    }


    inline int GetGroup( int idx )
    {
        if( idx >= m_numCachedGroups )
            return -1;

        return m_cachedGroups[idx];
    }

    inline int GetGroupCount( ) const
    {
            return m_numCachedGroups;
    }


    void SetOwner( VIEW_RTREE_NG* aOwner )
    {
        m_owner = aOwner;
        // m_item = aItem;
    }


    void SetBBox( const BOX2I& aBBox )
    {
        m_bbox = aBBox;
    }


    const BOX2I& GetBBox() const
    {
        return m_bbox;
    }


    void SetFlags( int aFlags )
    {
        m_flags |= aFlags;
    }


    void SetFlag( int aFlag, bool aValue )
    {
        if( aValue )
            m_flags |= aFlag;
        else
            m_flags &= ~aFlag;
    }


    void ClearFlags( int aFlags )
    {
        m_flags &= ~aFlags;
    }


    bool TestFlags( int aFlag ) const
    {
        return (m_flags & aFlag) == aFlag;
    }


    bool IsVisible() const
    {
        return (m_flags & VI_VISIBLE) && !(m_flags & VI_HIDDEN );
    }


    bool IsDirty() const
    {
        return m_flags & VI_DIRTY;
    }


    VIEW_RTREE_NG *GetOwner() const
    {
        return m_owner;
    }

    VIEW_RTREE_ENTRY*  GetEntry() const { return m_entry; }

private:

    inline void allocateGroups( int size )
    {
        if( size < 9 )
            size = 9;

        if( m_cachedGroups )
        {
            if( size > m_numCachedGroups )
            {
                int* old = m_cachedGroups;
                m_cachedGroups = new int[size];
                memcpy( m_cachedGroups, old, sizeof(int) * m_numCachedGroups );

                for( int i = m_numCachedGroups; i < size; i++ )
                    m_cachedGroups[i] = -1;

                m_numCachedGroups = size;
                delete [] old;
            }
        }
        else
        {
            m_cachedGroups = new int[size];

            for( int i = 0; i < size; i++ )
                m_cachedGroups[i] = -1;

            m_numCachedGroups = size;
        }
    }


    BOX2I m_bbox;
    int* m_cachedGroups;
    int m_numCachedGroups;
    VIEW_RTREE_NG* m_owner;
    VIEW_RTREE_ENTRY *m_entry;
    int m_flags;
};

class VIEW_CACHE
{
public:
    typedef boost::unordered_map<VIEW_ITEM_NG*, VIEW_CACHE_ENTRY*> CACHE_ENTRIES;

    VIEW_CACHE( VIEW_BASE * aParent ) :
        m_view( aParent ) {
    }

    ~VIEW_CACHE() {
        Clear();
    }

    VIEW_CACHE_ENTRY* Add( VIEW_ITEM_NG* aItem, VIEW_RTREE_ENTRY *rtreeEnt )
    {
        VIEW_CACHE_ENTRY* ent;

        if( m_cache.find( aItem ) == m_cache.end() )
        {
            ent = new VIEW_CACHE_ENTRY();

            m_cache[aItem] = ent;
            ent->SetFlags( VI_CACHEABLE );

        }
        else
        {
            ent = m_cache[aItem];
        }

        ent->SetBBox( aItem->ngViewBBox() );
        ent->SetFlags( VI_DIRTY );
        ent->SetEntry ( rtreeEnt );

        return ent;
    }

    void Remove ( VIEW_ITEM_NG *aItem )
    {

        VIEW_CACHE_ENTRY *ent = m_cache[ aItem ];
        delete ent;

        m_cache.erase (aItem );
    }

    VIEW_CACHE_ENTRY* GetEntry( VIEW_ITEM_NG* aItem )
    {
        CACHE_ENTRIES::iterator i = m_cache.find(aItem);

        if(i == m_cache.end())
            return NULL;

        return i->second;
    }


/*  void SetVisible( VIEW_ITEM_NG *aItem, bool aVisible )
 *  {
 *   m_cache[aItem]->visible = aVisible;
 *  }
 *
 *  void SetDirty( VIEW_ITEM_NG *aItem, bool aDirty )
 *  {
 *   m_cache[aItem]->dirty = aDirty;
 *  }*/

    int Size() const
    {
        return m_cache.size();
    }


    void SetDirty( bool dirty )
    {
        for( boost::unordered_map<VIEW_ITEM_NG*, VIEW_CACHE_ENTRY*>::iterator i = m_cache.begin();
             i != m_cache.end();
             ++i )
        {
            i->second->SetFlag( VI_DIRTY, dirty );
        }
    }


    CACHE_ENTRIES& GetCache()
    {
        return m_cache;
    }

    void Clear()
    {
        BOOST_FOREACH ( VIEW_CACHE_ENTRY *ent, m_cache | boost::adaptors::map_values )
        {
            delete ent;
        }

        m_cache.clear();
    }


private:



    CACHE_ENTRIES m_cache;
    VIEW_BASE* m_view;
};
}
#endif
