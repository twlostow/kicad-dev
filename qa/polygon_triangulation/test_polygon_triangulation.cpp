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


#include <ttl2/include/ttl/halfedge/HeDart.h>
#include <ttl2/include/ttl/halfedge/HeTraits.h>
#include <ttl2/include/ttl/halfedge/HeTriang.h>
#include <ttl2/include/ttl/ttl.h>

//using namespace hed; // (to avoid using prefix hed::)

#include <algorithm>
#include <fstream>
#include <iostream>
using namespace std;


// ------------------------------------------------------------------------------------------------
// Interpret two points as being coincident
inline bool eqPoints( hed::Node*& p1, hed::Node*& p2 )
{
    double       dx = p1->x() - p2->x();
    double       dy = p1->y() - p2->y();
    double       dist2 = dx * dx + dy * dy;
    const double eps = 1.0e-12;
    if( dist2 < eps )
        return true;

    return false;
}


// ------------------------------------------------------------------------------------------------
// Lexicographically compare two points (2D)
inline bool ltLexPoint( const hed::Node* p1, const hed::Node* p2 )
{
    return ( p1->x() < p2->x() ) || ( p1->x() == p2->x() && p1->y() < p2->y() );
};

hed::Node* findNode( const std::vector<hed::Node*>& nodes, VECTOR2I p )
{
    for( auto n : nodes )
    {
        if( n->x() == p.x && n->y() == p.y )
            return n;
    }

    return nullptr;
}

int findNodeIndex( const std::vector<hed::Node*>& nodes, VECTOR2I p )
{
    int i = 0;
    for( auto n : nodes )
    {
        if( n->x() == p.x && n->y() == p.y )
            return i;

        i++;
    }

    return i;
}

hed::Dart findDart( hed::Triangulation& triang, hed::Node* node )
{
    hed::Dart  d1 = triang.createDart();
    auto       tri = ttl::locateTriangle<hed::TTLtraits>( *node, d1 );

    bool found = false;
    for( int j = 0; j <= 2; j++ )
    {
        auto nn = d1.getNode();

        if( nn->x() == node->x() && nn->y() == node->y() )
        {
            found = true;
            break;
        }

        d1.alpha0().alpha1();
    }

    assert( found );
    return d1;
}

struct Tri 
{
    hed::Node *v[3];
    hed::Edge *e[3];
    hed::Edge *m_lead;
    Tri( hed::Edge *lead = nullptr)
    {
        if(!lead)
            return;
            
        m_lead = lead;
        auto dart = hed::Dart(lead, true);
        hed::Node* prev = nullptr;
        for( int j = 0; j <= 2; j++ )
        {
            auto e2 = dart.getEdge();

            e[j] = e2;
            v[j] = e2->getSourceNode() == prev ? e2->getTargetNode() : e2->getSourceNode();

            prev = v[j];

            dart.alpha0().alpha1();
        }
    }

    void getAdjacentTriangles( std::vector<Tri>& tris )
    {
        for (int j=0;j<=2;j++)
        {
            if(e[j] && !e[j]->isConstrained() )
            {
                auto twin = e[j]->getTwinEdge();
                if(twin)
                    tris.push_back( Tri( twin ));
            }
        }
    }

    hed::Edge* getLeadingEdge() const { return m_lead; }

    const VECTOR2I center() const
    {
        double x=0,y=0;
        for(int j=0;j<=2;j++)
        {
            x+=v[j]->x();
            y+=v[j]->y();
        }
        VECTOR2I c( (int)x/3.0, (int)y/3.0 );
        return c;
    }

    const VECTOR2I vertex(int j) const
    {
        return VECTOR2I((int)v[j]->x(), (int) v[j]->y() );
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

    //POLY_GRID_PARTITION part (poly);
    std::vector<hed::Node*> nodes;
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
            hed::Node* node = new hed::Node( p.x, p.y );
            nodes.push_back( node );
            node->setFlag(true);
            node->setId(i);
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

            auto node = new hed::Node( p.x, p.y, 0 );
            node->setFlag(false);
            node->setId(-1);
            nodes.push_back( node );
        }


