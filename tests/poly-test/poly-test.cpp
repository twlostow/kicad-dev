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
#include <unordered_set>

#include <stdarg.h>

using namespace std;

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

struct POLY_GRID_PARTITION
{
    enum HashFlag {
        LEAD_H = 1,
        LEAD_V = 2,
        TRAIL_H = 4,
        TRAIL_V = 8
    };

    using EdgeList = std::vector<int> ;



    template <class T>
    inline void hash_combine(std::size_t& seed, const T& v)
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    }

    struct segsEqual
    {
        bool operator()( const SEG& a, const SEG& b ) const
        {
            return (a.A == b.A && a.B == b.B) || (a.A == b.B && a.B == b.A);
        }
    };

    struct segHash
    {
        std::size_t operator()(  const SEG&a ) const
        {
        std::size_t seed = 0;

        return a.A.x + a.B.x + a.A.y + a.B.y;

        return seed;
        }
    };

    const VECTOR2I grid2poly( const VECTOR2I &p ) const
    {
        int px      = rescale ( p.x, m_bbox.GetWidth(), m_gridSize) +  m_bbox.GetPosition().x;
        int py      = rescale ( p.y, m_bbox.GetHeight(), m_gridSize) +  m_bbox.GetPosition().y;//(int) floor( (double) p.y / m_gridSize * (double) m_bbox.GetHeight() + m_bbox.GetPosition().y );

        return VECTOR2I(px, py);
    }

    int grid2polyX( int x ) const
    {
        return rescale ( x, m_bbox.GetWidth(), m_gridSize) +  m_bbox.GetPosition().x;
    }

    int grid2polyY( int y ) const
    {
        return rescale ( y, m_bbox.GetHeight(), m_gridSize) +  m_bbox.GetPosition().y;
    }

    const VECTOR2I poly2grid( const VECTOR2I &p ) const
    {
        int px      = rescale ( p.x - m_bbox.GetPosition().x, m_gridSize, m_bbox.GetWidth());
        int py      = rescale ( p.y - m_bbox.GetPosition().y, m_gridSize, m_bbox.GetHeight());

        //int px      =  (int) floor( (double)(p.x - m_bbox.GetPosition().x) * m_gridSize / (double) m_bbox.GetWidth() );
        //int py      =  (int) floor( (double)(p.y - m_bbox.GetPosition().y) * m_gridSize / (double) m_bbox.GetHeight() );

        if ( px < 0 ) px = 0;
        if ( px >= m_gridSize ) px = m_gridSize - 1;
        if ( py < 0 ) py = 0;
        if ( py >= m_gridSize ) py = m_gridSize - 1;

        return VECTOR2I(px, py);
    }

    int poly2gridX( int x ) const
    {
        int px      = rescale ( x - m_bbox.GetPosition().x, m_gridSize, m_bbox.GetWidth());

        if ( px < 0 ) px = 0;
        if ( px >= m_gridSize ) px = m_gridSize - 1;

        return px;
    }

    int poly2gridY( int y ) const
    {
        int py      = rescale ( y - m_bbox.GetPosition().y, m_gridSize, m_bbox.GetHeight());

        if ( py < 0 ) py = 0;
        if ( py >= m_gridSize ) py = m_gridSize - 1;

        return py;
    }


    POLY_GRID_PARTITION( const SHAPE_LINE_CHAIN& aPolyOutline, int gridSize )
    {
        m_outline = aPolyOutline;
        m_bbox = m_outline.BBox();
        m_gridSize = gridSize;

        m_outline.SetClosed(true);

        m_grid.reserve( gridSize * gridSize );

        for(int y = 0; y < gridSize; y++)
            for(int x = 0; x < gridSize; x++)
            {
                m_grid.push_back( EdgeList() );
            }


        VECTOR2I ref_v ( 0, 1 );
        VECTOR2I ref_h ( 0, 1 );

        m_flags.reserve( m_outline.SegmentCount() );

        std::unordered_map<SEG, int, segHash, segsEqual> edgeSet;

        for (int i = 0; i<m_outline.SegmentCount(); i++)
        {
            SEG edge = m_outline.CSegment(i);

            if ( edgeSet.find(edge) == edgeSet.end() )
                edgeSet[edge] = 1;
            else
                edgeSet[edge]++;
        }

        for (int i = 0; i<m_outline.SegmentCount(); i++)
        {
            auto edge = m_outline.CSegment(i);
            auto dir = edge.B - edge.A;
            int flags = 0;

            if (edgeSet[edge] == 1)
            {
                if (dir.Dot(ref_h) > 0)
                    flags |= LEAD_H;
                else if (dir.Dot(ref_h) < 0)
                    flags |= TRAIL_H;
            }

            m_flags.push_back( flags );

            if( !flags )
                continue;

            std::set<int> indices;

            indices.insert ( m_gridSize * poly2gridY(edge.A.y) + poly2gridX(edge.A.x));
            indices.insert ( m_gridSize * poly2gridY(edge.B.y) + poly2gridX(edge.B.x));

            if(edge.A.x > edge.B.x)
                std::swap(edge.A, edge.B);

            dir = edge.B - edge.A;

            if(dir.x != 0)
            {
                int gx0 = poly2gridX( edge.A.x ) + 1;
                int gx1 = poly2gridX( edge.B.x );

                for( int x = gx0 ; x <= gx1; x++ )
                {
                    int px = grid2polyX ( x );
                    int py =(  edge.A.y + rescale( dir.y , px - edge.A.x , dir.x ) );
                    int yy = poly2gridY( py );

                    indices.insert ( m_gridSize * yy + x );
                    if ( x > 0 )
                        indices.insert ( m_gridSize * yy + x - 1 );
                }
            }

            if(edge.A.y > edge.B.y)
                std::swap(edge.A, edge.B);

            dir = edge.B - edge.A;

            if(dir.y != 0)
            {
                int gy0 = poly2gridY( edge.A.y ) + 1;
                int gy1 = poly2gridY( edge.B.y ) ;
                for( int y = gy0 ; y <= gy1; y++ )
                {
                    int py = grid2polyY ( y );
                    int px = (  edge.A.x + rescale( dir.x , py - edge.A.y , dir.y ) );
                    int xx = poly2gridX( px );

                    indices.insert ( m_gridSize * y + xx );
                    if ( y > 0 )
                        indices.insert ( m_gridSize * (y - 1) + xx );
                }
            }

            for(auto idx:indices)
                m_grid[idx].push_back( i );
        }
    }

    bool inRange( int v1, int v2, int x ) const
    {
        if (v1 < v2)
        {
            return ( x >= v1 && x <= v2);
        }

        return (x >= v2 && x <= v1);
    }

    struct ScanState
    {
        ScanState()
        {
            dist_max = INT_MAX;
            nearest = -1;
            nearest_prev = -1;
        };

        int dist_prev;
        int dist_max;
        int nearest_prev;
        int nearest;
    };

   void scanCell ( ScanState& state, const EdgeList& cell, const VECTOR2I& aP ) const
    {
        for ( auto index : cell )
        {
            const SEG& edge = m_outline.CSegment(index);

            if ( edge.A.y == edge.B.y )  // horizontal edge
                    continue;

            if ( m_flags[index] == 0) // a slit
                continue;

            if ( inRange ( edge.A.y, edge.B.y, aP.y ) )
            {
                    int dist = 0;

                    if( edge.A.y == aP.y )
                    {
                        dist = - (aP.x - edge.A.x);
                    } else if (edge.B.y == aP.y)
                    {
                        dist = - (aP.x - edge.B.x);
                    } else {
                        const VECTOR2I  e( edge.B - edge.A );
                        const VECTOR2I  ff( 1 , 0 );
                        const VECTOR2I  ac( aP - edge.A );

                        auto d = ff.Cross( e );
                        auto q = e.Cross( ac );

                        using ecoord = VECTOR2I::extended_type;

                        dist = rescale( q, (ecoord) 1, d );
                    }
                    //printf("dist %d\n", dist);

                    if (dist == 0)
                    {
                        if( state.nearest_prev < 0 || state.nearest != index)
                        {
                            state.dist_prev = state.dist_max;
                            state.nearest_prev = state.nearest;
                        }
                        state.nearest = index;
                        state.dist_max = 0;
                        return;
                    }

                    if( dist != 0 && std::abs(dist) <= std::abs(state.dist_max) )
                    {
                        if( state.nearest_prev < 0 || state.nearest != index)
                        {
                            state.dist_prev = state.dist_max;
                            state.nearest_prev = state.nearest;
                        }
                        //printf("index %d dist %d\n", index, dist);
                        state.nearest = index;
                        state.dist_max = dist;
                    }
                }
            }
        }

        SEG nearestEdge;

    int ContainsPoint ( const VECTOR2I& aP )// const
    {
        const auto gridPoint = poly2grid (aP);

        if (!m_bbox.Contains(aP))
            return false;

        ScanState state;
        const EdgeList& cell = m_grid[ m_gridSize * gridPoint.y + gridPoint.x ];

        scanCell ( state, cell, aP );

        if ( state.nearest < 0)
        {
            state = ScanState();
            for ( int d = 1; d <= m_gridSize; d++)
            {
                int xl = gridPoint.x - d;
                int xh = gridPoint.x + d;

                if( xl >= 0 )
                {
                    const EdgeList& cell = m_grid[ m_gridSize * gridPoint.y + xl ];
                    scanCell ( state, cell, aP );
                    if( state.nearest >= 0)
                        break;
                }
                if( xh < m_gridSize )
                {
                    const EdgeList& cell = m_grid[ m_gridSize * gridPoint.y + xh ];
                    scanCell ( state, cell, aP );
                    if( state.nearest >= 0)
                        break;
                }
            }
        }

//        printf("nearest %d %d dist_max %d dist_prev %d\n", state.nearest,state.nearest_prev, state.dist_max, state.dist_prev);

        if ( state.nearest < 0)
            return 0;

        if( state.dist_max == 0)
            return 1;

        if (state.nearest_prev >= 0 && state.dist_max == state.dist_prev)
        {
            int d = std::abs(state.nearest_prev - state.nearest);
            if ( d == 1 ) // corner
            {
                //printf("n %d np %d %d\n", m_flags[state.nearest_prev] , m_flags[state.nearest], m_flags[state.nearest_prev] & m_flags[state.nearest] == 0 );
                if ( (m_flags[state.nearest_prev] & m_flags[state.nearest]) == 0)
                    return 0;
            } else if ( d>1) {
                return 1;
            }
        }

        if ( state.dist_max > 0 )
        {
            return m_flags[state.nearest] & LEAD_H ? 1 : 0 ;
        } else {
            return m_flags[state.nearest] & TRAIL_H ? 1 : 0 ;
        }
    }

