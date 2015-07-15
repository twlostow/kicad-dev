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
    wxThread ( wxTHREAD_DETACHED ),
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
            } catch( const IO_ERROR& ioe )
            {
                printf("Unable to load FP: %s", GetChars( ioe.errorText ) );
                ent.status = FPS_NOT_FOUND;
            }

            ent.status = FPS_READY;

            m_parent->m_loaderLock->Lock();
            m_parent->m_cachedFootprints [ ent.fpid ] = ent;
            m_parent->m_loaderLock->Unlock();



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

 //   Connect( wxEVT_PAINT, wxPaintEventHandler( FOOTPRINT_PREVIEW_PANEL::OnPaint ), NULL, this );
    DisplayFootprint (  FPID ( std::string ("Choke_SMD:Choke_SMD_7.3x7.3_H4.5" ) ) );


}

FOOTPRINT_PREVIEW_PANEL::~FOOTPRINT_PREVIEW_PANEL( )
{
    delete m_loaderThread;
    delete m_loaderLock;
//    delete m_ioManager;
}

FOOTPRINT_PREVIEW_PANEL::CACHE_ENTRY& FOOTPRINT_PREVIEW_PANEL::CacheFootprint ( const FPID& aFPID )
{
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

void FOOTPRINT_PREVIEW_PANEL::DisplayFootprint ( const FPID& aFPID )
{
    CACHE_ENTRY& fpe = CacheFootprint ( aFPID );

    wxSleep(1);

    if(fpe.status == FPS_READY)
    {
         GetView()->Clear();
         fpe.module->SetParent ( m_dummyBoard );
         fpe.module->RunOnChildren( boost::bind( &KIGFX::VIEW::Add, GetView(), _1 ) );

         GetView()->Add ( fpe.module );


         fpe.module->ViewUpdate();

        BOX2I bbox = fpe.module->ViewBBox();

        if( bbox.GetSize().x > 0 && bbox.GetSize().y > 0 )
        {

            // Autozoom
            GetView()->SetViewport( BOX2D( bbox.GetOrigin(), bbox.GetSize() ) );

            // Add a margin
            GetView()->SetScale( GetView()->GetScale() * 0.7 );

            Refresh();
        }

    }
}

void FOOTPRINT_PREVIEW_PANEL::OnPaint( wxPaintEvent& event )
{
    fprintf(stderr,"refresh!\n");
    Refresh();
}