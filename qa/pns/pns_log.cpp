#include "pns_log.h"

using namespace PNS;

static const wxString readLine( FILE* f )
{
    char str[16384];
    fgets( str, sizeof( str ) - 1, f );
    return wxString( str );
}

bool PNS_LOG_FILE::Load( const std::string& logName, const std::string boardName )
{
    FILE*    f = fopen( logName.c_str(), "rb" );

    if (!f)
        return false;

    while( !feof( f ) )
    {
        wxStringTokenizer tokens( readLine( f ) );
        if( !tokens.CountTokens() )
            continue;

        wxString cmd = tokens.GetNextToken();

        if (cmd == "event")
        {
            EVENT_ENTRY evt;
            evt.p.x = wxAtoi( tokens.GetNextToken() );
            evt.p.y = wxAtoi( tokens.GetNextToken() );
            evt.type = (PNS::LOGGER::EVENT_TYPE) wxAtoi( tokens.GetNextToken() );
            evt.uuid = KIID( tokens.GetNextToken() );
            m_events.push_back(evt);
        }
    }

    fclose( f );

    try {
        PCB_IO io;
        m_board.reset( io.Load( boardName.c_str(), nullptr, nullptr ) );

    } catch ( const PARSE_ERROR& parse_error ) {
                printf("parse error : %s (%s)\n", 
                    (const char *) parse_error.Problem().c_str(),
                    (const char *) parse_error.What().c_str() );

                    return false;
    }


    return true;
}

PNS_TEST_ENVIRONMENT::PNS_TEST_ENVIRONMENT()
{

}

PNS_TEST_ENVIRONMENT::~PNS_TEST_ENVIRONMENT()
{

}

void PNS_TEST_ENVIRONMENT::SetMode( PNS::ROUTER_MODE mode )
{
    m_mode = mode;
}
    

void PNS_TEST_ENVIRONMENT::createRouter()
{
    m_iface.reset ( new PNS_KICAD_IFACE_BASE);
    m_router.reset( new ROUTER );
    m_iface->SetBoard( m_board.get() );
    m_router->SetInterface( m_iface.get() );
    m_router->ClearWorld();
    m_router->SetMode( m_mode );
    m_router->SyncWorld();
    m_router->LoadSettings( new PNS::ROUTING_SETTINGS (nullptr, ""));
    m_router->Settings().SetMode( PNS::RM_Shove );
    m_debugDecorator.Clear();
    m_iface->SetDebugDecorator( &m_debugDecorator );
}

void PNS_TEST_ENVIRONMENT::ReplayLog ( PNS_LOG_FILE* aLog, int aStartEventIndex, int aFrom, int aTo )
{

    m_board = aLog->GetBoard();

    createRouter();

    for( auto evt : aLog->Events() )
    {
        auto item = aLog->ItemById(evt);
        ITEM* ritem = item ? m_router->GetWorld()->FindItemByParent( item ) : nullptr;

        switch(evt.type)
        {
            case LOGGER::EVT_START_ROUTE:
            {
                m_debugDecorator.NewStage("start", 0);
//                printf("  rtr start-route (%d, %d) %p \n", evt.p.x, evt.p.y, ritem);
                m_router->StartRouting( evt.p, ritem, ritem ? ritem->Layers().Start() : F_Cu );
                break;
            }
            case LOGGER::EVT_FIX:
            break;
            case LOGGER::EVT_MOVE:
                m_debugDecorator.NewStage("move", 0);
            //    printf("  move -> (%d, %d)\n", evt.p.x, evt.p.y );
                m_router->Move( evt.p, ritem );
            break;
            default:
            break;
        }

        auto traces = m_router->Placer()->Traces();

        for ( const auto& t : traces.CItems() )
        {
            const LINE *l  = static_cast<LINE*>(t.item);
            const auto& sh = l->CLine();

            m_debugDecorator.AddLine( sh, 4, 10000 );
        }

        auto node = m_router->Placer()->CurrentNode(true);
        if(!node)
            return;

        NODE::ITEM_VECTOR removed, added;

        node->GetUpdatedItems( removed, added );

        bool first = true;
        for( auto t : added )
        {
            if( t->OfKind( PNS::ITEM::SEGMENT_T ) )
            {
                auto s = static_cast<PNS::SEGMENT*>(t);
                m_debugDecorator.AddSegment( s->Seg(), 2, first ? "node-added-items" : "" );
                first = false;
            }
        }
    }
}