    PROF_COUNTER cnt( "triangulate-delaunay" );

    // Sort the nodes lexicographically in the plane.
    // This is recommended since the triangulation algorithm will run much faster.
    // (ltLexPoint is defined above)
    std::sort( nodes.begin(), nodes.end(), ltLexPoint );

    // Remove coincident points to avoid degenerate triangles. (eqPoints is defined above)
    std::vector<hed::Node*>::iterator new_end = std::unique( nodes.begin(), nodes.end(), eqPoints );

    if( nodes.size() < 3 )
    {
        return;
    }



    // Make the triangulation
    hed::Triangulation triang;
    triang.createDelaunay( nodes.begin(), new_end );

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
            auto dart = ttl::insertConstraint<hed::TTLtraits>( d1, d2, false );
            dart.getEdge()->setConstrained();
        }
    }

/*
    for( auto i = 0; i < outl.SegmentCount(); i++ )
    {
        auto a = outl.CSegment(i).A;
        auto b = outl.CSegment(i).B;
        auto dart = triang.createDart();
        auto nodea = findNode( nodes, a );
        auto nodeb = findNode( nodes, b );
        auto tri = ttl::locateTriangle<hed::TTLtraits>( *nodea, dart );

        std::vector<hed::Edge*> toCheck;

//REAL_TYPE Orient2DFast( REAL_TYPE aPA[2], REAL_TYPE aPB[2], REAL_TYPE aPC[2] )

        for( int j = 0; j <= 2; j++ )
        {
            auto edge = dart.getEdge();

            if ( ( edge->getSourceNode() == nodea && edge->getTargetNode() == nodeb )
            || ( edge->getSourceNode() == nodeb && edge->getTargetNode() == nodea ) )
            {
                printf("i %d j %d\n", i, j);
            }

            dart.alpha0().alpha1();
        }


    }
*/



#if 0
    auto outl = poly.COutline(0);
    for( auto i = 0; i < outl.SegmentCount(); i++ )
    {
        SEG s = outl.CSegment(i);
        hed::Node* n1 = findNode(nodes, s.A );
        hed::Node* n2 = findNode(nodes, s.B );

       /* if ( !n1 )
            n1 = new hed::Node ( s.A.x, s.A.y );
        if ( !n2 )
            n2 = new hed::Node ( s.B.x, s.B.y );*/

        
        printf("n %p %p\n", n1, n2 );
        hed::Dart d1 = triang.createDart();
        auto tri = ttl::locateFaceSimplest<hed::TTLtraits>(*n1, d1);
        hed::Dart d2 = triang.createDart();
        auto tri2 = ttl::locateFaceSimplest<hed::TTLtraits>(*n2, d2);

        printf("d1 %d d2 %d %p %p %p %p\n", d1.getNode() == n2 ? 1: 0, d2.getNode() == n1 ? 1 : 0, n1, n2 ,d1.getNode(), d2.getNode() );

        auto e1 = d1.getEdge();
        auto e2 = d2.getEdge();
        
        printf("tri %d %d\n", !!tri, !!tri2 );

        auto dart = ttl::insertConstraint<hed::TTLtraits>(d1, d2, false);
        dart.getEdge()->setConstrained();
    }
