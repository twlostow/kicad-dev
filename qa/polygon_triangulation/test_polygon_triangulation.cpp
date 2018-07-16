/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2017 CERN
 * @author Alejandro García Montoro <alejandro.garciamontoro@gmail.com>
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

#include <geometry/poly_grid_partition.h>
#include <geometry/shape_line_chain.h>
#include <geometry/shape_poly_set.h>

#include <io_mgr.h>
#include <kicad_plugin.h>

#include <class_board.h>
#include <class_zone.h>
#include <profile.h>

#include <unordered_set>
#include <utility>
#include <cassert>


#include <algorithm>
#include <fstream>
#include <iostream>

#include <ttl/halfedge/hetriang.h>
#include <ttl/halfedge/hetraits.h>
#include <ttl/ttl.h>

#define SHOW_THE_BUG

using namespace std;

BOARD* loadBoard( const std::string& filename )
{
    PLUGIN::RELEASER pi( new PCB_IO );
    BOARD*           brd = nullptr;

    try
    {
        brd = pi->Load( wxString( filename.c_str() ), NULL, NULL );
    }
    catch( const IO_ERROR& ioe )
    {
        wxString msg = wxString::Format( _( "Error loading board.\n%s" ), ioe.Problem() );

        printf( "%s\n", (const char*) msg.mb_str() );
        return nullptr;
    }

    return brd;
}


// ------------------------------------------------------------------------------------------------
// Interpret two points as being coincident
inline bool eqPoints( hed::NODE_PTR p1, hed::NODE_PTR p2 )
{
    int       dx = p1->GetX() - p2->GetX();
    int       dy = p1->GetY() - p2->GetY();
    


    return (dx ==0 && dy == 0);
}


// ------------------------------------------------------------------------------------------------
// Lexicographically compare two points (2D)
inline bool ltLexPoint( const hed::NODE_PTR p1, const hed::NODE_PTR p2 )
{
    return ( p1->GetX() < p2->GetX() ) || ( p1->GetX() == p2->GetX() && p1->GetY() < p2->GetY() );
};

hed::NODE_PTR findNode( const std::vector<hed::NODE_PTR>& nodes, VECTOR2I p )
{
    for( auto n : nodes )
    {
        if( n->GetX() == p.x && n->GetY() == p.y )
            return n;
    }

    assert(false);

    return nullptr;
}

hed::DART findDart( hed::TRIANGULATION& triang, hed::NODE_PTR node )
{
    hed::DART  d1 = triang.CreateDart();
    ttl::TRIANGULATION_HELPER helper(triang);

#ifdef SHOW_THE_BUG
    assert( helper.LocateTriangle<hed::TTLtraits>( node, d1 ) == true ); // WTF? this doesn't call LocateTriangle AT ALL
#else
    helper.LocateTriangle<hed::TTLtraits>( node, d1 );
#endif

    for( int j = 0; j <= 2; j++ )
    {
        auto nn = d1.GetNode();

        if( nn->GetX() == node->GetX() && nn->GetY() == node->GetY() )
  	    return d1;

        d1.Alpha0().Alpha1();
    }

    assert( false );
    return d1;
}

struct Tri 
{
    hed::NODE_PTR v[3];
    hed::EDGE_PTR e[3];
    hed::EDGE_PTR m_lead;
    
    Tri( hed::EDGE_PTR lead = nullptr)
    {
        if(!lead)
            return;
            
        m_lead = lead;
        auto dart = hed::DART(lead, true);
        hed::NODE_PTR prev = nullptr;
    
        for( int j = 0; j <= 2; j++ )
        {
            auto e2 = dart.GetEdge();

            e[j] = e2;
            v[j] = e2->GetSourceNode() == prev ? e2->GetTargetNode() : e2->GetSourceNode();

            prev = v[j];

            dart.Alpha0().Alpha1();
        }
    }

