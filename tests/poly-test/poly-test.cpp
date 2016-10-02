#include <class_board.h>
#include <class_pad.h>
#include <class_module.h>
#include <class_zone.h>

#include <geometry/shape_poly_set.h>

#include <io_mgr.h>
#include <profile.h>

#include <memory>
#include <algorithm>
#include <functional>
#include <vector>
#include <deque>
#include <stdarg.h>

using namespace std;
#if 1


bool pointInPolygon( const VECTOR2I& aP, const SHAPE_LINE_CHAIN& aPath )
{
    int result = 0;
    int cnt = aPath.PointCount();

    if ( !aPath.BBox().Contains( aP ) ) // test with bounding box first
        return false;

    if( cnt < 3 )
        return false;

    VECTOR2I ip = aPath.CPoint( 0 );

    for( int i = 1; i <= cnt; ++i )
    {
        VECTOR2I ipNext = ( i == cnt ? aPath.CPoint( 0 ) : aPath.CPoint( i ) );

        if( ipNext.y == aP.y )
        {
            if( ( ipNext.x == aP.x ) || ( ip.y == aP.y &&
                ( ( ipNext.x > aP.x ) == ( ip.x < aP.x ) ) ) )
                return true;
        }

        if( ( ip.y < aP.y ) != ( ipNext.y < aP.y ) ) // edge within our range of interest
        {
            if( ip.x >= aP.x )
            {
                if( ipNext.x > aP.x )
                    result = 1 - result;
                else
                {
                    int64_t d = (int64_t)( ip.x - aP.x ) * (int64_t)( ipNext.y - aP.y ) -
                                (int64_t)( ipNext.x - aP.x ) * (int64_t)( ip.y - aP.y );

                    if( !d )
                        return true;

                    if( ( d > 0 ) == ( ipNext.y > ip.y ) )
                        result = 1 - result;
                }
            }
            else
            {
                if( ipNext.x > aP.x )
                {
                    int64_t d = (int64_t)( ip.x - aP.x ) * (int64_t)( ipNext.y - aP.y ) -
                                (int64_t)( ipNext.x - aP.x ) * (int64_t)( ip.y - aP.y );

                if( !d )
                    return true;

                if( ( d > 0 ) == ( ipNext.y > ip.y ) )
                    result = 1 - result;
                }
            }
        }

        ip = ipNext;
    }

    return result ? true : false;
}


struct PointHash
{
    enum HashFlag {
        EMPTY = 1,
        FULL = 2,
        PARTIAL= 4,
        LEADING = 8,
        IN_USE = 16
    };

    struct Edge {
        Edge(int aIndex, int aIp)
        {
                index = aIndex;
                ip = aIp;
                flags = 0;
                leading = false;
        }

        bool leading;
        int ip;
        int seq;
        int flags;
        int index;
    };

    using EdgeList = std::vector<Edge> ;

    vector<EdgeList> m_grid;
    vector<EdgeList> h_edges;
    vector<EdgeList> v_edges;

    const EdgeList fillList ( int x, int y )
    {
            int px      = (int) ( (double) x / (double)m_gridSize * (double) m_bbox.GetWidth() + m_bbox.GetPosition().x );
            int py      = (int) ( (double) y / (double)m_gridSize * (double) m_bbox.GetHeight() + m_bbox.GetPosition().y );
            int px_next      = (int) ( (double) (x+1) / (double)m_gridSize * (double) m_bbox.GetWidth() + m_bbox.GetPosition().x );
            int py_next      = (int) ( (double) (y+1 )/ (double)m_gridSize * (double) m_bbox.GetHeight() + m_bbox.GetPosition().y );

            int k;
            auto CompareLo = [](  const Edge& e, int ip ) {
                return (e.ip < ip);
            };
            auto CompareHi = [](   int ip, const Edge& e ) {
                return (ip > e.ip);
            };

            auto first_h = std::lower_bound( h_edges[y].begin(), h_edges[y].end(), px, CompareLo );
            auto last_h = std::upper_bound( h_edges[y].begin(), h_edges[y].end(), px_next, CompareHi );
            auto first_v = std::lower_bound( v_edges[x].begin(), v_edges[x].end(), py, CompareLo );
            auto last_v = std::upper_bound( v_edges[x].begin(), v_edges[x].end(), py_next, CompareHi );

            EdgeList rv;

            set<int> usedEdges;

            for ( auto i = first_h ; i <= last_h; ++i )
            {
                    rv.push_back(*i);
                    usedEdges.insert(i->index);
            }
            for ( auto i = first_v ; i <= last_v; ++i )
            {
                if( usedEdges.find( i->index ) == usedEdges.end() )
                    rv.push_back(*i);
            }

            return rv;
    }