#define dec_dbg(...) printf("[decodbg]" __VA_ARGS__);

PNS_TEST_DEBUG_DECORATOR::STAGE& PNS_TEST_DEBUG_DECORATOR::currentStage()
{
    if (m_stages.empty() )
        m_stages.push_back( STAGE() );

    return m_stages.back();
}


void PNS_TEST_DEBUG_DECORATOR::BeginGroup( std::string name )
{
    auto &st = currentStage();
    DEBUG_ENT ent;

    ent.m_name = name;
    ent.m_iter = m_iter;
    st.m_entries.push_back( ent );

    m_grouping = true;
}


void PNS_TEST_DEBUG_DECORATOR::EndGroup( )
{
    m_grouping = false;
}


void PNS_TEST_DEBUG_DECORATOR::AddPoint( VECTOR2I aP, int aColor,  const std::string aName )
{
    auto &st = currentStage();
    auto sh = new SHAPE_LINE_CHAIN;

    sh->Append(aP.x-100000, aP.y-100000);
    sh->Append(aP.x+100000, aP.y+100000);
    sh->Append(aP.x, aP.y);
    sh->Append(aP.x-100000, aP.y+100000);
    sh->Append(aP.x+100000, aP.y-100000);

    if( !m_grouping ) // new debug entry
    {
        DEBUG_ENT ent;

        ent.m_shapes.push_back( sh );
        ent.m_color = aColor;
        ent.m_width = 30000;
        ent.m_iter = m_iter;
        ent.m_name = aName;
        st.m_entries.push_back( ent );
    } else {
        DEBUG_ENT& ent = st.m_entries.back();
        ent.m_name += " " + aName;
        ent.m_shapes.push_back( sh );
    }


}

void PNS_TEST_DEBUG_DECORATOR::AddLine( const SHAPE_LINE_CHAIN& aLine, int aType, int aWidth,  const std::string aName )
{
    auto &st = currentStage();
    auto sh = new SHAPE_LINE_CHAIN ( aLine );

    if( !m_grouping ) // new debug entry
    {
        DEBUG_ENT ent;
        ent.m_shapes.push_back( sh );
        ent.m_color = aType;
        ent.m_width = aWidth;
        ent.m_name = aName;
        ent.m_iter = m_iter;
        st.m_entries.push_back( ent );
    } else {
        DEBUG_ENT& ent = st.m_entries.back();
        ent.m_name += " " + aName;
        ent.m_shapes.push_back( sh );
    };

}

void PNS_TEST_DEBUG_DECORATOR::AddSegment( SEG aS, int aColor,  const std::string aName )
{
    auto &st = currentStage();
    auto sh = new SHAPE_LINE_CHAIN ( { aS.A, aS.B } );

    if( !m_grouping ) // new debug entry
    {
        DEBUG_ENT ent;
        ent.m_shapes.push_back( sh );
        ent.m_color = aColor;
        ent.m_width = 10000;
        ent.m_name = aName;
        ent.m_iter = m_iter;
        st.m_entries.push_back( ent );
    } else {
        DEBUG_ENT& ent = st.m_entries.back();
        ent.m_name += " " + aName;
        ent.m_shapes.push_back( sh );
    };
}

void PNS_TEST_DEBUG_DECORATOR::AddBox( BOX2I aB, int aColor,  const std::string aName )
{

}

void PNS_TEST_DEBUG_DECORATOR::AddDirections( VECTOR2D aP, int aMask, int aColor,  const std::string aName )
{

}

void PNS_TEST_DEBUG_DECORATOR::Message( const wxString msg )
{
    auto &st = currentStage();

    DEBUG_ENT ent;
    ent.m_msg = msg.c_str();
    st.m_entries.push_back( ent );
}

void PNS_TEST_DEBUG_DECORATOR::Clear()
{
    //dec_dbg("clear");
}

void PNS_TEST_DEBUG_DECORATOR::NewStage(const std::string& name, int iter)
{
    m_stages.push_back( STAGE() );
}

BOX2I PNS_TEST_DEBUG_DECORATOR::GetStageExtents(int stage) const
{
    const auto& st = m_stages[stage];
    BOX2I bb;
    bool first = true;

    for ( auto& ent : st.m_entries )
    {
        for( auto sh : ent.m_shapes )
        {
            if ( first )
                bb = sh->BBox();
            else
                bb.Merge( sh->BBox() );
        }
    }

    return bb;
}
