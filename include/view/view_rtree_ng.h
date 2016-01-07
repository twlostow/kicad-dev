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

#ifndef __VIEW_RTREE_NG_H
#define __VIEW_RTREE_NG_H

#include <math/box2.h>

#include <geometry/rtree.h>


namespace KIGFX
{

  class VIEW_ITEM_NG;
  class VIEW_CACHE_ENTRY;

struct VIEW_RTREE_ENTRY {

  bool operator== ( const VIEW_RTREE_ENTRY& aOther ) const
  {
      return item == aOther.item;
  }

  bool operator!= ( const VIEW_RTREE_ENTRY& aOther ) const
  {
      return item != aOther.item;
  }

  VIEW_ITEM_NG *item;
  VIEW_CACHE_ENTRY *ent;
};

typedef RTree<VIEW_RTREE_ENTRY*, int, 2, float> VIEW_RTREE_BASE_NG;

/**
 * Class VIEW_RTREE -
 * Implements an R-tree for fast spatial indexing of VIEW items.
 * Non-owning.
 */
class VIEW_RTREE_NG : public VIEW_RTREE_BASE_NG
{
public:

    /**
     * Function Insert()
     * Inserts an item into the tree. Item's bounding box is taken via its ViewBBox() method.
     */
    void Insert(  VIEW_RTREE_ENTRY* aItem, const BOX2I& bbox )
    {
        const int       mmin[2] = { bbox.GetX(), bbox.GetY() };
        const int       mmax[2] = { bbox.GetRight(), bbox.GetBottom() };

        VIEW_RTREE_BASE_NG::Insert( mmin, mmax, aItem );
    }

    /**
     * Function Remove()
     * Removes an item from the tree. Removal is done by comparing pointers, attepmting to remove a copy
     * of the item will fail.
     */
    void Remove( VIEW_RTREE_ENTRY* aItem, const BOX2I& bbox )
    {
        const int       mmin[2] = { bbox.GetX(), bbox.GetY() };
        const int       mmax[2] = { bbox.GetRight(), bbox.GetBottom() };

        VIEW_RTREE_BASE_NG::Remove( mmin, mmax, aItem );
    }

    /**
     * Function Query()
     * Executes a function object aVisitor for each item whose bounding box intersects
     * with aBounds.
     */
    template <class Visitor>
    void Query( const BOX2I& aBounds, Visitor& aVisitor )    // const
    {
        const int   mmin[2] = { aBounds.GetX(), aBounds.GetY() };
        const int   mmax[2] = { aBounds.GetRight(), aBounds.GetBottom() };

        VIEW_RTREE_BASE_NG::Search( mmin, mmax, aVisitor );
    }

private:
};
} // namespace KIGFX

#endif
