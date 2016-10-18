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

#ifndef GRAPHICS_IMPORTER_H
#define GRAPHICS_IMPORTER_H

#include "graphics_import_mgr.h"
#include "graphics_import_plugin.h"

#include <eda_text.h>

#include <list>
#include <memory>

class EDA_ITEM;

class GRAPHICS_IMPORTER
{
public:
    GRAPHICS_IMPORTER();

    virtual ~GRAPHICS_IMPORTER()
    {
    }

    void SetPlugin( GRAPHICS_IMPORT_MGR::GFX_FILE_T aType );

    bool Import( const wxString& aFileName );

    void SetLineWidth( unsigned int aWidth )
    {
        m_lineWidth = aWidth;
    }

    unsigned int GetLineWidth() const
    {
        return m_lineWidth;
    }

    void SetScale( double aScale )
    {
        m_scale = aScale;
    }

    double GetScale() const
    {
        return m_scale;
    }

    const std::list<EDA_ITEM*>& GetItems() const
    {
        return m_items;
    }

    ///> Default line thickness (in internal units)
    static constexpr unsigned int DEFAULT_LINE_WIDTH = 1;

    // Methods to be implemented by derived graphics importers
    virtual void AddLine( const wxPoint& aOrigin, const wxPoint& aEnd ) = 0;
    virtual void AddCircle( const wxPoint& aCenter, unsigned int aRadius ) = 0;
    virtual void AddArc( const wxPoint& aCenter, const wxPoint& aStart, double aAngle ) = 0;
    //virtual void AddArc( const wxPoint& aOrigin, double aStartAngle, double aEndAngle ) = 0;
    virtual void AddText( const wxPoint& aOrigin, const wxString& aText,
            unsigned int aHeight, unsigned aWidth, double aOrientation,
            EDA_TEXT_HJUSTIFY_T aHJustify, EDA_TEXT_VJUSTIFY_T aVJustify ) = 0;

protected:
    void addItem( EDA_ITEM* aItem )
    {
        m_items.emplace_back( aItem );
    }

private:
    ///> List of imported items
    std::list<EDA_ITEM*> m_items;

    ///> Plugin used to load a file
    std::unique_ptr<GRAPHICS_IMPORT_PLUGIN> m_plugin;

    ///> Default line thickness for the imported graphics
    unsigned int m_lineWidth;

    ///> Scale factor applied to the imported graphics
    double m_scale;
};

#endif /* GRAPHICS_IMPORTER_H */
