/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2014 CERN
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

#ifndef __SCH_PAINTER_H
#define __SCH_PAINTER_H

#include <painter.h>

class LIB_RECTANGLE;
class LIB_PIN;
class LIB_CIRCLE;
class LIB_ITEM;
class LIB_COMPONENT;
class LIB_POLYLINE;
class LIB_ARC;
class LIB_FIELD;
class LIB_TEXT;
class SCH_COMPONENT;
class SCH_FIELD;

namespace KIGFX
{
class GAL;
class SCH_PAINTER;


/**
 * Class SCH_RENDER_SETTINGS
 * Stores schematic-specific render settings.
 */

class SCH_RENDER_SETTINGS : public RENDER_SETTINGS
{
public:
    friend class SCH_PAINTER;

    SCH_RENDER_SETTINGS();

    /// @copydoc RENDER_SETTINGS::ImportLegacyColors()
    void ImportLegacyColors( COLORS_DESIGN_SETTINGS* aSettings );

    /// @copydoc RENDER_SETTINGS::GetColor()
    virtual const COLOR4D& GetColor( const VIEW_ITEM* aItem, int aLayer ) const;
};


/**
 * Class SCH_PAINTER
 * Contains methods for drawing schematic-specific items.
 */
class SCH_PAINTER : public PAINTER
{
public:
    SCH_PAINTER( GAL* aGal );

    /// @copydoc PAINTER::Draw()
    virtual bool Draw( const VIEW_ITEM*, int );

    /// @copydoc PAINTER::ApplySettings()
    virtual void ApplySettings( RENDER_SETTINGS* aSettings )
    {
        PAINTER::ApplySettings( aSettings );
    }

private:
	void draw( const LIB_RECTANGLE *, int );
	void draw( const LIB_PIN *, int );
	void draw( const LIB_CIRCLE *, int );
	void draw( const LIB_ITEM *, int );
	void draw( const LIB_COMPONENT *, int );
    void draw( const LIB_ARC *, int );
    void draw( const LIB_POLYLINE *, int );
    void draw( const LIB_FIELD *, int );
    void draw( const LIB_TEXT *, int );
    void draw( const SCH_COMPONENT *, int );
    void draw( const SCH_FIELD *, int );


    void defaultColors( const LIB_ITEM *aItem );
		
    void triLine ( const VECTOR2D &a, const VECTOR2D &b, const VECTOR2D &c );
};

}; // namespace KIGFX


#endif
