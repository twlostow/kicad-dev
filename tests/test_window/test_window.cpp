/*
   Simple STEP/IGES File Viewer

   GL stuff based on wxWidgets "isosurf" example.

   T.W. 2013
*/

#include <wx/wx.h>
#include <wx/app.h>

#include <wx/timer.h>
#include <wx/math.h>
#include <wx/log.h>
#include <wx/popupwin.h>

#include <layers_id_colors_and_visibility.h>

#include <gal/graphics_abstraction_layer.h>
#include <class_draw_panel_gal.h>
#include <class_draw_panel_gal_ng.h>
#include <view/view.h>
#include <view_ng.h>
#include <view/view_overlay.h>
#include <view_cache.h>
#include <view_rtree_ng.h>
#include <view/wx_view_controls.h>
#include <pcb_painter.h>
#include <pad_shapes.h>
#include <profile.h>

#include <class_pad.h>
#include <class_module.h>
#include <class_board.h>
#include <class_track.h>
#include <class_zone.h>
#include <io_mgr.h>

#include <set>

#include <boost/foreach.hpp>
#include <boost/bind.hpp>

#include "test_window.h"

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

    // Create the main frame window
    new GAL_TEST_FRAME(NULL, wxT("board-test"));

    return true;
}

void MyApp::OnInitCmdLine(wxCmdLineParser& parser)
{
/*    parser.AddSwitch("", "sb", "Do not use double buffering");
    parser.AddSwitch("", "db", "Use double buffering");
    parser.AddSwitch("", "va", "Use vertex arrays");

    wxApp::OnInitCmdLine(parser);*/
}

bool MyApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
    /*if (parser.Found("sb"))
        g_doubleBuffer = GL_FALSE;
    else if (parser.Found("db"))
        g_doubleBuffer = GL_TRUE;


    return wxApp::OnCmdLineParsed(parser);*/
    return true;
}


//---------------------------------------------------------------------------
// GAL_TEST_FRAME
//---------------------------------------------------------------------------

//BEGIN_EVENT_TABLE(GAL_TEST_FRAME, wxFrame)
  //  EVT_MENU(wxID_EXIT, GAL_TEST_FRAME::OnExit)
    //EVT_MENU(wxID_OPEN, GAL_TEST_FRAME::OnMenuFileOpen)
    ///VT_MOTION(GAL_TEST_FRAME::OnMotion)
//END_EVENT_TABLE()


void GAL_TEST_FRAME::OnMenuFileOpen( wxCommandEvent& WXUNUSED(event) )
{
}




void GAL_TEST_FRAME::OnMotion( wxMouseEvent& event )
{
}


using namespace KIGFX;

const BOX2I VIEW_ITEM_NG::ngViewBBox() const
{
  return static_cast<const VIEW_ITEM *>(this)->ViewBBox();
}

typedef std::vector<VIEW_RTREE_ENTRY*> DISP_LIST;

class PCB_VIEW : public KIGFX::VIEW_BASE {

private:

    enum Layers {
      L_PADS = 0,
      L_VIAS,
      L_TRACKS,
      L_GRAPHICS,
      L_ZONES,
      L_MODULES,
      L_RATSNEST
    };

public:

  PCB_VIEW();
  ~PCB_VIEW();

    bool m_needsRecache;

    virtual void SetBoard ( BOARD *aBoard )
    {
      PROF_COUNTER pcnt ("build-board-view",true);
      for ( MODULE *mod = aBoard->m_Modules; mod; mod = mod->Next() )
        addModule ( mod );
      for ( TRACK *trk = aBoard->m_Track; trk; trk = trk->Next() )
        if(VIA *via = dyn_cast<VIA*> ( trk ))
          VIEW_BASE::Add(via, L_VIAS);
        else
          VIEW_BASE::Add(trk, L_TRACKS);

      for( BOARD_ITEM* drawing = aBoard->m_Drawings; drawing; drawing = drawing->Next() )
        VIEW_BASE::Add(drawing, L_GRAPHICS);

//      for( int i = 0; i < aBoard->GetAreaCount(); ++i )
    //      VIEW_BASE::Add( aBoard->GetArea( i ), L_ZONES );


        pcnt.show();
        m_needsRecache = true;
        printf("cache size: %d\n", m_cache->Size());

        BOX2I extents = CalculateExtents();

        extents.Inflate( extents.GetWidth() / 10 );



        SetViewport ( BOX2D( extents.GetPosition(), extents.GetSize() ) );

    }