//private:
    int m_gridSize;
    SHAPE_LINE_CHAIN m_outline;
    BOX2I m_bbox;
    vector<int> m_flags;
    vector<EdgeList> m_grid;
};

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

int k = 0;

int benchmark(int np,  SHAPE_LINE_CHAIN lc  )
{
    int n = 0;
    lc.Simplify();

    POLY_GRID_PARTITION p ( lc, 16 );
    BOX2I bbox = lc.BBox();

    vector<int> hits, onEdge;

    for(int y = 0; y < np; y++)
        for(int x = 0; x < np; x++)
        {
            hits.push_back(0);
            onEdge.push_back(0);
        }

        lc.SetClosed(true);

    PROF_COUNTER cnt_old("old-pip"); cnt_old.start();

    n = 0;
    for(int y = 0; y < np; y++)
        for(int x = 0; x < np; x++)
        {
            int px      = (int) ( (double) x / np * (double) bbox.GetWidth() + bbox.GetPosition().x );
            int py      = (int) ( (double) y / np * (double) bbox.GetHeight() + bbox.GetPosition().y );
            hits[n] = pointInPolygon (VECTOR2I(px,py), lc);

            n++;
        }
    cnt_old.stop();

    n = 0;
    for(int y = 0; y < np; y++)
        for(int x = 0; x < np; x++)
        {
            int px      = (int) ( (double) x / np * (double) bbox.GetWidth() + bbox.GetPosition().x );
            int py      = (int) ( (double) y / np * (double) bbox.GetHeight() + bbox.GetPosition().y );

            int dd = lc.Distance( VECTOR2I(px, py  ));

            onEdge[n] = (dd <= 1);

            n++;
        }

    PROF_COUNTER cnt_new("new-pip"); cnt_new.start();

    n = 0;
    bool fail = false;
    for(int y = 0; y < np; y++)
        for(int x = 0; x < np; x++)
        {

            int px      = (int) ( (double) x / np * (double) bbox.GetWidth() + bbox.GetPosition().x );
            int py      = (int) ( (double) y / np * (double) bbox.GetHeight() + bbox.GetPosition().y );

            int hit = p.ContainsPoint ( VECTOR2I(px, py ));

            if(hit != hits[n])
            {

                if ( hit && !onEdge[n] )
                {
                    printf("Failure for (%d, %d) old : %d new : %d\n", px, py, hits[n], hit);
                    fail = true;
                    y=np;
                    break;
                }
            }

            n++;
        }
    cnt_new.stop();

    printf("%d vertices, old algo = %.1f ms, new algo = %.f ms\n", lc.PointCount(), cnt_old.msecs(), cnt_new.msecs());

    return 0;
}

