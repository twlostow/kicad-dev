/*
 * This program source code file is part of KICAD, a free EDA CAD application.
 *
 * Copyright (C) 2016 CERN
 * @author Janito V. Ferreira Filho
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

#include "svg_import_plugin.h"

#include <wx/gdicmn.h>

#include "nanosvg.h"

#include "graphics_importer.h"

static wxRealPoint getBezierPoint( const float* aCurvePoints, float aStep );
static wxRealPoint getPointInLine( const wxRealPoint& aLineStart, const wxRealPoint& aLineEnd,
        float aDistance );

bool SVG_IMPORT_PLUGIN::Load( const wxString& aFileName )
{
    wxCHECK( m_importer, false );

    struct NSVGimage* image = nsvgParseFromFile( aFileName.c_str(), "px", 96 );

    wxCHECK( image, false );

    for( NSVGshape* shape = image->shapes; shape != NULL; shape = shape->next )
    {
        for( NSVGpath* path = shape->paths; path != NULL; path = path->next )
            DrawCubicBezierPath( path->pts, path->npts );
    }

    nsvgDelete( image );

    return true;
}


void SVG_IMPORT_PLUGIN::DrawCubicBezierPath( const float* aPoints, int aNumPoints )
{
    const int numberOfCoordinatesPerSegment = 2 * 4;

    const float* currentPoints = aPoints;

    for( int numPoints = aNumPoints; numPoints > 0; numPoints -= 4 )
    {
        DrawCubicBezierCurve( currentPoints );
        currentPoints += numberOfCoordinatesPerSegment;
    }
}


void SVG_IMPORT_PLUGIN::DrawCubicBezierCurve( const float* aPoints )
{
    float startX = aPoints[0];
    float startY = aPoints[1];

    auto currentPoint = wxRealPoint( startX, startY );

    for( float step = 0.f; step < 1.f; step += 0.1f )
    {
        auto nextPoint = getBezierPoint( aPoints, step );

        m_importer->AddLine( currentPoint, nextPoint );

        currentPoint = nextPoint;
    }
}

static wxRealPoint getBezierPoint( const float* aPoints, float aStep )
{
    auto firstCubicPoint = wxRealPoint( aPoints[0], aPoints[1] );
    auto secondCubicPoint = wxRealPoint( aPoints[2], aPoints[3] );
    auto thirdCubicPoint = wxRealPoint( aPoints[4], aPoints[5] );
    auto fourthCubicPoint = wxRealPoint( aPoints[6], aPoints[7] );

    auto firstQuadraticPoint = getPointInLine( firstCubicPoint, secondCubicPoint, aStep );
    auto secondQuadraticPoint = getPointInLine( secondCubicPoint, thirdCubicPoint, aStep );
    auto thirdQuadraticPoint = getPointInLine( thirdCubicPoint, fourthCubicPoint, aStep );

    auto firstLinearPoint = getPointInLine( firstQuadraticPoint, secondQuadraticPoint, aStep );
    auto secondLinearPoint = getPointInLine( secondQuadraticPoint, thirdQuadraticPoint, aStep );

    return getPointInLine( firstLinearPoint, secondLinearPoint, aStep );
}

static wxRealPoint getPointInLine( const wxRealPoint& aLineStart, const wxRealPoint& aLineEnd,
        float aDistance )
{
    return aLineStart + ( aLineEnd - aLineStart ) * aDistance;
}