    struct VIEW_DISP_LIST
    {
      typedef std::vector<VIEW_RTREE_ENTRY *> ENTRIES;
      std::vector<ENTRIES> m_layers;

      VIEW_DISP_LIST()
      {
        int i;
        m_layers.reserve (LAYER_ID_COUNT);

        for(int i=0; i< LAYER_ID_COUNT; i++)
          m_layers.push_back( ENTRIES() );

        for(int i=0; i< LAYER_ID_COUNT; i++)
          m_layers[i].reserve(16384);
      }

      void Add ( LAYER_ID layer, VIEW_RTREE_ENTRY *ent )
      {
        m_layers [layer].push_back(ent);
      }

      void Clear ( )
      {
        for(int i=0; i< LAYER_ID_COUNT; i++)
          m_layers[i].clear();
      }

      ENTRIES& Layer ( LAYER_ID layer )
      {
        return m_layers [ layer ];
      }


    };



  void doDraw ( LAYER_ID layerId, RENDER_TARGET target, VIEW_DISP_LIST *displist, int aGalSublayer=-1, int cacheIndex=-1, boost::function<int (BOARD_ITEM*)> aLODFunc = NULL )
  {
      VIEW_DISP_LIST::ENTRIES & list = displist->m_layers[layerId];

      m_gal->SetTarget(target);

      BOOST_FOREACH( VIEW_RTREE_ENTRY* rtreeEnt, list)
      {
        BOARD_ITEM *item = static_cast<BOARD_ITEM *>(rtreeEnt->item);
        VIEW_CACHE_ENTRY *ent = rtreeEnt->ent;

        int lod = 0;

        if( !ent->IsVisible() )
            continue;

        if( aLODFunc )
            lod = aLODFunc(item);

        if( lod > m_scale && !ent->IsDirty() )
            continue;

        if( cacheIndex >= 0 && ent->TestFlags ( VI_CACHEABLE ) )
        {
            int grp = ent->GetGroup( cacheIndex );
            if( ent->IsDirty() || grp < 0 )
            {
                if(grp >= 0)
                    m_gal->DeleteGroup(grp);

                grp = m_gal->BeginGroup();
                ent->SetGroup(cacheIndex,  grp );
                m_painter->Draw( item, aGalSublayer );
                m_gal->EndGroup();
            }

            if(grp >= 0)
                m_gal->DrawGroup( grp );
        } else {
            m_painter->Draw( item, aGalSublayer );
        }
     }
 }


  static int netnameLOD( BOARD_ITEM *item )
  {
    switch(item->Type())
    {
      case PCB_MODULE_TEXT_T:
        return 0;
      case PCB_PAD_T:
      {
        D_PAD *pad = static_cast<D_PAD *>(item);
        if( ( pad->GetSize().x == 0 ) && ( pad->GetSize().y == 0 ) )
          return UINT_MAX;

          return ( 100000000 / std::max( pad->GetSize().x, pad->GetSize().y ) );
      }

      case PCB_TRACE_T:
        return ( 20000000 / ( static_cast<TRACK*>(item)->GetWidth() + 1 ) );
      default:
        return 0;
    }
  }

  int primaryLayer (  BOARD_ITEM *item )
  {
    switch(item->Type())
    {
      case PCB_PAD_T:
      {
        D_PAD *pad = static_cast<D_PAD *>(item);
        if(pad->GetAttribute( ) == PAD_ATTRIB_STANDARD )
          return ITEM_GAL_LAYER(PADS_VISIBLE);
        else
          return pad->GetLayer();
      }

      default:
        return item->GetLayer();
      }
  }

  virtual void recache()
  {

  }


#define THRU_PADS_ONLY 1

  struct queryVisitor
  {
      queryVisitor( VIEW_DISP_LIST& aCont, int aFlag = 0) :
          m_cont( aCont ),
          m_flag ( aFlag )
      {
      }

