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

    vector<int> m_flags;
    vector<EdgeList> m_grid;

    const VECTOR2I grid2poly( const VECTOR2I &p ) const
    {
        int px      = (int) ( (double) p.x / m_gridSize * (double) m_bbox.GetWidth() + m_bbox.GetPosition().x );
        int py      = (int) ( (double) p.y / m_gridSize * (double) m_bbox.GetHeight() + m_bbox.GetPosition().y );

        return VECTOR2I(px, py);
    }

    const VECTOR2I poly2grid( const VECTOR2I &p ) const
    {
        int px      =  (int) ( (double)(p.x - m_bbox.GetPosition().x) * m_gridSize / (double) m_bbox.GetWidth() );
        int py      =  (int) ( (double)(p.y - m_bbox.GetPosition().y) * m_gridSize / (double) m_bbox.GetHeight() );

        if ( px < 0 ) px = 0;
        if ( px >= m_gridSize ) px = m_gridSize - 1;
        if ( py < 0 ) py = 0;
        if ( py >= m_gridSize ) py = m_gridSize - 1;

        return VECTOR2I(px, py);
    }

    FILE *f;
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

        for (int i = 0; i<m_outline.SegmentCount(); i++)
        {
            const auto& edge = m_outline.CSegment(i);
            const auto dir = edge.B - edge.A;

            int flags = 0;

            if (dir.Dot(ref_h) > 0)
                flags |= LEAD_H;
            else if (dir.Dot(ref_h) < 0)
                flags |= TRAIL_H;

            /*if (dir.Dot(ref_v) > 0)
                flags |= LEAD_V;
            else if (dir.Dot(ref_v) < 0)
                flags |= TRAIL_V;*/

            m_flags.push_back( flags );

            double l = edge.Length();
            double delta = (double) std::max( m_bbox.GetWidth(), m_bbox.GetHeight() ) / (double) m_gridSize;
            int steps = (int)(2.0 * l / delta);

            //printf("edge %d : %d steps\n", i, steps);

            double dx = (double)dir.x / steps;
            double dy = (double)dir.y / steps;

            VECTOR2I p_prev;

            auto p = poly2grid ( edge.A );
            m_grid[ m_gridSize * p.y + p.x ].push_back( i );
            p = poly2grid ( edge.B );
            m_grid[ m_gridSize * p.y + p.x ].push_back( i );

            for(int j = 0; j < steps; j++ )
            {
                auto p = poly2grid ( VECTOR2I ( edge.A.x + (double) j * dx, edge.A.y + (double) j * dy ) );

                if( j > 0 && p != p_prev )
                {
                    //printf("scan (%d, %d)\n", p.x, p.y);

                    m_grid[ m_gridSize * p.y + p.x ].push_back( i );
                }
            }
        }

        f=fopen("log.log","wb");
        fprintf(f,"group crappy-polygon 0\n");


        for(int i = 0; i < m_outline.SegmentCount(); i++)
        {
            const SEG& edge = m_outline.CSegment(i);
            fprintf(f,"item %d path-pre 0 0 0 0 0 line 0 0 linechain 2 0 %d %d %d %d\n", m_flags[i], edge.A.x, edge.A.y, edge.B.x, edge.B.y );
        }

        int np = 200;
        for(int y = 0; y < np; y++)
            for(int x = 0; x < np; x++)
            {

                int px      = (int) ( (double) x / np * (double) m_bbox.GetWidth() + m_bbox.GetPosition().x );
                int py      = (int) ( (double) y / np * (double) m_bbox.GetHeight() + m_bbox.GetPosition().y );
                int c= ContainsPoint ( VECTOR2I(px, py ));

                fprintf(f,"item %d path-pre 0 0 0 0 0 line 0 0 linechain 2 0 %d %d %d %d\n",c, px,py,px,py );

            }

        fprintf(f,"endgroup\n");

        for(int y = 0; y < gridSize; y++)
            for(int x = 0; x < gridSize; x++)
            {

                int px      = (int) ( (double) x / gridSize * (double) m_bbox.GetWidth() + m_bbox.GetPosition().x );
                int px_next = (int) ( (double) (x+1) / gridSize * (double) m_bbox.GetWidth() + m_bbox.GetPosition().x );
                int py      = (int) ( (double) y / gridSize * (double) m_bbox.GetHeight() + m_bbox.GetPosition().y );
                int py_next = (int) ( (double) (y+1) / gridSize * (double) m_bbox.GetHeight() + m_bbox.GetPosition().y );

                fprintf(f,"group crappy-polygon 0\n");
                fprintf(f,"item 0 cell 0 0 0 0 0 line 0 0 linechain 4 1 %d %d %d %d %d %d %d %d\n",px, py, px_next, py, px_next, py_next, px, py_next);

                for ( auto e : m_grid[m_gridSize * y + x] )
                {

                    const SEG& edge = m_outline.CSegment(e);

                    fprintf(f,"item %d path-pre 0 0 0 0 0 line 0 0 linechain 2 0 %d %d %d %d\n", m_flags[e], edge.A.x, edge.A.y, edge.B.x, edge.B.y );


                }

                fprintf(f,"endgroup\n");

            }



            fclose(f);
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
        };

        int dist_max;
        int nearest;
    };

    template<class swapCoordsType>
        void scanCell ( ScanState& state, const EdgeList& cell, const VECTOR2I& aP, int flagMask ) const
        {
            swapCoordsType swp;

            for ( auto index : cell )
            {
                const SEG& edge = m_outline.CSegment(index);

                if ( swp(edge.A).y == swp(edge.B).y )  // horizontal edge
                    continue;

                if ( ( m_flags[index] & flagMask ) == 0)
                    continue;

                if ( inRange ( swp(edge.A).y, swp(edge.B).y, swp(aP).y ) )
                {
                    SEG t ( VECTOR2I( aP.x, aP.y ), VECTOR2I( aP.x + 1, aP.y )  );

                    const VECTOR2I  e( edge.B - edge.A );
                    const VECTOR2I  ff( swp ( VECTOR2I( 1 , 0 ) ) );
                    const VECTOR2I  ac( aP - edge.A );

                    auto d = ff.Cross( e );
                    auto p = ff.Cross( ac );
                    auto q = e.Cross( ac );

                    using ecoord = VECTOR2I::extended_type;

                    auto dist = rescale( q, (ecoord) 1, d );
                    //printf("dist %d\n", dist);

                    if (dist == 0)
                    {
                        state.nearest = index;
                        state.dist_max = 0;
                        return;
                    }

                    if( dist != 0 && std::abs(dist) < std::abs(state.dist_max) )
                    {
                        state.dist_max = dist;
                        state.nearest = index;
                    }
                }
            }
        }

    int ContainsPoint ( const VECTOR2I& aP ) const
    {
        const auto gridPoint = poly2grid (aP);

        struct directCoords
        {
             const VECTOR2I operator() ( const VECTOR2I &p ) { return p; };
        };
        struct reverseCoords
        {
             const VECTOR2I operator() ( const VECTOR2I &p ) { return VECTOR2I(p.y, p.x); };
        };

        if (!m_bbox.Contains(aP))
            return false;

            int flag = 0;
            int dist_max = INT_MAX;
            int nearest = -1;


            ScanState state;

            const EdgeList& cell = m_grid[ m_gridSize * gridPoint.y + gridPoint.x ];

            scanCell<directCoords> ( state, cell, aP, LEAD_H | TRAIL_H );

            if ( state.nearest < 0)
            {
                state =  ScanState();
                scanCell<reverseCoords> ( state, cell, aP, LEAD_V | TRAIL_V );
            }

            if ( state.nearest < 0)
            {
                state = ScanState();
                for ( int d = 1; d < m_gridSize; d++)
                {
                    int xl = gridPoint.x - d;
                    int xh = gridPoint.x + d;

                    if( xl >= 0 )
                    {
                        const EdgeList& cell = m_grid[ m_gridSize * gridPoint.y + xl ];
                        scanCell<directCoords> ( state, cell, aP, LEAD_H | TRAIL_H );
                        if( state.nearest >= 0)
                            break;
                    }
                    if( xh < m_gridSize )
                    {
                        const EdgeList& cell = m_grid[ m_gridSize * gridPoint.y + xh ];
                        scanCell<directCoords> ( state, cell, aP, LEAD_H | TRAIL_H );
                        if( state.nearest >= 0)
                            break;
                    }
                }
            }

            /*for ( auto index : cell )
            {
                const SEG& edge = m_outline.CSegment(index);

                if ( edge.A.y == edge.B.y )  // horizontal edge
                    continue;

                if ( ( m_flags[index] & (LEAD_H | TRAIL_H) ) == 0)
                    continue;

                if ( inRange ( edge.A.y, edge.B.y, aP.y ) )
                {
                    SEG t ( VECTOR2I( aP.x, aP.y ), VECTOR2I( aP.x + 1, aP.y )  );

                    const VECTOR2I  e( edge.B - edge.A );
                    const VECTOR2I  ff( t.B - t.A );
                    const VECTOR2I  ac( t.A - edge.A );


                    auto d = ff.Cross( e );
                    auto p = ff.Cross( ac );
                    auto q = e.Cross( ac );

                    using ecoord = VECTOR2I::extended_type;

                    auto ip_x = t.A.x + rescale( q, (ecoord) ff.x, d );

                    auto dist = aP.x - ip_x;

                    if (dist == 0)
                        return 1;

                    //fprintf(f,"item %d path-pre 0 0 0 0 0 line 0 0 linechain 2 0 %d %d %d %d\n", 3, ip_x, aP.y, ip_x + 10000, aP.y + 10000 );

                    //printf("ip_x %d\n", ip_x);


                    if( dist != 0 && std::abs(dist) < std::abs(dist_max) )
                    {
                        dist_max = dist;
                        nearest = index;
                    }
                }
            }*/


            //}
#if 0
            if ( nearest < 0 )
            {
                for ( auto index : cell )
                {
                    const SEG& edge = m_outline.CSegment(index);

                    if ( edge.A.x == edge.B.x )  // horizontal edge
                        continue;

                    if ( m_flags[index] & (LEAD_V | TRAIL_V) == 0)
                        continue;

                    if ( inRange ( edge.A.y, edge.B.y, aP.y ) )
                    {
                        const VECTOR2I  e( edge.B - edge.A );
                        const VECTOR2I  f( 0, 1 );
                        const VECTOR2I  ac( aP - edge.A );

                        auto d = f.Cross( e );
                        auto p = f.Cross( ac );
                        auto q = e.Cross( ac );

                        using ecoord = VECTOR2I::extended_type;

                        auto ip_y = aP.y + rescale( q, (ecoord) 1, d );

                        auto dist = aP.y - ip_y;

                        if( dist >= 0 && std::abs(dist) < std::abs(dist_max) )
                        {
                            dist_max = dist;
                            nearest = index;
                        }
                    }
                }

                if (nearest < 0)
                    return 0;

                    if( dist_max == 0)
                        return 1;

                    if ( dist_max >= 0 )
                    {
                        return m_flags[nearest] & LEAD_H ? 1 : 2 ;
                    } else {
                        return m_flags[nearest] & TRAIL_H ? 1 : 2 ;
                    }
            }
            #endif

            if ( state.nearest < 0)
                return 0;

            if( state.dist_max == 0)
                return 1;

            if ( state.dist_max > 0 )
            {
                return m_flags[state.nearest] & LEAD_H ? 1 : 0 ;
            } else {
                return m_flags[state.nearest] & TRAIL_H ? 1 : 0 ;
            }




    }

private:
    //uint8_t *m_grid;
    int m_gridSize;
    SHAPE_LINE_CHAIN m_outline;
    BOX2I m_bbox;

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
                if(  polys.COutline(o).PointCount() > 100 )
                {
                    PROF_COUNTER cnt ("build-poly"); cnt.start();
                    POLY_GRID_PARTITION p (polys.COutline(o), 8 );
                    cnt.show();
                    return 0;

                //    return 0;
                }
            }


    }

}
