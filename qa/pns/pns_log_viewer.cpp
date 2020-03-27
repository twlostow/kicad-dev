#define USE_TOOL_MANAGER

#include <wx/clipbrd.h>

#include <pcb_painter.h>
#include <pcb_test_frame.h>
#include <qa_utils/utility_registry.h>


#include <view/view_overlay.h>

#include "pns_log.h"
#include "router/pns_diff_pair.h"
#include "pns_log_viewer_frame_base.h"

#define ID_LIST_COPY 10001
#define ID_LIST_SHOW_ALL 10002
#define ID_LIST_SHOW_NONE 10003

class PNS_LOG_VIEWER_FRAME : public PNS_LOG_VIEWER_FRAME_BASE, public PCB_TEST_FRAME_BASE
{
public:
    PNS_LOG_VIEWER_FRAME( wxFrame* frame ) : PNS_LOG_VIEWER_FRAME_BASE( frame )
    {

        LoadSettings();
        
        createView( this, PCB_DRAW_PANEL_GAL::GAL_TYPE_OPENGL );

        m_viewSizer->Add( m_galPanel.get(), 1, wxEXPAND, 5 );

        Layout();

        Show( true );
        Maximize();
        Raise();

        auto settings = static_cast<KIGFX::PCB_RENDER_SETTINGS*>(
                m_galPanel->GetView()->GetPainter()->GetSettings() );
        settings->SetZoneDisplayMode( KIGFX::PCB_RENDER_SETTINGS::DZ_HIDE_FILLED );
        settings->SetColorDimFactor( 0.5 );

        m_listPopupMenu  = new wxMenu(wxT(""));
        m_listPopupMenu->Append(ID_LIST_COPY, wxT("Copy selected geometry"), wxT(""), wxITEM_NORMAL);
        m_listPopupMenu->Append(ID_LIST_SHOW_ALL, wxT("Show all"), wxT (""), wxITEM_NORMAL);
        m_listPopupMenu->Append(ID_LIST_SHOW_NONE, wxT("Show none"), wxT (""), wxITEM_NORMAL);
        m_itemList->Connect(m_itemList->GetId(),wxEVT_RIGHT_UP,wxMouseEventHandler(PNS_LOG_VIEWER_FRAME::onListRightClick),NULL,this);
        Connect(ID_LIST_COPY,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(PNS_LOG_VIEWER_FRAME::onListCopy),NULL,this);
        Connect(ID_LIST_SHOW_ALL,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(PNS_LOG_VIEWER_FRAME::onListShowAll),NULL,this);
        Connect(ID_LIST_SHOW_NONE,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(PNS_LOG_VIEWER_FRAME::onListShowNone),NULL,this);
        m_itemList->Connect(m_itemList->GetId(),wxEVT_LISTBOX,wxCommandEventHandler(PNS_LOG_VIEWER_FRAME::onListSelect),NULL,this);
    }

    virtual ~PNS_LOG_VIEWER_FRAME()
    {
    }

    std::shared_ptr<KIGFX::VIEW_OVERLAY> Overlay() const { return m_overlay; }
    
    void SetLogFile( PNS_LOG_FILE* aLog );
    

private:
    void drawLoggedItems( int iter );
    void updateDumpPanel( int iter );
    virtual void createUserTools() override;

    virtual void onReload( wxCommandEvent& event ) override;
    virtual void onExit( wxCommandEvent& event ) override;
    virtual void onRewindScroll( wxScrollEvent& event ) override;
    virtual void onRewindCountText( wxCommandEvent& event ) override;
    virtual void onListRightClick(wxMouseEvent& event);
    virtual void onListShowAll( wxCommandEvent& event );
    virtual void onListShowNone( wxCommandEvent& event );
    virtual void onListCopy( wxCommandEvent& event );
    virtual void onListSelect( wxCommandEvent& event );
    virtual void onBtnRewindLeft( wxCommandEvent& event ) override;
	virtual void onBtnRewindRight( wxCommandEvent& event ) override;

    std::shared_ptr<KIGFX::VIEW_OVERLAY>  m_overlay;
    std::shared_ptr<PNS_LOG_FILE>         m_logFile;
    std::shared_ptr<PNS_TEST_ENVIRONMENT> m_env;
    int                                   m_rewindIter;
    wxMenu* m_listPopupMenu;
    int m_selectedItem;
};


void PNS_LOG_VIEWER_FRAME::createUserTools()
{

}