      bool operator()( VIEW_RTREE_ENTRY* aItem )
      {
          LAYER_ID layer = static_cast<BOARD_ITEM *> (aItem->item)->GetLayer();

          BOARD_ITEM *item = static_cast<BOARD_ITEM*>(aItem->item);

          if(item->Type() == PCB_PAD_T)
          {
            D_PAD *pad = static_cast<D_PAD*>(item);
            //printf("Layer %d attr %d\n", pad->GetLayer(), pad->GetAttribute());

            if ( pad->IsOnLayer(F_Cu) )
                layer = F_Cu;
            else
                layer = B_Cu;

                switch(pad->GetAttribute())
                {
                  case PAD_ATTRIB_HOLE_NOT_PLATED:
                  case PAD_ATTRIB_STANDARD:
                    if( !m_flag )
                      return true;
                    break;
                  default:
                    if(m_flag == THRU_PADS_ONLY)
                      return true;
                    break;

                  }
                //if ( (pad->GetAttribute() == PAD_ATTRIB_STANDARD || pad->GetAttribute() == PAD_ATTRIB_HOLE_NOT_PLATED)  && m_flag != THRU_PADS_ONLY )
                  //return true;
          }


          m_cont.Add(layer, aItem);

          return true;
      }

      int m_flag;
      VIEW_DISP_LIST&  m_cont;
  };

  void Query ( const BOX2I& aRect, int layer, VIEW_DISP_LIST& list, int aFlag = 0 )
  {
    queryVisitor v (list, aFlag);
    m_layers[layer].items->Query(aRect, v);
  }

  void checkGeometryUpdates()
  {
      PROF_COUNTER cnt ("geom-update", true);
    int count = 0;
    VIEW_CACHE::CACHE_ENTRIES& cacheMap = m_cache->GetCache();

    for( VIEW_CACHE::CACHE_ENTRIES::iterator i = cacheMap.begin(); i != cacheMap.end(); ++i )
    {
      const BOX2I newBB = i->first->ngViewBBox();
      if  ( newBB.GetOrigin() != i->second->GetBBox().GetOrigin() || newBB.GetSize() != i->second->GetBBox().GetSize() ) // fixme: operator==
      {
        i->second->SetBBox( newBB );
        count++;
      }
    }

    cnt.show();
    printf("Updated cache entries : %d\n", count);
  }