    PointHash( const SHAPE_LINE_CHAIN& aPolyOutline, int gridSize )
    {
        m_outline = aPolyOutline;
        m_bbox = m_outline.BBox();
        m_gridSize = gridSize;
    //    m_grid = new uint8_t [ gridSize * gridSize ];

//        int x0 = m_bbox.GetPosition().x - 1;
        //int x1 = m_bbox.GetPosition().x + m_bbox.GetWidth() + 1;

        m_outline.SetClosed(true);
        m_outline.Simplify();
        //printf("PC : %d\n", m_outline.PointCount());

        //if ( m_outline.SegmentCount() < 3)
        //return ;

        set<int> pendingEdges;
        vector<int> leading;

        for ( int i = 0; i < m_outline.PointCount(); i++)
        {
            pendingEdges.insert(i);
            leading.push_back(0);
        }

        printf("PC %d\n", pendingEdges.size());

        while( !pendingEdges.empty() )
        {
            auto iter = pendingEdges.begin();
            int n_erased = 0;
            vector<Edge> visited;
            printf("pending: %d\n", pendingEdges.size() );


            while ( n_erased == 0 )
            {
                int ref_index = *iter;
                iter++;
                visited.clear();

                int ref_y = ( m_outline.CSegment(ref_index).A.y + m_outline.CSegment(ref_index).B.y ) / 2;



                for (int i=0; i< m_outline.PointCount(); i++)
                {
                    const VECTOR2I& pi = m_outline.CPoint(i + 1);
                    const VECTOR2I& pj = m_outline.CPoint(i);


                    if (pi.y< ref_y && pj.y>=ref_y ||   pj.y< ref_y && pi.y>=ref_y)
                    {
                        double xp = (double) pi.x+(double)(ref_y-pi.y)/(double)(pj.y-pi.y)*(double)(pj.x-pi.x);


                        if (pendingEdges.find(i) != pendingEdges.end())
                        {
                            pendingEdges.erase(i);
                            n_erased++;
                        }

                        visited.push_back( Edge ( i, (int) xp ) );
                    }
                }

            }

            #if 0
            for (int j = 0; j < m_outline.SegmentCount(); j++)
            {

                const SEG& edge = m_outline.CSegment(j);

                int y_min = std::min(edge.A.y, edge.B.y);
                int y_max = std::max(edge.A.y, edge.B.y);

                /*if (y_min == y_max)
                {
                    pendingEdges.erase(j);
                    leading[j] = 0;
                    continue;
                }*/

                if ( ref_y < y_min )
                    continue;

                if( ref_y > y_max )
                    continue;

                if ( ref_y == edge.A.y && edge.B.y < ref_y ) // below the threshold, we don't consired it intersecting
                    continue;


            //    printf("s %d ref_y: %d min %d max %d\n", j, ref_y, y_min, y_max );

                OPT_VECTOR2I ip_v = edge.IntersectLines ( SEG( VECTOR2I( 0, ref_y), VECTOR2I(1, ref_y )) );

                if (pendingEdges.find(j) != pendingEdges.end())
                    pendingEdges.erase(j);

                visited.push_back( Edge ( j, ip_v->x ) );

            }
            #endif

            std::sort ( visited.begin(), visited.end(), [] ( const Edge& a, const Edge&b ) { return a.ip < b.ip; } );

            bool l = true;

            printf("vsize %d\n", visited.size());
            int prev = INT_MAX;
            bool prev_duplicate= false;

            for (int i = 0; i < visited.size(); i++)
            {

                leading[visited[i].index] = l ? 1 : 2;
                l = !l;
            }

        }

        FILE *f=fopen("log.log","wb");
        fprintf(f,"group crappy-polygon 0\n");

        for (int i = 0; i<m_outline.SegmentCount();i++)
        {
            const SEG& edge = m_outline.CSegment(i);

            fprintf(f,"item %d path-pre 0 0 0 0 0 line 0 0 linechain 2 0 %d %d %d %d\n", leading[i], edge.A.x, edge.A.y, edge.B.x, edge.B.y );


        }
        fprintf(f,"endgroup\n");
        fclose(f);

        for(int k = 0; k <= gridSize; k++)
        {

            int px      = (int) ( (double) k / gridSize * (double) m_bbox.GetWidth() + m_bbox.GetPosition().x );
            int py      = (int) ( (double) k / gridSize * (double) m_bbox.GetHeight() + m_bbox.GetPosition().y );

            v_edges.push_back ( EdgeList() );
            h_edges.push_back ( EdgeList() );

            SEG ref_v ( VECTOR2I(px, py), VECTOR2I(px, py + 1) );
            SEG ref_h ( VECTOR2I(px, py), VECTOR2I(px + 1, py) );



            for (int i = 0; i < m_outline.SegmentCount(); i++)
            {
                const SEG& edge = m_outline.CSegment(i);

                int y_min = std::min(edge.A.y, edge.B.y);
                int y_max = std::max(edge.A.y, edge.B.y);
                int x_min = std::min(edge.A.x, edge.B.x);
                int x_max = std::max(edge.A.x, edge.B.x);


                //printf("%d %d %d %d\n", edge.A.x, edge.A.y, edge.B.x, edge.B.y);

                if( py >= y_min && py <= y_max && !edge.Collinear( ref_h ))
                {
                    //printf("h %d %d %d %d\n", ref_h.A.x, ref_h.A.y, ref_h.B.x, ref_h.B.y);
                    OPT_VECTOR2I ip_h = edge.IntersectLines ( ref_h );
                    h_edges[k].push_back( Edge(i, ip_h->x) );
                }

                if( px >= x_min && px <= x_max )
                {
                    int yy;
                    if ( !edge.Collinear( ref_v ) )
                    {
                        OPT_VECTOR2I ip_v = edge.IntersectLines ( ref_v );
                        yy = ip_v->y;
                    } else
                        yy = y_min;
                    //printf("v %d %d %d %d\n", ref_v.A.x, ref_v.A.y, ref_v.B.x, ref_v.B.y);

                    v_edges[k].push_back( Edge(i, yy) );
                }
            }

            std::sort ( h_edges[k].begin(), h_edges[k].end(), [] ( const Edge& a, const Edge&b ) { return a.ip < b.ip; } );
            std::sort ( v_edges[k].begin(), v_edges[k].end(), [] ( const Edge& a, const Edge&b ) { return a.ip < b.ip; } );




            //printf("Bin %d: %d y's\n", k, y_vals.size());

            //std::sort ( v_edges[k].begin(), v_edges[k].end() );
        }

        return;

        for(int y = 0; y < gridSize; y++)
            for(int x = 0; x < gridSize; x++)
            {
                const EdgeList l = fillList ( x, y );

                m_grid.push_back(l);

                //printf("cell (%d,%d) edges: %d\n", x,y,l.size());

/*                int px      = (int) ( (double) x / gridSize * (double) m_bbox.GetWidth() + m_bbox.GetPosition().x );
                int px_next = (int) ( (double) (x+1) / gridSize * (double) m_bbox.GetWidth() + m_bbox.GetPosition().x );
                int py      = (int) ( (double) y / gridSize * (double) m_bbox.GetHeight() + m_bbox.GetPosition().y );
                int py_next = (int) ( (double) (y+1) / gridSize * (double) m_bbox.GetHeight() + m_bbox.GetPosition().y );

                //printf("try %d %d\n", px, py);

                const VECTOR2I p0 ( px, py );
                const VECTOR2I p1 ( px_next, py );
                const VECTOR2I p2 ( px_next, py_next );
                const VECTOR2I p3 ( px, py_next );

                //const SEG s0 ( p0, p1 );
                //const SEG s1 ( p1, p2 );
                //const SEG s2 ( p2, p3 );
                //const SEG s3 ( p3, p0 );

                bool inside = pointInPolygon( p0, m_outline );

                //printf("in : %d\n", !!inside);

                bool hit = false;
                for (int i = 0; i < m_outline.SegmentCount(); i++)
                {
                    const SEG& edge = m_outline.CSegment(i);

                    int x_min = std::min(edge.A.x, edge.B.x);
                    int x_max = std::max(edge.A.x, edge.B.x);

                    int y_min = std::min(edge.A.y, edge.B.y);
                    int y_max = std::max(edge.A.y, edge.B.y);

                    int s0 = edge.Side( p0 );
                    int s1 = edge.Side( p1 );

                    // s0/s1 : horizontal

                    if( s0 != s1 && p0.y >= y_min && p0.y <= y_max ) { hit = true; break; }

                    int s2 = edge.Side( p2 );

                    // s1/s2: vertical
                    if( s1 != s2 && p1.x >= x_min && p1.x <= x_max ) { hit = true; break; }

                    int s3 = edge.Side( p3 );

                    if( s2 != s3 && p2.y >= y_min && p2.y <= y_max ) { hit = true; break; }

                    if( s0 != s3 && p0.x >= x_min && p0.x <= y_max ) { hit = true; break; }



                //    if ( edge.Intersect( s0 ) || edge.Intersect(s1) || edge.Intersect(s2) || edge.Intersect(s3) )
                //    {
                //        hit = true;
                //        break;
                //    }
                }

                int index = gridSize * y + x;
                if ( !hit && inside )
                    m_grid[index] = GRID_FULL;
                else if ( !hit && !inside )
                    m_grid[index] = GRID_EMPTY;
                else
                    m_grid[index] = GRID_PARTIAL;
*/



            }
#if 0

            int full = 0, empty = 0, partial = 0;

            for(int i =0 ; i < m_gridSize * m_gridSize ; i++)
            {
                uint8_t q = m_grid[i];
                switch(q)
                {
                    case GRID_FULL: full++; break;
                    case GRID_PARTIAL: partial++; break;
                    case GRID_EMPTY:empty++; break;
                }
            }

            printf(" full : %d empty : %d partial : %d\n", full, empty, partial );
#endif

    }

