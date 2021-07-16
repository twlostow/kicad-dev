/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2020 KiCad Developers.
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


// WARNING - this Tom's crappy PNS hack tool code. Please don't complain about its quality
// (unless you want to improve it).

#include <pgm_base.h>
#include <qa_utils/utility_registry.h>

#include "pns_log_viewer_frame.h"
#include "label_manager.h"

#include <geometry/shape_arc.h>

std::shared_ptr<PNS_LOG_VIEWER_OVERLAY> overlay;


template<class T>
static T within_range(T x, T minval, T maxval, T wrap)
{
    int rv;

    while (maxval >= wrap)
        maxval -= wrap;

    while (maxval < 0)
        maxval += wrap;

    while (minval >= wrap)
        minval -= wrap;

    while (minval < 0)
        minval += wrap;

    while (x < 0)
        x += wrap;

    while (x >= wrap)
        x -= wrap;

    if (maxval > minval)
        rv = (x >= minval && x <= maxval) ? 1 : 0;
    else
        rv = (x >= minval || x <= maxval) ? 1 : 0;

    return rv;
}

static bool sliceContainsPoint( const SHAPE_ARC& a, const VECTOR2I& p )
{
    double phi = 180.0 / M_PI * atan2( p.y - a.GetCenter().y, p.x - a.GetCenter().x );
    double ca = a.GetCentralAngle();
    double sa = a.GetStartAngle();
    double ea;
    if( ca >= 0 )
    {
        ea = sa + ca;
    }
    else
    {
        ea = sa;
        sa += ca;
    }


    return within_range( phi, sa, ea, 360.0 );
}

static int arclineIntersect( const SHAPE_ARC& a, const SEG& s, std::vector<VECTOR2I>* ips )
{
    CIRCLE circ( a.GetCenter(), a.GetRadius() );

    std::vector<VECTOR2I> intersections = circ.IntersectLine( s );

    size_t originalSize = ips->size();

    for( const VECTOR2I& intersection : intersections )
    {
        if( sliceContainsPoint( a, intersection ) )
            ips->push_back( intersection );
    }

    return ips->size() - originalSize;
}

int intersectArc2Arc( const SHAPE_ARC& a1, const SHAPE_ARC& a2, std::vector<VECTOR2I>* ips )
{
    CIRCLE circ1( a1.GetCenter(), a1.GetRadius() );
    CIRCLE circ2( a2.GetCenter(), a2.GetRadius() );

    std::vector<VECTOR2I> intersections = circ1.Intersect( circ2 );

    size_t originalSize = ips->size();

    for( const VECTOR2I& intersection : intersections )
    {
        if( sliceContainsPoint( a1, intersection ) && sliceContainsPoint( a2, intersection ) )
            ips->push_back( intersection );
    }

    return ips->size() - originalSize;
}


