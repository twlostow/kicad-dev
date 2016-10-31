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
static wxRealPoint getPoint( const float* aPointCoordinates );
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
            DrawPath( path->pts, path->npts, path->closed );
    }

    nsvgDelete( image );

    return true;
}


void SVG_IMPORT_PLUGIN::DrawPath( const float* aPoints, int aNumPoints, bool aClosedPath )
{
    const int numBezierPoints = aNumPoints & ~0x3;
    const int numLinePoints = aNumPoints & 0x3;

    const int numBezierCoordinates = numBezierPoints * 2;
    const float* linePoints = aPoints + numBezierCoordinates;

    std::vector< wxRealPoint > collectedPathPoints;

    if( numBezierPoints > 0 )
        DrawCubicBezierPath( aPoints, numBezierPoints, collectedPathPoints );

    if( numLinePoints > 0 )
        DrawLinePath( linePoints, numLinePoints, collectedPathPoints );

    if( aClosedPath )
        DrawPolygon( collectedPathPoints );
    else
        DrawLineSegments( collectedPathPoints );
}


void SVG_IMPORT_PLUGIN::DrawCubicBezierPath( const float* aPoints, int aNumPoints,
        std::vector< wxRealPoint >& aGeneratedPoints )
{
    const int numberOfCoordinatesPerSegment = 2 * 4;
    const float* currentPoints = aPoints;

    for( int numPoints = aNumPoints; numPoints > 0; numPoints -= 4 )
    {
        DrawCubicBezierCurve( currentPoints, aGeneratedPoints );
        currentPoints += numberOfCoordinatesPerSegment;
    }
}


void SVG_IMPORT_PLUGIN::DrawCubicBezierCurve( const float* aPoints,
        std::vector< wxRealPoint >& aGeneratedPoints )
{
    for( float step = 0.f; step < 1.f; step += 0.1f )
    {
        auto point = getBezierPoint( aPoints, step );

        aGeneratedPoints.push_back( point );
    }
}


void SVG_IMPORT_PLUGIN::DrawLinePath( const float* aPoints, int aNumPoints,
        std::vector< wxRealPoint >& aGeneratedPoints )
{
    const int coordinatesPerPoint = 2;
    const float* remainingPoints = aPoints;

    for( int numPoints = aNumPoints; numPoints > 0; --numPoints )
    {
        auto point = getPoint( remainingPoints );

        aGeneratedPoints.push_back( point );

        remainingPoints+= coordinatesPerPoint;
    }
}


void SVG_IMPORT_PLUGIN::DrawPolygon( const std::vector< wxRealPoint >& aPrecisePoints )
{
    std::vector< wxPoint > convertedPoints( aPrecisePoints.size() );

    for( const wxRealPoint& point : aPrecisePoints )
        convertedPoints.emplace_back( point.x, point.y );

    m_importer->AddPolygon( convertedPoints );
}


void SVG_IMPORT_PLUGIN::DrawLineSegments( const std::vector< wxRealPoint >& aPoints )
{
    unsigned int numLineStartPoints = aPoints.size() - 1;

    for( unsigned int pointIndex = 0; pointIndex < numLineStartPoints; ++pointIndex )
        m_importer->AddLine( aPoints[ pointIndex ], aPoints[ pointIndex + 1 ] );
}


static wxRealPoint getPoint( const float* aPointCoordinates )
{
    return wxRealPoint( aPointCoordinates[0], aPointCoordinates[1] );
}


static wxRealPoint getBezierPoint( const float* aPoints, float aStep )
{
    const int coordinatesPerPoint = 2;

    auto firstCubicPoint = getPoint( aPoints );
    auto secondCubicPoint = getPoint( aPoints + 1 * coordinatesPerPoint );
    auto thirdCubicPoint = getPoint( aPoints + 2 * coordinatesPerPoint );
    auto fourthCubicPoint = getPoint( aPoints + 3 * coordinatesPerPoint );

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