    bool ContainsPoint ( const VECTOR2I& aP ) const
    {

    }

private:
    //uint8_t *m_grid;
    int m_gridSize;
    SHAPE_LINE_CHAIN m_outline;
    BOX2I m_bbox;

};
#endif

#if 0

struct Poly
{
    struct Edge
    {
        int y0;
        int y1;
        int index;
    };

    struct Bin
    {
        int y0;
        int y1;

        vector<int> edges;

        bool operator== ( const Bin& b) const { return b.y0 == y0; }
        bool operator< ( const Bin& b) const { return b.y0 < y0; }
    };

    set<Bin> m_bins;

    Poly( const SHAPE_LINE_CHAIN& outline ) :
        m_poly ( outline )
    {
        set<Bin> bins;
        vector<Edge> edges;

        edges.reserve( outline.PointCount() );

        for (int i = 0; i <outline.PointCount(); i++)
        {
            const auto &p = outline.CPoint(i);
            const auto &p_next = outline.CPoint(i+1);
            Bin b;

            b.y0 = std::min( p.y, p_next.y );
            b.y1 = std::max( p.y, p_next.y );
            m_bins.insert(b);

            Edge e;

            b.y0 = std::min( p.y, p_next.y );
            b.y1 = std::max( p.y, p_next.y );
            e.index = i;
            edges.push_back(e);
        }
        printf("bins : %d edges : %d\n", m_bins.size(), edges.size() );

        //std::sort(edges.begin(), edges.end(), [](const Edge& a, const Edge& b) { return a.y0 < b.y0; });
        int e_total=  0;

        for ( auto& bin : m_bins )
        {
            vector<Edge> etmp;
            etmp.clear();

            for ( const auto &e: edges)
            {
                if ( e.y0 >= bin.y0 && e.y0 <= bin.y1 || (e.y0 <= bin.y0 && e.y1 >= bin.y1) || ( e.y0 <= bin.y0 && e.y1 <= bin.y1 ) )
                {
                    etmp.push_back(e);
                }

                std::sort(etmp.begin(), etmp.end(), [&](const Edge& a, const Edge& b) {
                        return m_poly.CPoint(a.index).x < m_poly.CPoint(b.index).x;
                    ; });

            }

            e_total += etmp.size();

        }

        printf("Total edges : %d\n", e_total);
    }