bool collideArc2Arc( const SHAPE_ARC& a1, const SHAPE_ARC& a2, int clearance, SEG& minDistSeg )
{
    SEG mediatrix( a1.GetCenter(), a2.GetCenter() );

    std::vector<VECTOR2I> ips;

    if( intersectArc2Arc( a1, a2, &ips ) > 0 )
    {
        minDistSeg.A = minDistSeg.B = ips[0];
        return true;
    }

    std::vector<VECTOR2I> ptsA;
    std::vector<VECTOR2I> ptsB;

    bool cocentered = ( mediatrix.A == mediatrix.B );

    // arcs don't have the same center point
    if( !cocentered )
    {
        arclineIntersect( a1, mediatrix, &ptsA );
        arclineIntersect( a2, mediatrix, &ptsB );
    }

    arclineIntersect( a1, SEG( a2.GetP0(), a2.GetCenter() ), &ptsA );
    arclineIntersect( a1, SEG( a2.GetP1(), a2.GetCenter() ), &ptsA );

    arclineIntersect( a2, SEG( a1.GetP0(), a1.GetCenter() ), &ptsB );
    arclineIntersect( a2, SEG( a1.GetP1(), a1.GetCenter() ), &ptsB );

    ptsA.push_back( a1.GetP0() );
    ptsA.push_back( a1.GetP1() );
    ptsB.push_back( a2.GetP0() );
    ptsB.push_back( a2.GetP1() );

    std::vector<VECTOR2I> nearestA, nearestB;

    // this probably can be made with less points, but still...
    CIRCLE circ1( a1.GetCenter(), a1.GetRadius() );
    for( auto pB: ptsB )
    {
        auto nearest = circ1.NearestPoint( pB );
        if( sliceContainsPoint( a1, nearest ) )
            nearestA.push_back( nearest );
    }
    CIRCLE circ2( a2.GetCenter(), a2.GetRadius() );
    for( auto pA: ptsA )
    {
        auto nearest = circ2.NearestPoint( pA );
        if( sliceContainsPoint( a2, nearest ) )
            nearestB.push_back( nearest );
    }

    ptsA.insert( ptsA.end(), nearestA.begin(), nearestA.end() );
    ptsB.insert( ptsB.end(), nearestB.begin(), nearestB.end() );

    double minDist = std::numeric_limits<double>::max();
    bool   minDistFound = false;

    for( const VECTOR2I& ptA: ptsA )
        for( const VECTOR2I& ptB : ptsB )
        {
            double dist = ( ptA - ptB ).EuclideanNorm() - a1.GetWidth() / 2.0 - a2.GetWidth() / 2.0;

            if( dist < clearance )
            {
                if( !minDistFound || dist < minDist )
                {
                    minDist = dist;
                    minDistSeg = SEG( ptA, ptB );
                }

                minDistFound = true;
            }
        }


    return minDistFound;
}