int main( int argc, char* argv[] ) {
    if( argc < 2 )
    {
        printf( "usage : %s board_file\n", argv[0] );
        return 0;
    }

    loadBoard( argv[1] );

#if 1
    if (argc == 6)
    {


        VECTOR2I tp (atoi(argv[2]), atoi(argv[3]));
        ZONE_CONTAINER *zone =  m_board->GetArea(atoi(argv[4]));
        const SHAPE_POLY_SET& polys = zone->GetFilledPolysList();
        SHAPE_LINE_CHAIN lc = polys.COutline(atoi(argv[5]));
        lc.Simplify();

        POLY_GRID_PARTITION p ( lc, 16 );

        FILE *f = fopen("reportX.log","wb");

        fprintf(f,"group crappy-polygon 0\n");


        for(int i = 0; i < lc.SegmentCount(); i++)
        {
            const SEG& edge = lc.CSegment(i);
            fprintf(f,"item %d path-pre 0 0 0 0 0 line 0 0 linechain 2 0 %d %d %d %d\n",  p.m_flags[i], edge.A.x, edge.A.y, edge.B.x, edge.B.y );
        }

        for(int i = 0; i<p.m_gridSize; i++)
        {
            auto v = p.grid2poly( VECTOR2I( i, i ) );
            fprintf(f,"item 3 path-pre 0 0 0 0 0 line 0 0 linechain 2 0 %d %d %d %d\n", v.x, lc.BBox().GetPosition().y, v.x, lc.BBox().GetPosition().y + lc.BBox().GetHeight() );
            fprintf(f,"item 3 path-pre 0 0 0 0 0 line 0 0 linechain 2 0 %d %d %d %d\n",  lc.BBox().GetPosition().x, v.y, lc.BBox().GetPosition().x + lc.BBox().GetWidth(), v.y );
        }

        fprintf(f,"item %d path-pre 0 0 0 0 0 line 0 0 linechain 2 0 %d %d %d %d\n",2, tp.x-1000000,tp.y-1000000,tp.x+1000000,tp.y+1000000 );
        fprintf(f,"item %d path-pre 0 0 0 0 0 line 0 0 linechain 2 0 %d %d %d %d\n",2, tp.x+1000000,tp.y-1000000,tp.x-1000000,tp.y+1000000 );
        fprintf(f,"endgroup\n");


        fprintf(f,"group crappy-polygon 0\n");

        VECTOR2I g= p.poly2grid(tp);
        for ( auto e : p.m_grid[p.m_gridSize * g.y + g.x] )
        {
            const SEG& edge = p.m_outline.CSegment(e);
            fprintf(f,"item %d path-pre 0 0 0 0 0 line 0 0 linechain 2 0 %d %d %d %d\n", p.m_flags[e], edge.A.x, edge.A.y, edge.B.x, edge.B.y );
        }
        fprintf(f,"item %d path-pre 0 0 0 0 0 line 0 0 linechain 2 0 %d %d %d %d\n",2, tp.x-1000000,tp.y-1000000,tp.x+1000000,tp.y+1000000 );
        fprintf(f,"item %d path-pre 0 0 0 0 0 line 0 0 linechain 2 0 %d %d %d %d\n",2, tp.x+1000000,tp.y-1000000,tp.x-1000000,tp.y+1000000 );

        fprintf(f,"endgroup\n");
        fclose(f);


        printf("Testing (%d, %d)\n", tp.x, tp.y);
        bool old_hit =  pointInPolygon(tp, lc);
        bool new_hit = p.ContainsPoint(tp);
        printf("old : %d\n", !!old_hit);
        printf("new : %d\n", !!new_hit);


        return 0;
    }
#endif

    for( int i = 0; i <m_board->GetAreaCount(); i++)
    {
            ZONE_CONTAINER *zone =  m_board->GetArea(i);
            const SHAPE_POLY_SET& polys = zone->GetFilledPolysList();



            for(int o = 0; o < polys.OutlineCount(); o++)
            {
                printf("zone %d outline %d\n", i, o);
                benchmark( 200, polys.COutline(o) );
                //return 0;
            }


    }

}
