	/****************************************************************/
	/* dialog_display_options.cpp - Gestion des Options d'affichage */
	/****************************************************************/
/*
 Affichage et modifications des parametres de travail de PcbNew
 Parametres = mode d'affichage des textes et elements graphiques
*/

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "dialog_display_options.h"
#endif


#include "fctsys.h"
#include "gr_basic.h"
#include "macros.h"

#include "common.h"
#include "pcbnew.h"
#include "pcbplot.h"
#include "autorout.h"

#include "id.h"

#include "protos.h"
#include <wx/spinctrl.h>

#include "dialog_display_options.h"

/* Fonctions locales */

/* variables locales */


/*************************************************************************/
void WinEDA_DisplayOptionsDialog::AcceptPcbOptions(wxCommandEvent& event)
/*************************************************************************/
/* Met a jour les options
*/
{
	if ( m_Show_Page_Limits->GetSelection() == 0 ) g_ShowPageLimits = TRUE;
	else g_ShowPageLimits = FALSE;

	if ( m_OptDisplayTracks->GetSelection() == 1)
		DisplayOpt.DisplayPcbTrackFill = TRUE;
	else DisplayOpt.DisplayPcbTrackFill = FALSE;

	m_Parent->m_DisplayPcbTrackFill = DisplayOpt.DisplayPcbTrackFill;
	DisplayOpt.m_DisplayViaMode = m_OptDisplayViaHole->GetSelection();

	switch ( m_OptDisplayTracksClearance->GetSelection() )
	{
		case 0:
			DisplayOpt.DisplayTrackIsol = TRUE;
			g_ShowIsolDuringCreateTrack = TRUE;
			break;
		case 1:
			DisplayOpt.DisplayTrackIsol = FALSE;
			g_ShowIsolDuringCreateTrack = TRUE;
			break;
		case 2:
			DisplayOpt.DisplayTrackIsol = FALSE;
			g_ShowIsolDuringCreateTrack = FALSE;
			break;
	}

	m_Parent->m_DisplayModText = DisplayOpt.DisplayModText =
			m_OptDisplayModTexts->GetSelection();
	m_Parent->m_DisplayModEdge = DisplayOpt.DisplayModEdge =
			m_OptDisplayModEdges->GetSelection();

	if (m_OptDisplayPads->GetSelection() == 1 )
		 DisplayOpt.DisplayPadFill = TRUE;
	else DisplayOpt.DisplayPadFill = FALSE;

	m_Parent->m_DisplayPadFill = DisplayOpt.DisplayPadFill;

	DisplayOpt.DisplayPadIsol = m_OptDisplayPadClearence->GetValue();

	m_Parent->m_DisplayPadNum = DisplayOpt.DisplayPadNum = m_OptDisplayPadNumber->GetValue();

	DisplayOpt.DisplayPadNoConn = m_OptDisplayPadNoConn->GetValue();

	DisplayOpt.DisplayDrawItems = m_OptDisplayDrawings->GetSelection();

	m_Parent->DrawPanel->Refresh(TRUE);

	EndModal(1);
}


/*!
 * WinEDA_DisplayOptionsDialog type definition
 */

IMPLEMENT_DYNAMIC_CLASS( WinEDA_DisplayOptionsDialog, wxDialog )

/*!
 * WinEDA_DisplayOptionsDialog event table definition
 */

BEGIN_EVENT_TABLE( WinEDA_DisplayOptionsDialog, wxDialog )

////@begin WinEDA_DisplayOptionsDialog event table entries
    EVT_BUTTON( wxID_OK, WinEDA_DisplayOptionsDialog::OnOkClick )

    EVT_BUTTON( wxID_CANCEL, WinEDA_DisplayOptionsDialog::OnCancelClick )

////@end WinEDA_DisplayOptionsDialog event table entries

END_EVENT_TABLE()

/*!
 * WinEDA_DisplayOptionsDialog constructors
 */

WinEDA_DisplayOptionsDialog::WinEDA_DisplayOptionsDialog( )
{
}

WinEDA_DisplayOptionsDialog::WinEDA_DisplayOptionsDialog( WinEDA_BasePcbFrame* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
	m_Parent = parent;

    Create(parent, id, caption, pos, size, style);

    if ( DisplayOpt.DisplayPcbTrackFill )
		m_OptDisplayTracks->SetSelection(1);
 	if ( DisplayOpt.DisplayTrackIsol )
		m_OptDisplayTracksClearance->SetSelection(0);
	else if ( g_ShowIsolDuringCreateTrack )
		m_OptDisplayTracksClearance->SetSelection(1);
	else m_OptDisplayTracksClearance->SetSelection(2);

	if ( DisplayOpt.DisplayPadFill )
		m_OptDisplayPads->SetSelection(1);

	m_Show_Page_Limits->SetSelection( g_ShowPageLimits ? 0 : 1);
}

