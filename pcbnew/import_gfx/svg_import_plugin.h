/*
 * This program source code file is part of KICAD, a free EDA CAD application.
 *
 * Copyright (C) 2016 CERN
 * @author Janito V. Ferreira Filho <janito.vff@gmail.com>
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

#ifndef SVG_IMPORT_PLUGIN_H
#define SVG_IMPORT_PLUGIN_H

#include "graphics_import_plugin.h"
#include "drw_interface.h"
#include <math/vector2d.h>


class SVG_IMPORT_PLUGIN : public GRAPHICS_IMPORT_PLUGIN
{
public:
    const wxString GetName() const override
    {
        return "Scalable Vector Graphics";
    }

    const wxArrayString GetFileExtensions() const override
    {
        return wxArrayString( 1, { "svg" } );
    }

    bool Load( const wxString& aFileName ) override;

private:
    void DrawPath( const float* aPoints, int aNumPoints, bool aClosedPath );

    void DrawLinePath( const float* aPoints, int aNumPoints,
            std::vector< VECTOR2D >& aGeneratedPoints );

    void DrawCubicBezierPath( const float* aPoints, int aNumPoints,
            std::vector< VECTOR2D >& aGeneratedPoints );

    void DrawCubicBezierCurve( const float* aPoints,
            std::vector< VECTOR2D >& aGeneratedPoints );

    void DrawPolygon( const std::vector< VECTOR2D >& aPoints );
    void DrawLineSegments( const std::vector< VECTOR2D >& aPoints );
};

#endif /* SVG_IMPORT_PLUGIN_H */