    void getAdjacentTriangles( std::vector<Tri>& tris )
    {
        for (int j=0;j<=2;j++)
        {
            if ( e[j] && !e[j]->IsConstrained() )
            {
                auto twin = e[j]->GetTwinEdge();
                if( twin )
                    tris.push_back( Tri( twin ) );
            }
        }
    }

    hed::EDGE_PTR getLeadingEdge() const { return m_lead; }

    const VECTOR2I center() const
    {
        double x=0,y=0;
        for(int j=0;j<=2;j++)
        {
            x+=v[j]->GetX();
            y+=v[j]->GetY();
        }
        VECTOR2I c( (int)x/3.0, (int)y/3.0 );
        return c;
    }

    const VECTOR2I vertex(int j) const
    {
        return VECTOR2I((int)v[j]->GetX(), (int) v[j]->GetY() );
    }

    bool isWellDefined() const
    {
            auto c = center();
        return vertex(0) !=c && vertex(1) != c && vertex(2) != c;
    }
};

static bool containsPoint( const SHAPE_POLY_SET::POLYGON& poly, const VECTOR2I& p)
{
    if( !poly[0].PointInside( p ) )
        return false;

    for(int i = 1; i < poly.size(); i++)
        if( poly[i].PointInside( p ) )
            return false;

    return true;
}


