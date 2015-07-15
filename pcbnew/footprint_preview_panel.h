#ifndef __FOOTPRINT_PREVIEW_PANEL_H
#define __FOOTPRINT_PREVIEW_PANEL_H

#include <wx/wx.h>

#include <map>
#include <deque>

#include <pcb_draw_panel_gal.h>
#include <fpid.h>
#include <kiway_player.h>

class MODULE;
class KIWAY;
class IO_MGR;
class BOARD;

enum FOOTPRINT_STATUS {
    FPS_NOT_FOUND = 0,
    FPS_READY = 1,
    FPS_LOADING = 2
};

class FOOTPRINT_PREVIEW_PANEL : public PCB_DRAW_PANEL_GAL, public KIWAY_HOLDER
{
public:


    struct CACHE_ENTRY {
        FPID fpid;
        MODULE *module;
        FOOTPRINT_STATUS status;
    };

    FOOTPRINT_PREVIEW_PANEL( KIWAY* aKiway, wxWindow* aParent );

    ~FOOTPRINT_PREVIEW_PANEL( );

    CACHE_ENTRY& CacheFootprint ( const FPID& aFPID );
    void DisplayFootprint ( const FPID& aFPID );
    FOOTPRINT_STATUS GetFootprintStatus ( const FPID& aFPID );

//private:


    void OnPaint( wxPaintEvent& event );

    wxThread *m_loaderThread;
    wxMutex *m_loaderLock;

    std::deque<CACHE_ENTRY> m_loaderQueue;
    std::map<FPID, CACHE_ENTRY> m_cachedFootprints;
    IO_MGR* m_ioManager;

    BOARD *m_dummyBoard;
};

#endif
