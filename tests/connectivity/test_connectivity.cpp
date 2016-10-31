#include <class_board.h>
#include <class_zone.h>
#include <io_mgr.h>

#include "connectivity.h"
#include "ratsnest_v2.h"

#include <profile.h>

using namespace std;

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


void saveBoard( shared_ptr<BOARD> board, string name )
{
    PLUGIN::RELEASER pi( IO_MGR::PluginFind( IO_MGR::KICAD ) );

    try
    {
        pi->Save( name, board.get(), NULL );
    }
    catch( const IO_ERROR& ioe )
    {
        wxString msg = wxString::Format( _( "Error saving board.\n%s" ),
                ioe.What() );

        fprintf( stderr, "%s\n", (const char*) msg.mb_str() );
        exit( -1 );
    }
}


int main( int argc, char* argv[] )
{
    if( argc < 2 )
    {
        printf( "usage : %s board_file\n", argv[0] );
        return 0;
    }

    loadBoard( argv[1] );

    CONNECTIVITY_DATA conns(m_board.get());

    conns.ProcessBoard();

    auto cnAlgo = conns.GetConnectivityAlgo();
    PROF_COUNTER cnt_cl("build-clusters",true);
    auto clusters = cnAlgo->GetClusters();
    cnt_cl.show();

    printf("%d clusters\n", clusters.size());

    std::vector<RN::RN_NET> rnNets;
    rnNets.resize ( cnAlgo->NetCount() );

    for ( auto& c : clusters )
    {
        if ( c->OriginNet() > 0)
        {
            rnNets[c->OriginNet()].AddCluster ( c );
        }
    }

    PROF_COUNTER cnt("build-ratsnest\n",true);
    for ( auto& net : rnNets )
    {
        net.Update();
    }
    cnt.show();



    for ( auto& net : rnNets )
    {
        for ( auto edge : *net.GetUnconnected() )
        {
            auto src = edge->GetSourceNode();
            auto dst = edge->GetTargetNode();

            printf("Unconnected edge: (%d, %d) -> (%d, %d)\n", src->GetX(), src->GetY(), dst->GetX(), dst->GetY());
        }
    }




#if 0
    for(int k =0 ; k < 10; k++)
    {
    printf("Unconnected : %d\n", conns.GetUnconnectedCount());

    for( int i = 0; i<m_board->GetAreaCount(); i++ )
    {
        auto zone = m_board->GetArea( i );
        conns.Update( zone );
        printf("Upd %p\n", zone);
    }
    }
#endif

//    vector<int> islands;
    //vector<CN_CONNECTIVITY_IMPL::DISJOINT_NET_ENTRY> report;

//    conns.PropagateNets();

    //return 0;

#if 0
    for( int i = 0; i <m_board->GetAreaCount(); i++ )
    {
        ZONE_CONTAINER* zone = m_board->GetArea( i );
        conns.FindIsolatedCopperIslands( zone, islands );

        std::sort( islands.begin(), islands.end(), std::greater<int>() );

        for( auto idx : islands )
        {
            printf("Delete poly %d/%d\n", idx, zone->FilledPolysList().OutlineCount());
//            zone->FilledPolysList().DeletePolygon( idx );
        }
//        conns.Remove( zone );
//        conns.Add( zone );

    }
#endif
    //conns.CheckConnectivity( report );


//    saveBoard( m_board, "tmp.kicad_pcb" );
    return 0;
}
