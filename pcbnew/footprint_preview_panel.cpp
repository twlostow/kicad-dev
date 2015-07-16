#include <footprint_preview_panel.h>

#include <kiway.h>
#include <io_mgr.h>
#include <fp_lib_table.h>
#include <view/view.h>
#include <math/box2.h>
#include <class_module.h>
#include <class_board.h>

#include <boost/bind.hpp>

class LOADER_THREAD : public wxThread
    {
        public:
            LOADER_THREAD ( FOOTPRINT_PREVIEW_PANEL *aParent );
            ~LOADER_THREAD ();


        protected:
            void* Entry();
            FOOTPRINT_PREVIEW_PANEL *m_parent;
    };

LOADER_THREAD::LOADER_THREAD ( FOOTPRINT_PREVIEW_PANEL* aParent ) :
    wxThread ( wxTHREAD_JOINABLE ),
    m_parent( aParent )
{

}

LOADER_THREAD::~LOADER_THREAD ()
{

}

void* LOADER_THREAD::Entry()
{
    printf("Loader thread starting...\n");
    while(!TestDestroy ())
    {
        m_parent->m_loaderLock->Lock();

        FOOTPRINT_PREVIEW_PANEL::CACHE_ENTRY ent;
        bool empty = m_parent->m_loaderQueue.empty();

        if(!empty)
        {
            ent = m_parent->m_loaderQueue.front();
            m_parent->m_loaderQueue.pop_front();
        }

        m_parent->m_loaderLock->Unlock();

        if(!empty)
        {
            printf("Loading footprint %s\n", ent.fpid.Format().c_str());

            FP_LIB_TABLE*   fptbl = m_parent->Prj().PcbFootprintLibs();

            if(!fptbl)
                continue;

            ent.module = NULL;

            try {
                ent.module = fptbl->FootprintLoadWithOptionalNickname( ent.fpid );

                if(ent.module == NULL)
                    ent.status = FPS_NOT_FOUND;

            } catch( const IO_ERROR& ioe )
            {
                printf("Unable to load FP: %s", GetChars( ioe.errorText ) );
                ent.status = FPS_NOT_FOUND;
            }


            if(ent.status != FPS_NOT_FOUND )
                ent.status = FPS_READY;


            m_parent->m_loaderLock->Lock();
            m_parent->m_cachedFootprints [ ent.fpid ] = ent;
            m_parent->m_loaderLock->Unlock();

            if (ent.fpid == m_parent->m_currentFPID)
            {
                wxCommandEvent event( wxEVT_COMMAND_TEXT_UPDATED, 1 );
                m_parent->GetEventHandler()->AddPendingEvent ( event );
            }

        } else {
            wxMilliSleep(100);
        }
    }

    return NULL;
}



FOOTPRINT_PREVIEW_PANEL::FOOTPRINT_PREVIEW_PANEL( KIWAY* aKiway, wxWindow* aParent ) :
    PCB_DRAW_PANEL_GAL ( aParent,-1, wxPoint( 0, 0 ), wxSize(200, 200) ),
    KIWAY_HOLDER ( aKiway )
{

  //  m_ioManager = new IO_MGR();
    m_loaderLock = new wxMutex;
    m_loaderThread = new LOADER_THREAD ( this );
    m_loaderThread->Run();

    ShowScrollbars( wxSHOW_SB_NEVER, wxSHOW_SB_NEVER );
    EnableScrolling( false, false );    // otherwise Zoom Auto disables GAL canvas

    m_dummyBoard = new BOARD;

    UseColorScheme( m_dummyBoard->GetColorsSettings() );
    SyncLayersVisibility( m_dummyBoard );

    Raise();
    Show(true);
    StartDrawing();

    Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( FOOTPRINT_PREVIEW_PANEL::OnLoaderThreadUpdate ), NULL, this );

    DisplayFootprint (  FPID ( std::string ("Choke_SMD:Choke_SMD_7.3x7.3_H4.5" ) ) );
    DisplayFootprint (  FPID ( std::string ("SMD_Packages:BGA-121-1mm" ) ) );
    DisplayFootprint (  FPID ( std::string ("SMD_Packages:BGA-400-1mm" ) ) );
    DisplayFootprint (  FPID ( std::string ("SMD_Packages:BGA-256_pitch1mm_dia0.4mm" ) ) );
    DisplayFootprint (  FPID ( std::string ("SMD_Packages:BGA-352" ) ) );
    DisplayFootprint (  FPID ( std::string ("SMD_Packages:BGA-256" ) ) );
    DisplayFootprint (  FPID ( std::string ("SMD_Packages:BGA-144-1mm" ) ) );
    DisplayFootprint (  FPID ( std::string ("SMD_Packages:BGA-1023-1mm" ) ) );
    DisplayFootprint (  FPID ( std::string ("SMD_Packages:BGA-64-0.8mm" ) ) );
    DisplayFootprint (  FPID ( std::string ("SMD_Packages:BGA-625_pitch0.8mm_dia0.4mm" ) ) );

}