int playground_main_func( int argc, char* argv[] )
{
    auto frame = new PNS_LOG_VIEWER_FRAME( nullptr );
    Pgm().App().SetTopWindow( frame );      // wxApp gets a face.
    frame->Show();

    struct ARC_DATA
    {
        double cx, cy, sx, sy, ca, w;
    };

    const ARC_DATA test_data [] =
    {
        {73.843527, 74.355869, 71.713528, 72.965869, -76.36664803, 0.2},
        {71.236473, 74.704131, 73.366472, 76.094131, -76.36664803, 0.2},
        {82.542335, 74.825975, 80.413528, 73.435869, -76.4, 0.2},
        {76.491192, 73.839894, 78.619999, 75.23, -76.4, 0.2},
        {89.318807, 74.810106, 87.19, 73.42, -76.4, 0.2},
        {87.045667, 74.632941, 88.826472, 75.794131, -267.9, 0.2},
        {94.665667, 73.772941, 96.446472, 74.934131, -267.9, 0.2},
        {94.750009, 73.74012, 93.6551, 73.025482, -255.5, 0.2},
        {72.915251, 80.493054, 73.570159, 81.257692, -260.5, 0.2}, // end points clearance false positive
        {73.063537, 82.295989, 71.968628, 81.581351, -255.5, 0.2},
        {79.279991, 80.67988, 80.3749, 81.394518, -255.5, 0.2},
        {79.279991, 80.67988, 80.3749, 81.694518, -255.5, 0.2 },
        {88.495265, 81.766089, 90.090174, 82.867869, -255.5, 0.2},
        {86.995265, 81.387966, 89.090174, 82.876887, -255.5, 0.2},
        {96.149734, 81.792126, 94.99, 83.37, -347.2, 0.2},
        {94.857156, 81.240589, 95.91, 83.9, -288.5, 0.2},
        {72.915251, 86.493054, 73.970159, 87.257692, -260.5, 0.2}, // end points clearance #1
        {73.063537, 88.295989, 71.968628, 87.581351, -255.5, 0.2},
        {78.915251, 86.393054, 79.970159, 87.157692, 99.5, 0.2}, // end points clearance #2 - false positive
        {79.063537, 88.295989, 77.968628, 87.581351, -255.5, 0.2},
        {85.915251, 86.993054, 86.970159, 87.757692, 99.5, 0.2}, // intersection - false negative
        {86.063537, 88.295989, 84.968628, 87.581351, -255.5, 0.2},
        {94.6551, 88.295989, 95.6551, 88.295989, 90.0, 0.2 }, // simulating diff pair
        {94.6551, 88.295989, 95.8551, 88.295989, 90.0, 0.2 },
    };


    overlay = frame->GetOverlay();


    overlay->SetIsFill(false);
    overlay->SetLineWidth(10000);

    std::vector<SHAPE_ARC> arcs;
    int n_arcs = sizeof( test_data ) / sizeof( ARC_DATA );

    BOX2I vp;

    for( int i = 0; i < n_arcs; i++ )
    {
        const ARC_DATA& d = test_data[i];

        SHAPE_ARC arc( VECTOR2D( Millimeter2iu( d.cx ), Millimeter2iu( d.cy ) ),
                       VECTOR2D( Millimeter2iu( d.sx ), Millimeter2iu( d.sy ) ), d.ca,
                       Millimeter2iu( d.w ) );

        arcs.push_back( arc );

        if( i == 0 )
            vp = arc.BBox();
        else
            vp.Merge( arc.BBox() );
    }

    printf("Read %lu arcs\n", arcs.size() );

    LABEL_MANAGER labelMgr( frame->GetPanel()->GetGAL() );
    frame->GetPanel()->GetView()->SetViewport( BOX2D( vp.GetOrigin(), vp.GetSize() ) );

    for(int i = 0; i < arcs.size(); i+= 2)
    {
        SEG closestDist;
        std::vector<VECTOR2I> ips;
        bool collides = collideArc2Arc( arcs[i], arcs[i+1], 0, closestDist );
        int ni = intersectArc2Arc( arcs[i], arcs[i+1], &ips );

        overlay->SetLineWidth( 10000.0 );
        overlay->SetStrokeColor( GREEN );

        for( int j = 0; j < ni; j++ )
            overlay->AnnotatedPoint( ips[j], arcs[i].GetWidth() );

        if( collides )
        {
            overlay->SetStrokeColor( YELLOW );
            overlay->Line( closestDist.A, closestDist.B );
            overlay->SetLineWidth( 10000.0 );
            overlay->SetGlyphSize( { 100000.0, 100000.0 } );
            overlay->BitmapText( wxString::Format( "dist: %d", closestDist.Length() ),
                                 closestDist.A + VECTOR2I( 0, -arcs[i].GetWidth() ), 0 );
        }

        overlay->SetLineWidth( 10000.0 );
        overlay->SetStrokeColor( CYAN );
        overlay->AnnotatedPoint( arcs[i].GetP0(), arcs[i].GetWidth() / 2 );
        overlay->AnnotatedPoint( arcs[i + 1].GetP0(), arcs[i + 1].GetWidth() / 2 );
        overlay->AnnotatedPoint( arcs[i].GetArcMid(), arcs[i].GetWidth() / 2 );
        overlay->AnnotatedPoint( arcs[i + 1].GetArcMid(), arcs[i + 1].GetWidth() / 2 );
        overlay->AnnotatedPoint( arcs[i].GetP1(), arcs[i].GetWidth() / 2 );
        overlay->AnnotatedPoint( arcs[i + 1].GetP1(), arcs[i + 1].GetWidth() / 2 );


        overlay->SetStrokeColor( RED );
        overlay->Arc( arcs[i] );
        overlay->SetStrokeColor( MAGENTA );
        overlay->Arc( arcs[i + 1] );
    }


    overlay = nullptr;

    return 0;
}

static bool registered = UTILITY_REGISTRY::Register( {
        "playground",
        "Geometry/drawing playground",
        playground_main_func,
} );