static const COLOR4D assignColor( int aStyle )
{
    COLOR4D color;

    switch( aStyle )
    {
    case 0:
        color = COLOR4D( 0, 1, 0, 1 );
        break;

    case 1:
        color = COLOR4D( 1, 0, 0, 1 );
        break;

    case 2:
        color = COLOR4D( 1, 1, 0, 1 );
        break;

    case 3:
        color = COLOR4D( 0, 0, 1, 1 );
        break;

    case 4:
        color = COLOR4D( 1, 1, 1, 1 );
        break;

    case 5:
        color = COLOR4D( 1, 1, 0, 1 );
        break;

    case 6:
        color = COLOR4D( 0, 1, 1, 1 );
        break;

    case 32:
        color = COLOR4D( 0, 0, 1, 1 );
        break;

    default:
        color = COLOR4D( 0.4, 0.4, 0.4, 1 );
        break;
    }

    return color;
}


void PNS_LOG_VIEWER_FRAME::drawLoggedItems( int iter )
{
    if( !m_env )
        return;
    
    auto dbgd = m_env->GetDebugDecorator();
    int  count = dbgd->GetStageCount();
    int itemID = 0;


    m_overlay->Clear();

    if( count <= 0 )
        return;

    if( iter < 0 )
        iter = 0;

    if( iter >= count )
        iter = count - 1;


    auto st = dbgd->GetStage( iter );

    for( auto& sh : st.m_shapes )
    {
        if ( !m_itemList->IsChecked(itemID ) || !sh.m_shape )
        {
            itemID++;
            continue;
        }

        m_overlay->SetIsStroke( true );
        m_overlay->SetIsFill( false );
        m_overlay->SetStrokeColor( assignColor( sh.m_color ) );
        m_overlay->SetLineWidth( sh.m_width );

        switch( sh.m_shape->Type() )
        {
        case SH_CIRCLE:
        {
            auto cir = static_cast<SHAPE_CIRCLE*>( sh.m_shape );
            m_overlay->Circle( cir->GetCenter(), cir->GetRadius() );

            break;
        }
        case SH_LINE_CHAIN:
        {
            auto lc = static_cast<SHAPE_LINE_CHAIN*>( sh.m_shape );
            //  printf("DrawLC PC %d\n", lc->PointCount() );

            if( itemID == m_selectedItem )
            {
                m_overlay->SetLineWidth( sh.m_width * 2 );
                m_overlay->SetStrokeColor( COLOR4D(1.0, 1.0, 1.0, 1.0) );
            }

            for( int i = 0; i < lc->SegmentCount(); i++ )
            {
                auto s = lc->CSegment( i );
                m_overlay->Line( s.A, s.B );
            }
            break;
        }
        default:
            break;
        }
        itemID++;
    }

    m_galPanel->ForceRefresh();

    m_galPanel->GetView()->MarkDirty();
    m_galPanel->GetParent()->Refresh();
}


static BOARD* loadBoard( const std::string& filename )
{
    PLUGIN::RELEASER pi( new PCB_IO );
    BOARD* brd = nullptr;

    try
    {
        brd = pi->Load( wxString( filename.c_str() ), NULL, NULL );
    }
    catch( const IO_ERROR& ioe )
    {
        wxString msg = wxString::Format( _( "Error loading board.\n%s" ),
                ioe.Problem() );

        printf( "%s\n", (const char*) msg.mb_str() );
        return nullptr;
    }

    return brd;
}


void PNS_LOG_VIEWER_FRAME::SetLogFile( PNS_LOG_FILE* aLog )
{
    m_logFile.reset( aLog );
    
    SetBoard( m_logFile->GetBoard() );

    m_overlay.reset( new KIGFX::VIEW_OVERLAY() );
    m_galPanel->GetView()->Add( m_overlay.get() );

    m_env.reset( new PNS_TEST_ENVIRONMENT );

    m_env->SetMode( PNS::PNS_MODE_ROUTE_SINGLE );
    m_env->ReplayLog( m_logFile.get() );

    auto dbgd = m_env->GetDebugDecorator();
    int  n_stages = dbgd->GetStageCount();
    m_rewindSlider->SetMax( n_stages - 1 );
    m_rewindSlider->SetValue( n_stages - 1 );   
    m_rewindIter = n_stages - 1;
    
    auto extents = m_board->GetBoundingBox();


    BOX2D bbd;
    bbd.SetOrigin( extents.GetOrigin() );
    bbd.SetWidth( extents.GetWidth() );
    bbd.SetHeight( extents.GetHeight() );
    
    bbd.Inflate( std::min( bbd.GetWidth(), bbd.GetHeight() )/ 5);

    m_galPanel->GetView()->SetViewport( bbd );

    drawLoggedItems( m_rewindIter );
    updateDumpPanel( m_rewindIter );
}

