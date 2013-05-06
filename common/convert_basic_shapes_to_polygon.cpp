/**
 * @file convert_basic_shapes_to_polygon.cpp
 */
/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 1992-2012 KiCad Developers, see change_log.txt for contributors.
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
#include <vector>

#include <fctsys.h>
#include <trigo.h>
#include <macros.h>
#include <common.h>
#include <convert_basic_shapes_to_polygon.h>

/* Helper functions:
 * We are using a lots polygons in calculations.
 * and we are using 2 descriptions,
 * one easy to use with boost::polygon (KI_POLYGON_SET)
 * one easy to use in zones and in draw functions (CPOLYGONS_LIST)
 * Copy polygons from a KI_POLYGON_SET set of polygons to
 * a CPOLYGONS_LIST polygon list
 * Therefore we need conversion functions between these 2 descriptions
 */
void CopyPolygonsFromKiPolygonListToPolysList( KI_POLYGON_SET&  aKiPolyList,
                                               CPOLYGONS_LIST&  aPolysList )
{
    for( unsigned ii = 0; ii < aKiPolyList.size(); ii++ )
    {
        KI_POLYGON& poly = aKiPolyList[ii];
        CPolyPt     corner( 0, 0, false );

        for( unsigned jj = 0; jj < poly.size(); jj++ )
        {
            KI_POLY_POINT point = *(poly.begin() + jj);
            corner.x    = point.x();
            corner.y    = point.y();
            corner.end_contour = false;
            aPolysList.push_back( corner );
        }

        aPolysList.back().end_contour = true;
    }
}


/**
 * Helper function AddPolygonCornersToKiPolygonList
 * This function adds a KI_POLYGON_SET description to a
 * CPOLYGONS_LIST description
 * @param aCornersBuffer = source (set of polygons using CPolyPt corners descr)
 * @param aPolysList = destination (set of polygons)
 */
void AddPolygonCornersToKiPolygonList( CPOLYGONS_LIST&  aCornersBuffer,
                                       KI_POLYGON_SET&  aKiPolyList )
{
    std::vector<KI_POLY_POINT> cornerslist;
    unsigned    corners_count = aCornersBuffer.size();

    // Count the number of polygons in aCornersBuffer
    int         polycount = 0;

    for( unsigned ii = 0; ii < corners_count; ii++ )
    {
        if( aCornersBuffer[ii].end_contour )
            polycount++;
    }

    aKiPolyList.reserve( polycount );

    for( unsigned icnt = 0; icnt < corners_count; )
    {
        KI_POLYGON  poly;
        cornerslist.clear();

        unsigned    ii;

        for( ii = icnt; ii < aCornersBuffer.size(); ii++ )
        {
            cornerslist.push_back( KI_POLY_POINT( aCornersBuffer[ii].x, aCornersBuffer[ii].y ) );

            if( aCornersBuffer[ii].end_contour )
                break;
        }

        bpl::set_points( poly, cornerslist.begin(), cornerslist.end() );
        aKiPolyList.push_back( poly );
        icnt = ii + 1;
    }
}


/**
 * Function TransformCircleToPolygon
 * convert a circle to a polygon, using multiple straight lines
 * @param aCornerBuffer = a buffer to store the polygon
 * @param aCenter = the center of the circle
 * @param aRadius = the radius of the circle
 * @param aCircleToSegmentsCount = the number of segments to approximate a circle
 * Note: the polygon is inside the circle, so if you want to have the polygon
 * outside the circle, you should give aRadius calculated with a corrrection factor
 */
void TransformCircleToPolygon( CPOLYGONS_LIST& aCornerBuffer,
                               wxPoint aCenter, int aRadius,
                               int aCircleToSegmentsCount )
{
    wxPoint corner_position;
    int     delta       = 3600 / aCircleToSegmentsCount;    // rot angle in 0.1 degree
    int     halfstep    = 1800 / aCircleToSegmentsCount;    // the starting value for rot angles

    for( int ii = 0; ii < aCircleToSegmentsCount; ii++ )
    {
        corner_position.x   = aRadius;
        corner_position.y   = 0;
        int     angle = (ii * delta) + halfstep;
        RotatePoint( &corner_position.x, &corner_position.y, angle );
        corner_position += aCenter;
        CPolyPt polypoint( corner_position.x, corner_position.y );
        aCornerBuffer.push_back( polypoint );
    }

    aCornerBuffer.back().end_contour = true;
}


/**
 * Function TransformRoundedEndsSegmentToPolygon
 * convert a segment with rounded ends to a polygon
 * Convert arcs to multiple straight lines
 * @param aCornerBuffer = a buffer to store the polygon
 * @param aStart = the segment start point coordinate
 * @param aEnd = the segment end point coordinate
 * @param aCircleToSegmentsCount = the number of segments to approximate a circle
 * @param aWidth = the segment width
 * Note: the polygon is inside the arc ends, so if you want to have the polygon
 * outside the circle, you should give aStart and aEnd calculated with a correction factor
 */