  virtual void redrawRect( const BOX2I& aRect  )
  {
    static int count = 0;

    VIEW_DISP_LIST vias, pads_smd, pads_tht, items, mods;

    BOX2I rect (aRect);

    checkGeometryUpdates();

    if(m_needsRecache)
    {
        m_cache->SetDirty(true);
        rect.SetMaximum();
    }

    PROF_COUNTER cnt ( "query-items", true );
    //#pragma omp parallel
    {
      Query (rect, L_TRACKS, items);
      Query (rect, L_GRAPHICS, items);
      Query (rect, L_ZONES, items);
      Query (rect, L_PADS, pads_smd);
      Query (rect, L_PADS, pads_tht, THRU_PADS_ONLY);
      Query (rect, L_VIAS, vias);
      Query (rect, L_MODULES, mods);
    }


    cnt.show();

    PROF_COUNTER cnt2("draw-items",true);

    doDraw(B_SilkS, TARGET_CACHED, &mods, ITEM_GAL_LAYER( MOD_TEXT_BK_VISIBLE ) , 0);
    doDraw(B_SilkS, TARGET_CACHED, &items, B_SilkS, 0);
    doDraw(B_Paste, TARGET_CACHED, &items, B_Paste, 0);
    doDraw(B_Adhes, TARGET_CACHED, &items, B_Adhes, 0);
    doDraw(B_Mask, TARGET_CACHED, &items, B_Mask, 0);
    doDraw(B_Cu, TARGET_CACHED, &pads_smd, B_SilkS, 1 );
    doDraw(F_Cu, TARGET_CACHED, &pads_tht, B_SilkS, 1 );
    doDraw(B_Cu, TARGET_CACHED, &pads_smd, B_Paste, 2 );
    doDraw(F_Cu, TARGET_CACHED, &pads_tht, B_Paste, 2 );
    doDraw(B_Cu, TARGET_CACHED, &pads_smd, B_Adhes, 3 );
    doDraw(F_Cu, TARGET_CACHED, &pads_tht, B_Adhes, 3 );
    doDraw(B_Cu, TARGET_CACHED, &pads_smd, B_Mask, 4 );
    doDraw(F_Cu, TARGET_CACHED, &pads_tht, B_Mask, 4 );
    doDraw(B_Cu, TARGET_CACHED, &items, B_Cu, 0 );
    doDraw(B_Cu, TARGET_CACHED, &items, NETNAMES_GAL_LAYER( B_Cu ), 1, netnameLOD );
    doDraw(B_Cu, TARGET_CACHED, &pads_smd, ITEM_GAL_LAYER( PAD_BK_VISIBLE ), 0 );
    doDraw(B_Cu, TARGET_CACHED, &pads_smd, ITEM_GAL_LAYER( PAD_BK_NETNAMES_VISIBLE ), 1, netnameLOD );

    for(int i = In30_Cu; i >= In1_Cu; i--)
    {
      doDraw( (LAYER_ID) i, TARGET_CACHED, &items, i, 0 );
      doDraw( (LAYER_ID) i, TARGET_CACHED, &items, NETNAMES_GAL_LAYER( i ), 1, netnameLOD );
      doDraw( (LAYER_ID) i, TARGET_CACHED, &vias, ITEM_GAL_LAYER (VIAS_VISIBLE), 0 );
      doDraw( (LAYER_ID) i, TARGET_CACHED, &vias, ITEM_GAL_LAYER (VIAS_HOLES_VISIBLE), 1 );
    }


    doDraw(F_Cu, TARGET_CACHED, &pads_smd, F_Adhes, 1 );
    doDraw(F_Cu, TARGET_CACHED, &pads_tht, F_Adhes, 5 );
    doDraw(F_Cu, TARGET_CACHED, &pads_smd, F_Paste, 2 );
    doDraw(F_Cu, TARGET_CACHED, &pads_tht, F_Paste, 6 );
    doDraw(F_Cu, TARGET_CACHED, &pads_smd, F_SilkS, 3 );
    doDraw(F_Cu, TARGET_CACHED, &pads_tht, F_SilkS, 7 );
    doDraw(F_Cu, TARGET_CACHED, &pads_smd, F_Mask, 4 );
    doDraw(F_Cu, TARGET_CACHED, &pads_tht, F_Mask, 8 );
    doDraw(F_Cu, TARGET_CACHED, &items, F_Cu, 0 );
    doDraw(F_Cu, TARGET_CACHED, &items, NETNAMES_GAL_LAYER( F_Cu ), 1, netnameLOD );
    doDraw(F_Cu, TARGET_CACHED, &pads_smd, ITEM_GAL_LAYER( PAD_FR_VISIBLE ), 0 );
    doDraw(F_Cu, TARGET_CACHED, &pads_smd, ITEM_GAL_LAYER( PAD_FR_NETNAMES_VISIBLE ), 1, netnameLOD );
    doDraw(F_Cu, TARGET_CACHED, &pads_tht, ITEM_GAL_LAYER( PADS_VISIBLE ), 0 );
    doDraw(F_Cu, TARGET_CACHED, &pads_tht, ITEM_GAL_LAYER( PADS_HOLES_VISIBLE ), 1 );
    doDraw(F_Cu, TARGET_CACHED, &pads_tht, ITEM_GAL_LAYER( PADS_NETNAMES_VISIBLE ), 2, netnameLOD );
    doDraw(F_Cu, TARGET_CACHED, &vias, ITEM_GAL_LAYER (VIAS_VISIBLE), 0 );
    doDraw(F_Cu, TARGET_CACHED, &vias, ITEM_GAL_LAYER (VIAS_HOLES_VISIBLE), 1 );

    doDraw(Dwgs_User, TARGET_CACHED, &items, Dwgs_User, 0 );
    doDraw(Cmts_User, TARGET_CACHED, &items, Cmts_User, 0 );
    doDraw(Eco1_User, TARGET_CACHED, &items, Eco1_User, 0 );
    doDraw(Eco2_User, TARGET_CACHED, &items, Eco2_User, 0 );
    doDraw(Edge_Cuts, TARGET_CACHED, &items, Edge_Cuts, 0 );

    doDraw(F_SilkS, TARGET_CACHED, &mods, ITEM_GAL_LAYER( MOD_TEXT_FR_VISIBLE ), 0);
    doDraw(F_SilkS, TARGET_CACHED, &items, F_SilkS, 0);


    cnt2.show();

    if(m_needsRecache)
    {
      m_cache->SetDirty(false);
      m_needsRecache = false;
    }


  };


  void SyncLayersVisibility( )
  {


  }

private:
    void addModule( MODULE *mod )
    {
      for ( D_PAD *pad = mod->Pads(); pad; pad = pad->Next() )
        VIEW_BASE::Add(pad, L_PADS);
      for ( BOARD_ITEM *item = mod->GraphicalItems(); item; item = item->Next() )
        VIEW_BASE::Add(item, L_GRAPHICS);

      VIEW_BASE::Add(&mod->Reference(), L_MODULES );
      VIEW_BASE::Add(&mod->Value(), L_MODULES );
    }

};

