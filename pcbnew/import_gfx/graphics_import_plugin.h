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

#ifndef GRAPHICS_IMPORT_PLUGIN_H
#define GRAPHICS_IMPORT_PLUGIN_H

#include <wx/arrstr.h>

class GRAPHICS_IMPORTER;

class GRAPHICS_IMPORT_PLUGIN
{
public:
    virtual ~GRAPHICS_IMPORT_PLUGIN()
    {
    }

    void SetImporter( GRAPHICS_IMPORTER* aImporter )
    {
        m_importer = aImporter;
    }

    virtual const wxString GetName() const = 0;

    virtual const wxArrayString GetFileExtensions() const = 0;

    virtual bool Load( const wxString& aFileName ) = 0;

protected:
    GRAPHICS_IMPORTER* m_importer;
};

#endif /* GRAPHICS_IMPORT_PLUGIN_H */