#endif


    cnt.Show();

    printf( "Triang edges : %d\n", triang.noTriangles() );


    const std::list<hed::Edge*> edges = triang.getLeadingEdges( );

    char str[1024];
    sprintf(str,"edges_%d.txt", index);
    FILE* f = fopen( str, "wb" );
    
    Tri startTriangle;
    bool found = false;

    for( auto& e : edges )
    {
        Tri t (e);
        if(!t.isWellDefined() )
            continue;

        if( containsPoint( poly, t.center() ) )
        {
            startTriangle = t;
            found = true;
        }
    }
    
    assert(found);

    auto outl = poly[0];

    for(int i = 0; i < outl.SegmentCount(); i++)
    {
        auto p0 = outl.CPoint(i);
        auto p1 = outl.CPoint(i+1);

        fprintf(f,"%d %d %d %d %d\n", p0.x, p0.y, p1.x, p1.y, 1);
    }

    
    std::deque<Tri> Q;
    std::set<hed::Edge*> processedEdges;
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
        fprintf(f,"%d %d %d %d %d\n", p2.x, p2.y, p1.x, p1.y, 0);
        fprintf(f,"%d %d %d %d %d\n", p0.x, p0.y, p2.x, p2.y, 0);

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

    #if 0
    for( auto& e : edges )
    {
        auto dart = hed::Dart(e, true);
        hed::Node* n1, *n2;
        bool refFound = false;

        VECTOR2I ref;
    //    std::vector<VECTOR2I> pts;

        for( int j = 0; j <= 2; j++ )
        {
            auto e2 = dart.getEdge();

            n1 = e2->getSourceNode();
            n2 = e2->getTargetNode();

            if( e2->isConstrained() )
            {
                bool dir = false, dir2=false;

                if( ( ( n1->id() + 1) % outl.PointCount() ) == n2->id() )
                    dir = true;
                if( ( ( n2->id() + 1) % outl.PointCount() ) == n1->id() )
                    dir2 = true;

                if( !dir )
                    std::swap(n1, n2);

                refFound = true;
                break;
            }

            
            dart.alpha0().alpha1();
        }


        VECTOR2I pr0( n1->x(), n1->y() );
        VECTOR2I pr1( n2->x(), n2->y() );

        bool reject = false;

        if(refFound)
        {
            
            dart = hed::Dart(e, true);

            VECTOR2I pt;

            for( int j = 0; j <= 2; j++ )
            {
                auto e2 = dart.getEdge();

                auto n1a = e2->getSourceNode();
                auto n2a = e2->getTargetNode();

                if( n1->x() == n1a->x() && n1->y() == n1a->y() && ( n2->x() != n2a->x() || n2->y() != n2a->y() ) )
                {
                    pt = VECTOR2I( n2a->x(), n2a->y() );
                }
                
                if( n1->x() == n2a->x() && n1->y() == n2a->y() && ( n2->x() != n1a->x() || n2->y() != n1a->y() ) )
                {
                    pt = VECTOR2I( n1a->x(), n1a->y() );
                }
                
                dart.alpha0().alpha1();
            }
            
            auto det = (pt - pr0).Cross(pr1 - pr0);

            if (det > 0)
                reject = true;
        }

        
        if(reject)
            continue;

        dart = hed::Dart(e, true);

        for( int j = 0; j <= 2; j++ )
        {
            auto e2 = dart.getEdge();

                auto n1a = e2->getSourceNode();
                auto n2a = e2->getTargetNode();
                VECTOR2I p1( n1a->x(), n1a->y() );
                VECTOR2I p0( n2a->x(), n2a->y() );
            
                fprintf( f, "%d %d %d %d %d\n",p0.x,p0.y,p1.x,p1.y, e2->isConstrained()?1:0);
                dart.alpha0().alpha1();
        }
            
    }
    
    /*for( auto& e : *edges )
    {
        int outside = 0;
        auto n0 = e->getSourceNode();
        auto n1 = e->getTargetNode();

        //if(!e->isConstrained() )
        //    continue;

        VECTOR2I p0 ( n0->x(), n0->y() );
        VECTOR2I p1 ( n1->x(), n1->y() );
        
        VECTOR2I midp( (p0 + p1) / 2);

        if( !n0->getFlag() || !n1->getFlag() )
        {
            outside = 0;
        } else {
            if( !poly.Contains( midp ) && !e->isConstrained() )
                outside = 1;
        }

        if( !outside )
        fprintf( f, "%d %d %d %d %d\n",p0.x,p0.y,p1.x,p1.y, e->isConstrained()?1:0);
    }*/

#endif


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
