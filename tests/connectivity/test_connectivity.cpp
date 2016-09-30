#include <class_board.h>
#include <class_zone.h>
#include <io_mgr.h>

#include "connectivity.h"
#include "connectivity_algo.h"

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

#include "case2.cpp"

int main( int argc, char* argv[] )
{

    std::vector<BOARD_ITEM*> allItems;

/*    if( argc < 2 )
    {
        printf( "usage : %s board_file\n", argv[0] );
        return 0;
    }*/


    testRN();
    return 0;

    loadBoard( argv[1] );

    CONNECTIVITY_DATA conns;

    conns.Build( m_board.get() );

    auto cnAlgo = conns.GetConnectivityAlgo();
    PROF_COUNTER cnt_cl("build-clusters",true);
    auto clusters = cnAlgo->GetClusters();
    cnt_cl.Show();

    printf("%d clusters\n", clusters.size());

    for ( auto item : m_board->Tracks() )
        allItems.push_back( item );
    for ( int i = 0; i < m_board->GetAreaCount(); i++)
        allItems.push_back( m_board->GetArea(i) );

    int tries = 10;
    int itemsPerTurn = 10;

    std::set<BOARD_ITEM *> erased;

    while(tries--)
    {
        /*for(i = 0; i < itemsPerTurn && allItems.size() > 0; i++)
        {
            auto item = allItems[ random() % allItems.size() ];
            conns.Remove(item);
            erased.Insert(item);
        }*/
    }


}