void PNS_LOG_VIEWER_FRAME::onReload( wxCommandEvent& event )
{
    event.Skip();
}

void PNS_LOG_VIEWER_FRAME::onExit( wxCommandEvent& event )
{
    event.Skip();
}

void PNS_LOG_VIEWER_FRAME::onRewindScroll( wxScrollEvent& event )
{
    m_rewindIter = event.GetPosition();
    drawLoggedItems(m_rewindIter);
    updateDumpPanel(m_rewindIter);
    char str[128];
    sprintf(str,"%d",m_rewindIter);
    m_rewindPos->SetValue( str );
    event.Skip();
}

void PNS_LOG_VIEWER_FRAME::onBtnRewindLeft( wxCommandEvent& event )
{
    if(m_rewindIter > 0)
    {
        m_rewindIter--;
        drawLoggedItems(m_rewindIter);
        updateDumpPanel(m_rewindIter);
        char str[128];
        sprintf(str,"%d",m_rewindIter);
        m_rewindPos->SetValue( str );
    }
}

void PNS_LOG_VIEWER_FRAME::onBtnRewindRight( wxCommandEvent& event )
{
    auto dbgd = m_env->GetDebugDecorator();
    int  count = dbgd->GetStageCount();
    
    if(m_rewindIter < count)
    {
        m_rewindIter++;
        drawLoggedItems(m_rewindIter);
        updateDumpPanel(m_rewindIter);
        char str[128];
        sprintf(str,"%d",m_rewindIter);
        m_rewindPos->SetValue( str );
    }
}

void PNS_LOG_VIEWER_FRAME::onRewindCountText( wxCommandEvent& event )
{
    printf("OnRewindCountText!\n");

    if( !m_env )
        return;

    int val = wxAtoi( m_rewindPos->GetValue() );

    auto dbgd = m_env->GetDebugDecorator();
    int  count = dbgd->GetStageCount();

    if( val < 0 )
        val = 0;

    if( val >= count )
        val = count - 1;

    m_rewindIter = val;
    m_rewindSlider->SetValue( m_rewindIter );
    drawLoggedItems( m_rewindIter );
    updateDumpPanel( m_rewindIter );

    event.Skip();
}

void PNS_LOG_VIEWER_FRAME::onListRightClick(wxMouseEvent& event)
{
    m_itemList->PopupMenu(m_listPopupMenu);
}

void PNS_LOG_VIEWER_FRAME::onListShowAll( wxCommandEvent& event )
{
    for(unsigned int i = 0; i < m_itemList->GetCount(); i++)
        m_itemList->Check(i, true);

    drawLoggedItems( m_rewindIter );
}

void PNS_LOG_VIEWER_FRAME::onListShowNone( wxCommandEvent& event )
{
    for(unsigned int i = 0; i < m_itemList->GetCount(); i++)
        m_itemList->Check(i, false);

    drawLoggedItems( m_rewindIter );
}

void PNS_LOG_VIEWER_FRAME::onListCopy( wxCommandEvent& event )
{
    wxString s;

    int sel = m_itemList->GetSelection();
    s = m_itemList->GetString(sel);


    if (wxTheClipboard->Open())
    {
        // This data objects are held by the clipboard,
        // so do not delete them in the app.
        wxTheClipboard->SetData( new wxTextDataObject(s) );
        wxTheClipboard->Close();
    }
}

void PNS_LOG_VIEWER_FRAME::onListSelect( wxCommandEvent& event )
{
    m_selectedItem = m_itemList->GetSelection();
    drawLoggedItems( m_rewindIter );
}


void PNS_LOG_VIEWER_FRAME::updateDumpPanel( int iter )
{
    if ( !m_env )
        return;

    auto dbgd = m_env->GetDebugDecorator();
    int  count = dbgd->GetStageCount();

    wxArrayString dumpStrings;

    m_selectedItem = -1;

    //printf("updateDump: %d\n", iter);

    if( count <= 0 )
        return;

    if( iter < 0 )
        iter = 0;

    if( iter >= count )
        iter = count - 1;


    auto st = dbgd->GetStage( iter );



    for( auto& sh : st.m_shapes )
    {
        if( sh.m_shape )
            dumpStrings.Add( wxString::Format( "shape: %s [iter %d]", sh.m_name.c_str(), sh.m_iter ) );
        else
            dumpStrings.Add( sh.m_msg );

        #if 0
        case SH_CIRCLE:
            dumpStrings.Add( wxString::Format( "// shape: circle ", sh.m_name.c_str() ) );
            break;
        case SH_LINE_CHAIN:
        {
            wxString s = wxString::Format( "%s : ", sh.m_name.c_str() );
            auto lc = static_cast<SHAPE_LINE_CHAIN*>( sh.m_shape );
            //  printf("DrawLC PC %d\n", lc->PointCount() );
            s += wxString::Format("auto sh = SHAPE_LINE_CHAIN(");
            for( int i = 0; i < lc->PointCount(); i++ )
            {
                auto p = lc->CPoint( i );
                s += wxString::Format("%d, %d", p.x, p.y );

                if ( i != lc->PointCount() - 1 )
                    s += ",";
                
            }
            s += "); ";
            if ( lc->IsClosed() )
                s += "sh.SetClosed(true); ";
            dumpStrings.Add(s);
            break;
        }
        default:
            break;
            #endif
    }

    m_itemList->Clear();
    m_itemList->Append( dumpStrings );
    
    
    for(unsigned int i = 0; i < m_itemList->GetCount(); i++)
        m_itemList->Check(i, true);
}

