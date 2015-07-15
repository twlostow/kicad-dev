/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007 Jean-Pierre Charras, jp..charras at wanadoo.fr
 * Copyright (C) 2014 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 1992-2014 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

/**
 * @file cvpcb.cpp
 */

#include <fctsys.h>
#include <macros.h>
#include <kiface_i.h>
#include <kiway.h>

#include <pgm_base.h>
#include <wxBasePcbFrame.h>

#include <footprint_preview_panel.h>

static struct IFACE : public KIFACE_I
{
    // Of course all are virtual overloads, implementations of the KIFACE.

    IFACE( const char* aName, KIWAY::FACE_T aType ) :
        KIFACE_I( aName, aType )
    {}

    bool OnKifaceStart( PGM_BASE* aProgram, int aCtlBits );

    void OnKifaceEnd();

    wxWindow* CreateWindow( wxWindow* aParent, int aClassId, KIWAY* aKiway, int aCtlBits = 0 );


    /**
     * Function IfaceOrAddress
     * return a pointer to the requested object.  The safest way to use this
     * is to retrieve a pointer to a static instance of an interface, similar to
     * how the KIFACE interface is exported.  But if you know what you are doing
     * use it to retrieve anything you want.
     *
     * @param aDataId identifies which object you want the address of.
     *
     * @return void* - and must be cast into the know type.
     */
    void* IfaceOrAddress( int aDataId )
    {
        return NULL;
    }

} kiface( "test", KIWAY::FACE_TEST );

/**
 * The CvPcb application main window.
 */
class TEST_FRAME : public KIWAY_PLAYER
{
    friend struct IFACE;

protected:
    TEST_FRAME( KIWAY* aKiway, wxWindow* aParent );

public:
    ~TEST_FRAME ();

    void KiwayMailIn( KIWAY_EXPRESS& aEvent );


    DECLARE_EVENT_TABLE()
private:

    FOOTPRINT_PREVIEW_PANEL *m_fpPreview;
};


TEST_FRAME::TEST_FRAME( KIWAY* aKiway, wxWindow* aParent )
 :
 KIWAY_PLAYER( aKiway, aParent, FRAME_TEST, wxT( "Test" ), wxDefaultPosition,
 wxDefaultSize, KICAD_DEFAULT_DRAWFRAME_STYLE, "Test" )
 {
     this->Show(true);

     KIFACE* kiface = Kiway().KiFACE( KIWAY::FACE_PCB );
     wxASSERT( kiface );

     m_fpPreview = static_cast<FOOTPRINT_PREVIEW_PANEL*> ( kiface->CreateWindow ( this, FRAME_PCB_FOOTPRINT_PREVIEW, &Kiway() ) );
     m_fpPreview->Raise();
     m_fpPreview->Show();
 }

TEST_FRAME::~TEST_FRAME()
{
}

void TEST_FRAME::KiwayMailIn( KIWAY_EXPRESS& aEvent )
{

}

BEGIN_EVENT_TABLE(TEST_FRAME, wxFrame)
END_EVENT_TABLE()

static PGM_BASE* process;


KIFACE_I& Kiface() { return kiface; }


// KIFACE_GETTER's actual spelling is a substitution macro found in kiway.h.
// KIFACE_GETTER will not have name mangling due to declaration in kiway.h.
MY_API( KIFACE* ) KIFACE_GETTER(  int* aKIFACEversion, int aKIWAYversion, PGM_BASE* aProgram )
{
    process = (PGM_BASE*) aProgram;
    return &kiface;
}


PGM_BASE& Pgm()
{
    wxASSERT( process );    // KIFACE_GETTER has already been called.
    return *process;
}




// A short lived implementation.  cvpcb will get combine into pcbnew shortly, so
// we skip setting KISYSMOD here for now.  User should set the environment
// variable.

bool IFACE::OnKifaceStart( PGM_BASE* aProgram, int aCtlBits )
{
    // This is process level, not project level, initialization of the DSO.

    // Do nothing in here pertinent to a project!

    start_common( aCtlBits );

    return true;
}

void IFACE::OnKifaceEnd()
{
    end_common();
}

wxWindow* IFACE::CreateWindow( wxWindow* aParent, int aClassId, KIWAY* aKiway, int aCtlBits  )
{
        switch( aClassId )
        {
            case FRAME_TEST:
                return new TEST_FRAME ( aKiway, aParent );
        }

        return NULL;
}