/*!
 * WinEDA_DisplayOptionsDialog creator
 */

bool WinEDA_DisplayOptionsDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin WinEDA_DisplayOptionsDialog member initialisation
    m_OptDisplayTracks = NULL;
    m_OptDisplayTracksClearance = NULL;
    m_OptDisplayViaHole = NULL;
    m_OptDisplayModTexts = NULL;
    m_OptDisplayModEdges = NULL;
    m_OptDisplayPads = NULL;
    m_OptDisplayPadClearence = NULL;
    m_OptDisplayPadNumber = NULL;
    m_OptDisplayPadNoConn = NULL;
    m_OptDisplayDrawings = NULL;
    m_Show_Page_Limits = NULL;
////@end WinEDA_DisplayOptionsDialog member initialisation

////@begin WinEDA_DisplayOptionsDialog creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end WinEDA_DisplayOptionsDialog creation
    return true;
}

/*!
 * Control creation for WinEDA_DisplayOptionsDialog
 */

void WinEDA_DisplayOptionsDialog::CreateControls()
{
	SetFont(*g_DialogFont);
////@begin WinEDA_DisplayOptionsDialog content construction
    // Generated by DialogBlocks, 26/02/2006 15:47:05 (unregistered)

    WinEDA_DisplayOptionsDialog* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Tracks and vias"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxString m_OptDisplayTracksStrings[] = {
        _("Sketch"),
        _("Filled")
    };
    m_OptDisplayTracks = new wxRadioBox( itemDialog1, ID_RADIOBOX_OPT_TRACK, _("Tracks:"), wxDefaultPosition, wxDefaultSize, 2, m_OptDisplayTracksStrings, 1, wxRA_SPECIFY_COLS );
    itemStaticBoxSizer3->Add(m_OptDisplayTracks, 0, wxGROW|wxALL, 5);

    wxString m_OptDisplayTracksClearanceStrings[] = {
        _("Always"),
        _("New track"),
        _("Never")
    };
    m_OptDisplayTracksClearance = new wxRadioBox( itemDialog1, ID_RADIOBOX_SHOWCLR, _("Show Track Clearance"), wxDefaultPosition, wxDefaultSize, 3, m_OptDisplayTracksClearanceStrings, 1, wxRA_SPECIFY_COLS );
    itemStaticBoxSizer3->Add(m_OptDisplayTracksClearance, 0, wxGROW|wxALL, 5);

    wxString m_OptDisplayViaHoleStrings[] = {
        _("Never"),
        _("defined holes"),
        _("Always")
    };
    m_OptDisplayViaHole = new wxRadioBox( itemDialog1, ID_RADIOBOX_SHOW_VIAS, _("Show Via Holes"), wxDefaultPosition, wxDefaultSize, 3, m_OptDisplayViaHoleStrings, 1, wxRA_SPECIFY_COLS );
    itemStaticBoxSizer3->Add(m_OptDisplayViaHole, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer7Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Modules"));
    wxStaticBoxSizer* itemStaticBoxSizer7 = new wxStaticBoxSizer(itemStaticBoxSizer7Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer7, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxVERTICAL);
    itemStaticBoxSizer7->Add(itemBoxSizer8, 0, wxGROW|wxALL, 5);

    wxString m_OptDisplayModTextsStrings[] = {
        _("Line"),
        _("Filled"),
        _("Sketch")
    };
    m_OptDisplayModTexts = new wxRadioBox( itemDialog1, ID_RADIOBOX_MODTXT, _("Module Texts"), wxDefaultPosition, wxDefaultSize, 3, m_OptDisplayModTextsStrings, 1, wxRA_SPECIFY_COLS );
    itemBoxSizer8->Add(m_OptDisplayModTexts, 0, wxGROW|wxALL, 5);

    wxString m_OptDisplayModEdgesStrings[] = {
        _("Line"),
        _("Filled"),
        _("Sketch")
    };
    m_OptDisplayModEdges = new wxRadioBox( itemDialog1, ID_RADIOBOX_MOD_EDGES, _("Module Edges:"), wxDefaultPosition, wxDefaultSize, 3, m_OptDisplayModEdgesStrings, 1, wxRA_SPECIFY_COLS );
    itemBoxSizer8->Add(m_OptDisplayModEdges, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxVERTICAL);
    itemStaticBoxSizer7->Add(itemBoxSizer11, 0, wxGROW|wxALL, 5);

    wxString m_OptDisplayPadsStrings[] = {
        _("Sketch"),
        _("Filled")
    };
    m_OptDisplayPads = new wxRadioBox( itemDialog1, ID_RADIOBOX_SHOWPADS, _("Pads:"), wxDefaultPosition, wxDefaultSize, 2, m_OptDisplayPadsStrings, 1, wxRA_SPECIFY_COLS );
    itemBoxSizer11->Add(m_OptDisplayPads, 0, wxGROW|wxALL, 5);

    m_OptDisplayPadClearence = new wxCheckBox( itemDialog1, ID_CHECKBOX_PAD_CLR, _("Show Pad Clearance"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_OptDisplayPadClearence->SetValue(false);
    itemBoxSizer11->Add(m_OptDisplayPadClearence, 0, wxGROW|wxALL, 5);

    m_OptDisplayPadNumber = new wxCheckBox( itemDialog1, ID_CHECKBOX_PADNUM, _("Show Pad Number"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_OptDisplayPadNumber->SetValue(false);
    itemBoxSizer11->Add(m_OptDisplayPadNumber, 0, wxGROW|wxALL, 5);

    m_OptDisplayPadNoConn = new wxCheckBox( itemDialog1, ID_CHECKBOX_PADNC, _("Show Pad NoConnect"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_OptDisplayPadNoConn->SetValue(false);
    itemBoxSizer11->Add(m_OptDisplayPadNoConn, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer16, 0, wxGROW|wxALL, 5);

    wxString m_OptDisplayDrawingsStrings[] = {
        _("Line"),
        _("Filled"),
        _("Sketch")
    };
    m_OptDisplayDrawings = new wxRadioBox( itemDialog1, ID_RADIOBOX_SHOW_OTHERS, _("Display other items:"), wxDefaultPosition, wxDefaultSize, 3, m_OptDisplayDrawingsStrings, 1, wxRA_SPECIFY_COLS );
    itemBoxSizer16->Add(m_OptDisplayDrawings, 0, wxGROW|wxALL, 5);

    wxString m_Show_Page_LimitsStrings[] = {
        _("Yes"),
        _("No")
    };
    m_Show_Page_Limits = new wxRadioBox( itemDialog1, ID_RADIOBOX_PAGE_LIMITS, _("Show page limits"), wxDefaultPosition, wxDefaultSize, 2, m_Show_Page_LimitsStrings, 1, wxRA_SPECIFY_COLS );
    itemBoxSizer16->Add(m_Show_Page_Limits, 0, wxGROW|wxALL, 5);

    itemBoxSizer16->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton20 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton20->SetForegroundColour(wxColour(202, 0, 0));
    itemBoxSizer16->Add(itemButton20, 0, wxGROW|wxALL, 5);

    wxButton* itemButton21 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton21->SetForegroundColour(wxColour(0, 0, 255));
    itemBoxSizer16->Add(itemButton21, 0, wxGROW|wxALL, 5);

    // Set validators
    m_OptDisplayViaHole->SetValidator( wxGenericValidator(& DisplayOpt.m_DisplayViaMode) );
    m_OptDisplayModTexts->SetValidator( wxGenericValidator(& DisplayOpt.DisplayModText) );
    m_OptDisplayModEdges->SetValidator( wxGenericValidator(& DisplayOpt.DisplayModEdge) );
    m_OptDisplayPadClearence->SetValidator( wxGenericValidator(& DisplayOpt.DisplayPadIsol) );
    m_OptDisplayPadNumber->SetValidator( wxGenericValidator(& DisplayOpt.DisplayPadNum) );
    m_OptDisplayPadNoConn->SetValidator( wxGenericValidator(& DisplayOpt.DisplayPadNoConn) );
    m_OptDisplayDrawings->SetValidator( wxGenericValidator(& DisplayOpt.DisplayDrawItems) );
////@end WinEDA_DisplayOptionsDialog content construction
}

/*!
 * Should we show tooltips?
 */

bool WinEDA_DisplayOptionsDialog::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap WinEDA_DisplayOptionsDialog::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin WinEDA_DisplayOptionsDialog bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end WinEDA_DisplayOptionsDialog bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon WinEDA_DisplayOptionsDialog::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin WinEDA_DisplayOptionsDialog icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end WinEDA_DisplayOptionsDialog icon retrieval
}
/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void WinEDA_DisplayOptionsDialog::OnOkClick( wxCommandEvent& event )
{
	AcceptPcbOptions(event);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void WinEDA_DisplayOptionsDialog::OnCancelClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in WinEDA_DisplayOptionsDialog.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in WinEDA_DisplayOptionsDialog.
}