using namespace PNS;

static bool commonParallelProjection( SEG p, SEG n, SEG &pClip, SEG& nClip )
{
    SEG n_proj_p( p.LineProject( n.A ), p.LineProject( n.B ) );

    int64_t t_a = 0;
    int64_t t_b = p.TCoef( p.B );

    int64_t tproj_a = p.TCoef( n_proj_p.A );
    int64_t tproj_b = p.TCoef( n_proj_p.B );

    if( t_b < t_a )
        std::swap( t_b, t_a );

    if( tproj_b < tproj_a )
        std::swap( tproj_b, tproj_a );

    if( t_b <= tproj_a )
        return false;

    if( t_a >= tproj_b )
        return false;

    int64_t t[4] = { 0, p.TCoef( p.B ), p.TCoef( n_proj_p.A ), p.TCoef( n_proj_p.B ) };
    std::vector<int64_t> tv( t, t + 4 );
    std::sort( tv.begin(), tv.end() ); // fixme: awful and disgusting way of finding 2 midpoints

    int64_t pLenSq = p.SquaredLength();

    VECTOR2I dp = p.B - p.A;
    pClip.A.x = p.A.x + rescale( (int64_t)dp.x, tv[1], pLenSq );
    pClip.A.y = p.A.y + rescale( (int64_t)dp.y, tv[1], pLenSq );

    pClip.B.x = p.A.x + rescale( (int64_t)dp.x, tv[2], pLenSq );
    pClip.B.y = p.A.y + rescale( (int64_t)dp.y, tv[2], pLenSq );

    nClip.A = n.LineProject( pClip.A );
    nClip.B = n.LineProject( pClip.B );

    return true;
}

std::shared_ptr<KIGFX::VIEW_OVERLAY> g_overlay;

template <typename T1, typename T2>
typename std::map<T1, T2>::iterator findClosestKey( std::map<T1, T2> & data, T1 key)
{
    if (data.size() == 0) {
        return data.end();
    }

    auto lower = data.lower_bound(key);

    if (lower == data.end()) // If none found, return the last one.
        return std::prev(lower);

    if (lower == data.begin())
        return lower;

    // Check which one is closest.
    auto previous = std::prev(lower);
    if ((key - previous->first) < (lower->first - key))
        return previous;

    return lower;
}