void TransformRoundedEndsSegmentToPolygon( CPOLYGONS_LIST& aCornerBuffer,
                                           wxPoint aStart, wxPoint aEnd,
                                           int aCircleToSegmentsCount,
                                           int aWidth )
{
    int     radius  = aWidth / 2;
    wxPoint endp    = aEnd - aStart; // end point coordinate for the same segment starting at (0,0)
    wxPoint startp  = aStart;
    wxPoint corner;
    CPolyPt polypoint;

    // normalize the position in order to have endp.x >= 0;
    if( endp.x < 0 )
    {
        endp    = aStart - aEnd;
        startp  = aEnd;
    }

    double delta_angle = ArcTangente( endp.y, endp.x ); // delta_angle is in 0.1 degrees
    int seg_len        = KiROUND( EuclideanNorm( endp ) );

    int delta = 3600 / aCircleToSegmentsCount;    // rot angle in 0.1 degree

    // Compute the outlines of the segment, and creates a polygon
    // add right rounded end:
    for( int ii = 0; ii < 1800; ii += delta )
    {
        corner = wxPoint( 0, radius );
        RotatePoint( &corner, ii );
        corner.x += seg_len;
        RotatePoint( &corner, -delta_angle );
        corner += startp;
        polypoint.x = corner.x;
        polypoint.y = corner.y;
        aCornerBuffer.push_back( polypoint );
    }

    // Finish arc:
    corner = wxPoint( seg_len, -radius );
    RotatePoint( &corner, -delta_angle );
    corner += startp;
    polypoint.x = corner.x;
    polypoint.y = corner.y;
    aCornerBuffer.push_back( polypoint );

    // add left rounded end:
    for( int ii = 0; ii < 1800; ii += delta )
    {
        corner = wxPoint( 0, -radius );
        RotatePoint( &corner, ii );
        RotatePoint( &corner, -delta_angle );
        corner += startp;
        polypoint.x = corner.x;
        polypoint.y = corner.y;
        aCornerBuffer.push_back( polypoint );
    }

    // Finish arc:
    corner = wxPoint( 0, radius );
    RotatePoint( &corner, -delta_angle );
    corner += startp;
    polypoint.x = corner.x;
    polypoint.y = corner.y;
    aCornerBuffer.push_back( polypoint );

    aCornerBuffer.back().end_contour = true;
}


/**
 * Function TransformArcToPolygon
 * Creates a polygon from an Arc
 * Convert arcs to multiple straight segments
 * @param aCornerBuffer = a buffer to store the polygon
 * @param aCentre = centre of the arc or circle
 * @param aStart = start point of the arc, or a point on the circle
 * @param aArcAngle = arc angle in 0.1 degrees. For a circle, aArcAngle = 3600
 * @param aCircleToSegmentsCount = the number of segments to approximate a circle
 * @param aWidth = width (thickness) of the line
 */
void TransformArcToPolygon( CPOLYGONS_LIST& aCornerBuffer,
                            wxPoint aCentre, wxPoint aStart, double aArcAngle,
                            int aCircleToSegmentsCount, int aWidth )
{
    wxPoint arc_start, arc_end;
    int     delta = 3600 / aCircleToSegmentsCount;   // rotate angle in 0.1 degree

    arc_end = arc_start = aStart;

    if( aArcAngle != 3600 )
    {
        RotatePoint( &arc_end, aCentre, -aArcAngle );
    }

    if( aArcAngle < 0 )
    {
        EXCHG( arc_start, arc_end );
        NEGATE( aArcAngle );
    }

    // Compute the ends of segments and creates poly
    wxPoint curr_end    = arc_start;
    wxPoint curr_start  = arc_start;

    for( int ii = delta; ii < aArcAngle; ii += delta )
    {
        curr_end = arc_start;
        RotatePoint( &curr_end, aCentre, -ii );
        TransformRoundedEndsSegmentToPolygon( aCornerBuffer, curr_start, curr_end,
                                              aCircleToSegmentsCount, aWidth );
        curr_start = curr_end;
    }

    if( curr_end != arc_end )
        TransformRoundedEndsSegmentToPolygon( aCornerBuffer,
                                              curr_end, arc_end,
                                              aCircleToSegmentsCount, aWidth );
}


/**
 * Function TransformRingToPolygon
 * Creates a polygon from a ring
 * Convert arcs to multiple straight segments
 * @param aCornerBuffer = a buffer to store the polygon
 * @param aCentre = centre of the arc or circle
 * @param aRadius = radius of the circle
 * @param aCircleToSegmentsCount = the number of segments to approximate a circle
 * @param aWidth = width (thickness) of the ring
 */
void TransformRingToPolygon( CPOLYGONS_LIST& aCornerBuffer,
                             wxPoint aCentre, int aRadius,
                             int aCircleToSegmentsCount, int aWidth )
{
    int     delta = 3600 / aCircleToSegmentsCount;   // rotate angle in 0.1 degree

    // Compute the corners posituions and creates poly
    wxPoint curr_point;
    int     inner_radius    = aRadius - ( aWidth / 2 );
    int     outer_radius    = inner_radius + aWidth;
    CPolyPt polycorner;

    // Draw the inner circle of the ring
    for( int ii = 0; ii < 3600; ii += delta )
    {
        curr_point.x    = inner_radius;
        curr_point.y    = 0;
        RotatePoint( &curr_point, ii );
        curr_point      += aCentre;
        polycorner.x    = curr_point.x;
        polycorner.y    = curr_point.y;
        aCornerBuffer.push_back( polycorner );
    }

    // Draw the last point of inner circle
    polycorner.x    = aCentre.x + inner_radius;
    polycorner.y    = aCentre.y;
    aCornerBuffer.push_back( polycorner );

    // Draw the outer circle of the ring
    for( int ii = 0; ii < 3600; ii += delta )
    {
        curr_point.x    = outer_radius;
        curr_point.y    = 0;
        RotatePoint( &curr_point, -ii );
        curr_point      += aCentre;
        polycorner.x    = curr_point.x;
        polycorner.y    = curr_point.y;
        aCornerBuffer.push_back( polycorner );
    }

    // Draw the last point of outer circle
    polycorner.x    = aCentre.x + outer_radius;
    polycorner.y    = aCentre.y;
    aCornerBuffer.push_back( polycorner );

    // Close the polygon
    polycorner.x = aCentre.x + inner_radius;
    polycorner.end_contour = true;
    aCornerBuffer.push_back( polycorner );
}