void test2( SHAPE_POLY_SET::POLYGON& poly, int index )
{
    
    if(poly.size() == 0)
        return;

    std::vector<hed::NODE_PTR> nodes;
    auto                    bb = poly[0].BBox();

    const int gridPointDensity = std::min( 10000000, std::max(bb.GetWidth() / 10, bb.GetHeight() / 10 ) );
    const int nGridPointsX = ( bb.GetWidth() / gridPointDensity ) + 1;
    const int nGridPointsY = ( bb.GetHeight() / gridPointDensity ) + 1;


    if(poly.size() == 1 && poly[0].PointCount() < 3 )
        return;
    
   
    for( const auto& outl : poly )
    {
        for( int i = 0; i < outl.PointCount(); i++)
        {
            auto p = outl.CPoint(i);
            hed::NODE_PTR node ( new hed::NODE( p.x, p.y ) );
            nodes.push_back( node );
            node->SetFlag(true);
            node->SetId(i);
        }
    }

    for( int xi = 0; xi < nGridPointsX; xi++ )
        for( int yi = 0; yi < nGridPointsY; yi++ )
        {
            VECTOR2I p = bb.Centre();

            p.x += ( (double) xi - (double) nGridPointsX / 2.0 ) * (double) gridPointDensity;
            p.y += ( (double) yi - (double) nGridPointsY / 2.0 ) * (double) gridPointDensity;

            if( !containsPoint( poly, p ) )
                continue;

            hed::NODE_PTR node ( new hed::NODE( p.x, p.y, 0 ) );
            node->SetFlag(false);
            node->SetId(-1);
            nodes.push_back( node );
        }


    PROF_COUNTER cnt( "triangulate-delaunay" );

    // Sort the nodes lexicographically in the plane.
    // This is recommended since the triangulation algorithm will run much faster.
    // (ltLexPoint is defined above)
    std::sort( nodes.begin(), nodes.end(), ltLexPoint );

    // Remove coincident points to avoid degenerate triangles. (eqPoints is defined above)
    std::vector<hed::NODE_PTR>::iterator new_end = std::unique( nodes.begin(), nodes.end(), eqPoints );

    if( nodes.size() < 3 )
    {
        return;
    }



    // Make the triangulation
    hed::TRIANGULATION triang;
    triang.CreateDelaunay( nodes.begin(), new_end );

    ttl::TRIANGULATION_HELPER helper( triang );

    int nconstr = 0;
    char str[1024];
    sprintf(str,"edges_%d.txt", index);
    FILE *f = fopen( str, "wb" );

    for( const auto& outl : poly )
    {
        for( auto i = 0; i < outl.SegmentCount(); i++ )
        {
            if( outl.CSegment(i).A == outl.CSegment(i).B )
                continue;
            
            auto n1 = findNode( nodes, outl.CSegment(i).A );
            auto n2 = findNode( nodes, outl.CSegment(i).B );
            
            auto d1 = findDart( triang, n1 );
            auto d2 = findDart( triang, n2 );
            
            auto dart = helper.InsertConstraint<hed::TTLtraits>( d1, d2, true );
            dart.GetEdge()->SetConstrained();

            nconstr++;
        }
    }

    

    cnt.Show();

    printf( "Triang edges : %d constr %d\n", triang.NoTriangles(), nconstr );

    const std::list<hed::EDGE_PTR> edges = triang.GetLeadingEdges( );

    
    Tri startTriangle;
    bool found = false;
    int i = 0;
    for( auto& e : edges )
    {
        Tri t (e);
        if(!t.isWellDefined() )
            continue;

        if( containsPoint( poly, t.center() ) )
        {
            startTriangle = t;
            found = true;
            break;
        }
    }
    
    assert(found);

    auto outl = poly[0];

    std::list<hed::EDGE_PTR> allEdges;

    triang.GetEdges( allEdges );

    printf("All Edges : %d\n", allEdges.size() );

    for( auto& e : allEdges )
    {
        VECTOR2I p0 ( e->GetSourceNode()->GetX(), e->GetSourceNode()->GetY() );
        VECTOR2I p1 ( e->GetTargetNode()->GetX(), e->GetTargetNode()->GetY() );
        
        if(!e->IsConstrained() )
            continue;

        fprintf(f,"%d %d %d %d %d\n", p0.x, p0.y, p1.x, p1.y, 1);
    }

    
    std::deque<Tri> Q;
    std::set<hed::EDGE_PTR> processedEdges;
    Q.push_back(startTriangle);

    do {
        if ( Q.empty() ) 
            break;
        
        auto t = Q.back();

        Q.pop_back();

        processedEdges.insert( t.getLeadingEdge() );

        auto p0 = t.vertex(0);
        auto p1 = t.vertex(1);
        auto p2 = t.vertex(2);

        fprintf(f,"%d %d %d %d %d\n", p0.x, p0.y, p1.x, p1.y, 0);
        fprintf(f,"%d %d %d %d %d\n", p0.x, p0.y, p2.x, p2.y, 0);
        fprintf(f,"%d %d %d %d %d\n", p2.x, p2.y, p1.x, p1.y, 0);

        std::vector<Tri> adj_v;
        t.getAdjacentTriangles( adj_v );
        for(auto t_adj : adj_v)
        {
            if( processedEdges.find( t_adj.getLeadingEdge() ) == processedEdges.end() )
            {
                Q.push_back(t_adj);
            }
        }
    } while(1);

   

    fclose( f );
}

int main( int argc, char* argv[] )
{
    auto brd = loadBoard( argc > 1 ? argv[1] : "../../../../tests/dp.kicad_pcb" );

    if( !brd )
        return -1;


    PROF_COUNTER cnt( "allBoard" );


    //#pragma omp parallel for schedule( dynamic )
    for( int z = 0; z < brd->GetAreaCount(); z++ )
    {
        auto           zone = brd->GetArea( z );
        SHAPE_POLY_SET poly = zone->GetFilledPolysList();
        poly.Unfracture( SHAPE_POLY_SET::PM_FAST );
        //  poly.CacheTriangulation();

        printf( "zone %d/%d\n", ( z + 1 ), brd->GetAreaCount() );

        if( poly.OutlineCount() == 0 )
            continue;
        
        test2( poly.Polygon(0), z );
#if 0
        PROF_COUNTER unfrac("unfrac");
        
        unfrac.Show();

        PROF_COUNTER triangulate("triangulate");

        for(int i =0; i< poly.OutlineCount(); i++)
        {
            poly.triangulatePoly( &poly.Polygon(i) );
        }
        triangulate.Show();
#endif
    }

    cnt.Show();

    delete brd;

    return 0;
}
