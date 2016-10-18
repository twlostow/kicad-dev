/*
 * This program source code file is part of KICAD, a free EDA CAD application.
 *
 * Copyright (C) 2016 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
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

#ifndef GRAPHICS_IMPORTER_PCBNEW_H
#define GRAPHICS_IMPORTER_PCBNEW_H

#include "graphics_importer.h"

#include <layers_id_colors_and_visibility.h>

class BOARD_ITEM;
class DRAWSEGMENT;
class EDA_TEXT;

class GRAPHICS_IMPORTER_PCBNEW : public GRAPHICS_IMPORTER
{
public:
    GRAPHICS_IMPORTER_PCBNEW()
        : m_layer( Dwgs_User )
    {
    }

    void SetLayer( LAYER_ID aLayer )
    {
        m_layer = aLayer;
    }

    LAYER_ID GetLayer() const
    {
        return m_layer;
    }

    void AddLine( const wxPoint& aOrigin, const wxPoint& aEnd ) override;
    void AddCircle( const wxPoint& aOrigin, unsigned int aRadius ) override;
    void AddArc( const wxPoint& aCenter, const wxPoint& aStart, double aAngle ) override;
    void AddText( const wxPoint& aOrigin, const wxString& aText,
            unsigned int aHeight, unsigned aWidth, double aOrientation,
            EDA_TEXT_HJUSTIFY_T aHJustify, EDA_TEXT_VJUSTIFY_T aVJustify ) override;

protected:
    virtual DRAWSEGMENT* createDrawing() const = 0;
    virtual std::pair<BOARD_ITEM*, EDA_TEXT*> createText() const = 0;

    LAYER_ID m_layer;
};


class GRAPHICS_IMPORTER_BOARD : public GRAPHICS_IMPORTER_PCBNEW
{
protected:
    DRAWSEGMENT* createDrawing() const override;
    std::pair<BOARD_ITEM*, EDA_TEXT*> createText() const override;
};


class GRAPHICS_IMPORTER_MODULE : public GRAPHICS_IMPORTER_PCBNEW
{
protected:
    DRAWSEGMENT* createDrawing() const override;
    std::pair<BOARD_ITEM*, EDA_TEXT*> createText() const override;
};

#endif /* GRAPHICS_IMPORTER_PCBNEW */