    const SHAPE_LINE_CHAIN& m_poly;

};
#endif
shared_ptr<BOARD> m_board;


void loadBoard( string name )
{
    PLUGIN::RELEASER pi( IO_MGR::PluginFind( IO_MGR::KICAD ) );

    try
    {
        m_board.reset( pi->Load( name, NULL, NULL ) );
    }
    catch( const IO_ERROR& ioe )
    {
        wxString msg = wxString::Format( _( "Error loading board.\n%s" ),
                ioe.What() );

        fprintf( stderr, "%s\n", (const char*) msg.mb_str() );
        exit( -1 );
    }
}

#include <valgrind/callgrind.h>

int main( int argc, char* argv[] ) {
    if( argc < 2 )
    {
        printf( "usage : %s board_file\n", argv[0] );
        return 0;
    }

    loadBoard( argv[1] );

    for( int i = 0; i <m_board->GetAreaCount(); i++)
    {
            ZONE_CONTAINER *zone =  m_board->GetArea(i);
            const SHAPE_POLY_SET& polys = zone->GetFilledPolysList();
            for(int o = 0; o < polys.OutlineCount(); o++)
            {
  //CALLGRIND_START_INSTRUMENTATION;
                if( polys.COutline(o).PointCount() > 50 )
                {
                    PointHash p (polys.COutline(o), 32);
                    return 0;
                }
    //            CALLGRIND_STOP_INSTRUMENTATION;
  //CALLGRIND_DUMP_STATS;
            }

    }

}