PCB_VIEW::PCB_VIEW() :
    KIGFX::VIEW_BASE()
{
    AddLayer( L_PADS );
    AddLayer( L_VIAS );
    AddLayer( L_TRACKS );
    AddLayer( L_GRAPHICS );
    AddLayer( L_ZONES );
    AddLayer( L_MODULES );

    m_needsRecache = true;
}

PCB_VIEW::~PCB_VIEW()
{
    // delete layers here & r-trees here
}

GAL_TEST_FRAME::GAL_TEST_FRAME(wxFrame *frame, const wxString& title, const wxPoint& pos,
                 const wxSize& size, long style)
    : wxFrame(frame, wxID_ANY, title, pos, size, style)
{


    // Make a menubar
    wxMenu *fileMenu = new wxMenu;

    fileMenu->Append(wxID_OPEN, wxT("&Open..."));
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, wxT("E&xit"));
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, wxT("&File"));
    SetMenuBar(menuBar);

    Show(true);
    Raise();

    PCB_PAINTER *painter = new KIGFX::PCB_PAINTER( NULL );
    PCB_VIEW *view = new PCB_VIEW;
    view->SetPainter ( painter );

    m_galPanel = new EDA_DRAW_PANEL_GAL_NG( this, -1, wxPoint( 0, 0 ), wxDefaultSize, EDA_DRAW_PANEL_GAL_NG::GAL_TYPE_OPENGL, view ) ;
    m_galPanel->SetEvtHandlerEnabled( true );
    m_galPanel->SetFocus();

    //m_galPanel->Connect ( wxEVT_MOTION, wxMouseEventHandler( GAL_TEST_FRAME::OnMotion ), NULL, this );
    m_galPanel->StartDrawing();

    //KIGFX::VIEW *view = m_galPanel->GetView();

    printf("View: %p\n", view );

  //view->SetLayerTarget( 0, KIGFX::TARGET_OVERLAY );

    view->SetCenter (VECTOR2D (0, 0));
    view->SetScale ( 1.0 );



//    m_ovl = m_galPanel->GetView()->MakeOverlay();

  //  m_ovl->Begin();


    BOARD *b = new BOARD();

    PLUGIN::RELEASER pi( IO_MGR::PluginFind( IO_MGR::KICAD ) );

    try
    {
	     b=pi -> Load( wxT("../../demos/video/video.kicad_pcb"), NULL, NULL );
       //b=pi -> Load( wxT("../../../altium-import/wrs.kicad_pcb"), NULL, NULL );
    }
    catch( const IO_ERROR& ioe )
    {
        wxString msg = wxString::Format( _( "Error loading board.\n%s" ),
                                         ioe.errorText.GetData() );

        printf("%s\n",(const char *) msg.mb_str());
    }

    printf("brd %p\n", b);

    view->SetBoard(b);

    KIGFX::PCB_RENDER_SETTINGS* rs;
    rs = static_cast<KIGFX::PCB_RENDER_SETTINGS*>( painter->GetSettings() );
    rs->ImportLegacyColors( b->GetColorsSettings() );




/*    BOARD* brd = new BOARD();
    TRACK  t1(brd);

    t1.SetStart(wxPoint(0, 0));
    t1.SetEnd(wxPoint(1000000, 0));
    t1.SetLayer(F_Cu);
    t1.SetWidth(100000);*/

#if 0
    for (double angle = 0; angle < 360.0; angle += 5.0)
    {
        VECTOR2D center (0, 0);
        VECTOR2D p ( center.x + 20000000 * cos (angle * M_PI/180.0), center.y + 20000000 * sin (angle * M_PI/180.0) );

        VECTOR2D p0 =  center;
        VECTOR2D p1 =  p;

        m_ovl->DrawLine (p0, p1 );

//        printf("%.0f %.0f %.0f %.0f\n", p0.x, p0.y, p1.x ,p1.y);



     //   printf("DrawL %.1f\n", angle);

    }
    m_ovl->End();
#endif


}

GAL_TEST_FRAME::~GAL_TEST_FRAME()
{
    delete m_galPanel;
}

// Intercept menu commands
void GAL_TEST_FRAME::OnExit( wxCommandEvent& WXUNUSED(event) )
{
    // true is to force the frame to close
    Close(true);
}