void extractDiffPairItems ( PNS::NODE* node, int net_p, int net_n )
{
    std::set<PNS::ITEM*> pendingItems, complementItems;
    node->AllItemsInNet( net_p, pendingItems, PNS::ITEM::SEGMENT_T | PNS::ITEM::ARC_T );
    node->AllItemsInNet( net_n, complementItems, PNS::ITEM::SEGMENT_T | PNS::ITEM::ARC_T );

    while( pendingItems.size() )
    {
        PNS::LINE l = node->AssembleLine( static_cast<PNS::LINKED_ITEM*>( *pendingItems.begin() ) );

        printf("l net %d segs %d layer %d\n", net_p, l.SegmentCount(), l.Layer() );

        std::map<int, int> gapMap;
        std::map<PNS::LINKED_ITEM*, int> coupledCandidates;

        for( auto li : l.Links() )
        {
            pendingItems.erase( li );
            auto sp = dyn_cast<PNS::SEGMENT*> ( li );

            if(!sp)
                continue;

            for ( auto ci : complementItems )
            {
                auto sn = dyn_cast<PNS::SEGMENT*> ( ci );

                if( !sn->Layers().Overlaps( sp->Layers() ))
                    continue;

                
                auto ssp = sp->Seg();
                auto ssn = sn->Seg();
                
                if( ssp.ApproxParallel(ssn) )
                {
                    SEG ca, cb;
                    bool coupled = commonParallelProjection( ssp, ssn, ca, cb );
                    
                    if( coupled )
                    {
                       /* g_overlay->SetStrokeColor( KIGFX::COLOR4D( 1.0, 0.8, 0.8, 1.0 ) );
                        g_overlay->SetIsFill(false);
                        g_overlay->SetIsStroke(true );
                        g_overlay->SetLineWidth( 10000 );
                        g_overlay->Line( ca );
                        g_overlay->SetStrokeColor( KIGFX::COLOR4D( 0.8, 0.8, 1.0, 1.0 ) );
                        g_overlay->Line( cb );*/

                        int len = ca.Length();
                        int gap = (int)(ca.A - cb.A).EuclideanNorm() - (sp->Width()+sn->Width()) / 2;

                        auto closestGap = findClosestKey( gapMap, gap );

                        coupledCandidates[sn] = gap;

                        if( closestGap == gapMap.end() || std::abs(closestGap->first - gap) > 50 )
                        {
                            gapMap[gap] = len;
                        } else {
                            closestGap->second += len;
                        }

                        printf("Seg %p %p gap %d dist %d\n", sp, sn, gap, len );
                    }
                }
            }
        }

        int bestGap = -1;
        int maxLength = 0;
        for( auto i : gapMap )
        {
            if( i.second > maxLength )
            {
                maxLength = i.second;
                bestGap = i.first;
            }
        }

        printf("Best gap: %d\n", bestGap);

        bool pendingCandidates = true;

        while ( pendingCandidates )
        {
            pendingCandidates = false;

            for ( auto c : coupledCandidates )
            {
                if( std::abs(c.second - bestGap ) < 50 )
                {
                    PNS::LINE l_coupled = node->AssembleLine( c.first );

                    PNS::DIFF_PAIR pair(l, l_coupled, bestGap );
                    pair.SetNets( l.Net(), l_coupled.Net() );

/*                     g_overlay->SetStrokeColor( KIGFX::COLOR4D( 1.0, 0.8, 0.8, 1.0 ) );
                     g_overlay->SetIsFill(true);
                     g_overlay->SetIsStroke(true );
                     g_overlay->SetLineWidth( 10000 );
                     g_overlay->Polyline( l.CLine() );
                     g_overlay->SetStrokeColor( KIGFX::COLOR4D( 0.8, 0.8, 1.0, 1.0 ) );
                     g_overlay->Polyline( l_coupled.CLine() );
*/
                    for( auto li : l_coupled.Links() )
                        coupledCandidates.erase( li );

                    printf("Orig line : %d segs, coupled line: %d segs\n", l.SegmentCount(), l_coupled.SegmentCount() );

                    pendingCandidates = true;
                    break;
                }
            }
        }
    }
}



int pns1_main_func( int argc, char* argv[] )
{
    auto frame = new PNS_LOG_VIEWER_FRAME(nullptr);
    std::shared_ptr<BOARD> board;

    board.reset( loadBoard ( argv[1] ) );

    frame->SetBoard( board );

    SetTopFrame( frame );      // wxApp gets a face.

    PNS_KICAD_IFACE_BASE iface;
    PNS::NODE *world = new PNS::NODE;

    iface.SetBoard( board.get() );
    iface.SyncWorld( world );
    g_overlay = frame->Overlay();

    #if 0
    std::map<int, int> diffPairs;

    // FIXMe: GetNetInfo copies f***ing pointers...
    const auto& netinfo = board->GetNetInfo();

    int n_nets = netinfo.GetNetCount();

    for (int net = 0 ; net < n_nets; net++ )
    {
        int coupled = iface.GetRuleResolver()->DpCoupledNet( net );
        if( coupled > 0 && diffPairs.find( coupled ) == diffPairs.end() )
        {
            printf("net %d coupled %d\n", net, coupled );
            diffPairs[net] = coupled;
        }
    }

    for( auto p : diffPairs )
    {
        extractDiffPairItems ( world, p.first, p.second );
    }
    #endif

    return 0;
}

static bool registered = UTILITY_REGISTRY::Register( {
        "pns1",
        "PNS test1",
        pns1_main_func,
} );

int replay_main_func( int argc, char* argv[] )
{
    auto frame = new PNS_LOG_VIEWER_FRAME(nullptr);

    PNS_LOG_FILE* logFile = new PNS_LOG_FILE;
    logFile->Load( argv[1], argv[2] );

    frame->SetLogFile( logFile );
    SetTopFrame( frame );      // wxApp gets a face.

    return 0;
}

static bool registered2 = UTILITY_REGISTRY::Register( {
        "replay",
        "PNS log replay",
        replay_main_func,
} );
