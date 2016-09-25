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

/**
 * @file view_item.h
 * @brief VIEW_ITEM class definition.
 */

#ifndef __VIEW_ITEM_H
#define __VIEW_ITEM_H

#include <vector>
#include <bitset>
#include <math/box2.h>
#include <gal/definitions.h>


namespace KIGFX
{

class VIEW_ITEM_DATA;
// Forward declarations
class VIEW_BASE;
class PAINTER;

/**
 * Class VIEW_ITEM -
 * is an abstract base class for deriving all objects that can be added to a VIEW.
 * It's role is to:
 * - communicte geometry, appearance and visibility updates to the associated dynamic VIEW,
 * - provide a bounding box for redraw area calculation,
 * - (optional) draw the object using the GAL API functions for PAINTER-less implementations.
 * VIEW_ITEM objects are never owned by a VIEW. A single VIEW_ITEM can belong to any number of
 * static VIEWs, but only one dynamic VIEW due to storage of only one VIEW reference.
 */
class VIEW_ITEM
{
public:

    VIEW_ITEM() {}

    /**
     * Destructor. For dynamic views, removes the item from the view.
     */
    virtual ~VIEW_ITEM()
    {

    }

    /**
     * Function ViewBBox()
     * returns the bounding box of the item covering all its layers.
     * @return BOX2I - the current bounding box
     */
    virtual const BOX2I ViewBBox() const;/*
    {
      return BOX2I();
    }*/

    /**
     * Function ViewDraw()
     * Draws the parts of the object belonging to layer aLayer.
     * viewDraw() is an alternative way for drawing objects if
     * if there is no PAINTER assigned for the view or if the PAINTER
     * doesn't know how to paint this particular implementation of
     * VIEW_ITEM. The preferred way of drawing is to design an
     * appropriate PAINTER object, the method below is intended only
     * for quick hacks and debugging purposes.
     *
     * @param aLayer: current drawing layer
     * @param aGal: pointer to the GAL device we are drawing on
     */
    virtual void ViewDraw( int aLayer, VIEW_BASE* aView ) const
    {}

    virtual void ViewGetLayers( int aLayers[], int& aCount ) const;

protected:

private:
    VIEW_ITEM_DATA *m_viewData;


};
} // namespace KIGFX

#endif