FOOTPRINT_PREVIEW_PANEL::~FOOTPRINT_PREVIEW_PANEL( )
{
    printf("killing thread...\n");
    m_loaderThread->Delete();

    delete m_loaderThread;
    delete m_loaderLock;
//    delete m_ioManager;
}

FOOTPRINT_PREVIEW_PANEL::CACHE_ENTRY& FOOTPRINT_PREVIEW_PANEL::CacheFootprint ( const FPID& aFPID )
{
    if( m_cachedFootprints.find( aFPID ) != m_cachedFootprints.end() )
        return m_cachedFootprints[aFPID];

    CACHE_ENTRY ent;

    ent.fpid = aFPID;
    ent.status = FPS_LOADING;
    ent.module = NULL;

    printf("Cache footprint %s\n", ent.fpid.Format().c_str());


    m_loaderLock->Lock();
    m_cachedFootprints [ ent.fpid ] = ent;
    m_loaderQueue.push_back ( ent );

    CACHE_ENTRY& rv = m_cachedFootprints [ ent.fpid ];

    m_loaderLock->Unlock();

    return rv;
}

void FOOTPRINT_PREVIEW_PANEL::renderFootprint(  MODULE *module )
{
      GetView()->Clear();
         module->SetParent ( m_dummyBoard );
         module->RunOnChildren( boost::bind( &KIGFX::VIEW::Add, GetView(), _1 ) );

         GetView()->Add ( module );


         module->ViewUpdate();

        BOX2I bbox = module->ViewBBox();
        bbox.Merge ( module->Value().ViewBBox() );
        bbox.Merge ( module->Reference().ViewBBox() );

        if( bbox.GetSize().x > 0 && bbox.GetSize().y > 0 )
        {

            // Autozoom
            GetView()->SetViewport( BOX2D( bbox.GetOrigin(), bbox.GetSize() ) );

            // Add a margin
            GetView()->SetScale( GetView()->GetScale() * 0.7 );

            Refresh();
        }
}


void FOOTPRINT_PREVIEW_PANEL::DisplayFootprint ( const FPID& aFPID )
{
    m_currentFPID = aFPID;
    m_footprintDisplayed = false;

    CACHE_ENTRY& fpe = CacheFootprint ( aFPID );


    printf("status %d\n", fpe.status);
    if(fpe.status == FPS_READY)
    {
        renderFootprint ( fpe.module );
        m_footprintDisplayed = true;
    }
}

void FOOTPRINT_PREVIEW_PANEL::OnLoaderThreadUpdate( wxCommandEvent& event )
{
    CACHE_ENTRY& fpe = CacheFootprint ( m_currentFPID );

    printf("thread update [ stat %d %p ]\n", fpe.status, fpe.module);

    if(fpe.status != FPS_READY )
        return;

    if ( !m_footprintDisplayed )
    {
        renderFootprint ( fpe.module );
        m_footprintDisplayed = true;
        Refresh();
    }
}