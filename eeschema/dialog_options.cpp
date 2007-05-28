/////////////////////////////////////////////////////////////////////////////
// Name:        dilaog_options.cpp
// Purpose:     
// Author:      jean-pierre Charras
// Modified by: 
// Created:     31/01/2006 13:27:33
// RCS-ID:      
// Copyright:   GNU Licence
// Licence:     GNU
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 31/01/2006 13:27:33

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "dilaog_options.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"

#include "id.h"

#include "protos.h"

////@begin includes
////@end includes

#include "dialog_options.h"

////@begin XPM images
////@end XPM images



/**************************************************************************/
void DisplayOptionFrame(WinEDA_DrawFrame * parent, const wxPoint & framepos)
/**************************************************************************/
{
	WinEDA_SetOptionsFrame * frame =
			new WinEDA_SetOptionsFrame(parent);
	frame->ShowModal(); frame->Destroy();
}


/*!
 * WinEDA_SetOptionsFrame type definition
 */

IMPLEMENT_DYNAMIC_CLASS( WinEDA_SetOptionsFrame, wxDialog )

/*!
 * WinEDA_SetOptionsFrame event table definition
 */

BEGIN_EVENT_TABLE( WinEDA_SetOptionsFrame, wxDialog )

////@begin WinEDA_SetOptionsFrame event table entries
    EVT_BUTTON( wxID_OK, WinEDA_SetOptionsFrame::OnOkClick )

    EVT_BUTTON( wxID_CANCEL, WinEDA_SetOptionsFrame::OnCancelClick )

////@end WinEDA_SetOptionsFrame event table entries

END_EVENT_TABLE()

/*!
 * WinEDA_SetOptionsFrame constructors
 */

WinEDA_SetOptionsFrame::WinEDA_SetOptionsFrame( )
{
}

WinEDA_SetOptionsFrame::WinEDA_SetOptionsFrame( WinEDA_DrawFrame* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
	m_Parent = parent;
    Create(parent, id, caption, pos, size, style);
	
	/* Init options */
	if ( m_Parent->GetScreen() )
	{
		switch( m_Parent->GetScreen()->GetGrid().x )
		{
			case 50:
				m_SelGridSize->SetSelection(0);
				break;

			case 25:
				m_SelGridSize->SetSelection(1);
				break;

			case 10:
				m_SelGridSize->SetSelection(2);
				break;

			case 5:
				m_SelGridSize->SetSelection(3);
				break;

			case 2:
				m_SelGridSize->SetSelection(4);
				break;

			case 1:
				m_SelGridSize->SetSelection(5);
				break;

			default:
				DisplayError(this, wxT("WinEDA_SetOptionsFrame: Grid value not handle"));
				break;
		}
	}
	
	/* Adjust the current selections and options: */
	m_ShowGridOpt->SetValue(m_Parent->m_Draw_Grid);
	m_AutoPANOpt->SetValue(m_Parent->DrawPanel-> m_AutoPAN_Enable);
	m_SelShowPins->SetSelection( g_ShowAllPins ? TRUE : FALSE);
	m_Selunits->SetSelection( g_UnitMetric ? 0 : 1);
	m_SelDirWires->SetSelection( g_HVLines ? 0 : 1);
	m_Show_Page_Limits->SetSelection( g_ShowPageLimits ? 0 : 1);
wxString msg;
	msg = ReturnStringFromValue( g_UnitMetric,g_RepeatStep.x, m_Parent->m_InternalUnits);
	m_DeltaStepCtrl_X->SetValue( msg );
wxString title = _("Delta Step X") + ReturnUnitSymbol(g_UnitMetric);
    m_DeltaStepXTitle->SetLabel( title );

	msg = ReturnStringFromValue( g_UnitMetric,g_RepeatStep.y, m_Parent->m_InternalUnits);
	m_DeltaStepCtrl_Y->SetValue( msg );
	title = _("Delta Step Y") + ReturnUnitSymbol(g_UnitMetric);
    m_DeltaStepYTitle->SetLabel( title );

	m_DeltaLabelCtrl->SetValue( g_RepeatDeltaLabel );
	
}

/*!
 * WinEDA_SetOptionsFrame creator
 */

bool WinEDA_SetOptionsFrame::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin WinEDA_SetOptionsFrame member initialisation
    m_DrawOptionsSizer = NULL;
    m_ShowGridOpt = NULL;
    m_SelGridSize = NULL;
    m_SelShowPins = NULL;
    m_AutoPANOpt = NULL;
    m_Selunits = NULL;
    m_LabelSizeCtrlSizer = NULL;
    m_SelDirWires = NULL;
    m_Show_Page_Limits = NULL;
    m_DeltaStepXTitle = NULL;
    m_DeltaStepCtrl_X = NULL;
    m_DeltaStepYTitle = NULL;
    m_DeltaStepCtrl_Y = NULL;
    m_DeltaIncTitle = NULL;
    m_DeltaLabelCtrl = NULL;
////@end WinEDA_SetOptionsFrame member initialisation

////@begin WinEDA_SetOptionsFrame creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end WinEDA_SetOptionsFrame creation
    return true;
}

/*!
 * Control creation for WinEDA_SetOptionsFrame
 */

void WinEDA_SetOptionsFrame::CreateControls()
{    
	SetFont(*g_DialogFont);
////@begin WinEDA_SetOptionsFrame content construction
    // Generated by DialogBlocks, 23/02/2007 10:59:46 (unregistered)

    WinEDA_SetOptionsFrame* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Draw Options:"));
    m_DrawOptionsSizer = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    itemBoxSizer3->Add(m_DrawOptionsSizer, 0, wxGROW|wxALL, 5);

    m_ShowGridOpt = new wxCheckBox( itemDialog1, ID_CHECKBOX1, _("Show grid"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_ShowGridOpt->SetValue(false);
    m_DrawOptionsSizer->Add(m_ShowGridOpt, 0, wxALIGN_LEFT|wxALL, 5);

    wxString m_SelGridSizeStrings[] = {
        _("Normal (50 mils)"),
        _("Small (25 mils)"),
        _("Very small (10 mils)"),
        _("Special (5 mils)"),
        _("Special (2 mils)"),
        _("Special (1 mil)")
    };
    m_SelGridSize = new wxRadioBox( itemDialog1, ID_RADIOBOX, _("Grid Size"), wxDefaultPosition, wxDefaultSize, 6, m_SelGridSizeStrings, 1, wxRA_SPECIFY_COLS );
    m_SelGridSize->SetSelection(0);
    itemBoxSizer3->Add(m_SelGridSize, 0, wxGROW|wxALL, 5);

    wxString m_SelShowPinsStrings[] = {
        _("Normal"),
        _("Show alls")
    };
    m_SelShowPins = new wxRadioBox( itemDialog1, ID_RADIOBOX1, _("Show pins"), wxDefaultPosition, wxDefaultSize, 2, m_SelShowPinsStrings, 1, wxRA_SPECIFY_COLS );
    m_SelShowPins->SetSelection(0);
    itemBoxSizer3->Add(m_SelShowPins, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AutoPANOpt = new wxCheckBox( itemDialog1, ID_CHECKBOX, _("Auto PAN"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_AutoPANOpt->SetValue(false);
    m_AutoPANOpt->SetForegroundColour(wxColour(0, 0, 255));
    itemBoxSizer8->Add(m_AutoPANOpt, 0, wxGROW|wxALL, 5);

    wxString m_SelunitsStrings[] = {
        _("millimeter"),
        _("inches")
    };
    m_Selunits = new wxRadioBox( itemDialog1, ID_RADIOBOX2, _("Units"), wxDefaultPosition, wxDefaultSize, 2, m_SelunitsStrings, 1, wxRA_SPECIFY_COLS );
    m_Selunits->SetSelection(0);
    itemBoxSizer8->Add(m_Selunits, 0, wxGROW|wxALL, 5);

    m_LabelSizeCtrlSizer = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer8->Add(m_LabelSizeCtrlSizer, 0, wxGROW|wxALL, 5);

    wxString m_SelDirWiresStrings[] = {
        _("Horiz/Vertical"),
        _("Any")
    };
    m_SelDirWires = new wxRadioBox( itemDialog1, ID_RADIOBOX3, _("Wires - Bus orient"), wxDefaultPosition, wxDefaultSize, 2, m_SelDirWiresStrings, 1, wxRA_SPECIFY_COLS );
    m_SelDirWires->SetSelection(0);
    itemBoxSizer8->Add(m_SelDirWires, 0, wxGROW|wxALL, 5);

    wxString m_Show_Page_LimitsStrings[] = {
        _("Yes"),
        _("No")
    };
    m_Show_Page_Limits = new wxRadioBox( itemDialog1, ID_RADIOBOX4, _("Show page limits"), wxDefaultPosition, wxDefaultSize, 2, m_Show_Page_LimitsStrings, 1, wxRA_SPECIFY_COLS );
    m_Show_Page_Limits->SetSelection(0);
    itemBoxSizer8->Add(m_Show_Page_Limits, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer14 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton15 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton15->SetForegroundColour(wxColour(202, 0, 0));
    itemBoxSizer14->Add(itemButton15, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton16 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton16->SetForegroundColour(wxColour(0, 0, 255));
    itemBoxSizer14->Add(itemButton16, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    itemBoxSizer14->Add(5, 5, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer18Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Auto increment params"));
    wxStaticBoxSizer* itemStaticBoxSizer18 = new wxStaticBoxSizer(itemStaticBoxSizer18Static, wxVERTICAL);
    itemBoxSizer14->Add(itemStaticBoxSizer18, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_DeltaStepXTitle = new wxStaticText( itemDialog1, wxID_STATIC, _("Delta Step X"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer18->Add(m_DeltaStepXTitle, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_DeltaStepCtrl_X = new wxTextCtrl( itemDialog1, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer18->Add(m_DeltaStepCtrl_X, 0, wxGROW|wxALL, 5);

    m_DeltaStepYTitle = new wxStaticText( itemDialog1, wxID_STATIC, _("Delta Step Y"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer18->Add(m_DeltaStepYTitle, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_DeltaStepCtrl_Y = new wxTextCtrl( itemDialog1, ID_TEXTCTRL1, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer18->Add(m_DeltaStepCtrl_Y, 0, wxGROW|wxALL, 5);

    m_DeltaIncTitle = new wxStaticText( itemDialog1, wxID_STATIC, _("Delta Label:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer18->Add(m_DeltaIncTitle, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_DeltaLabelCtrl = new wxSpinCtrl( itemDialog1, ID_SPINCTRL, _T("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -16, 16, 0 );
    itemStaticBoxSizer18->Add(m_DeltaLabelCtrl, 0, wxGROW|wxALL, 5);

////@end WinEDA_SetOptionsFrame content construction

	m_DefaultDrawLineWidthCtrl = new WinEDA_ValueCtrl(this, _("Default Line Width"),g_DrawMinimunLineWidth,
			g_UnitMetric, m_DrawOptionsSizer, EESCHEMA_INTERNAL_UNIT);

	m_DefaultLabelSizeCtrl = new WinEDA_ValueCtrl(this, _("Default Label Size"),g_DefaultTextLabelSize,
			g_UnitMetric, m_LabelSizeCtrlSizer, EESCHEMA_INTERNAL_UNIT);

}

/*!
 * Should we show tooltips?
 */

bool WinEDA_SetOptionsFrame::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap WinEDA_SetOptionsFrame::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin WinEDA_SetOptionsFrame bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end WinEDA_SetOptionsFrame bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon WinEDA_SetOptionsFrame::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin WinEDA_SetOptionsFrame icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end WinEDA_SetOptionsFrame icon retrieval
}
/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void WinEDA_SetOptionsFrame::OnOkClick( wxCommandEvent& event )
{
	Accept(event);
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in WinEDA_SetOptionsFrame.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK in WinEDA_SetOptionsFrame. 
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void WinEDA_SetOptionsFrame::OnCancelClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in WinEDA_SetOptionsFrame.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in WinEDA_SetOptionsFrame. 
}


/**************************************************************************/
void WinEDA_SetOptionsFrame::Accept(wxCommandEvent& event)
/**************************************************************************/
{
wxSize grid;
bool setgrid = TRUE;
wxString msg;
	
	g_DrawMinimunLineWidth = m_DefaultDrawLineWidthCtrl->GetValue();
	if ( g_DrawMinimunLineWidth < 0 ) g_DrawMinimunLineWidth = 0;
	if ( g_DrawMinimunLineWidth > 100 ) g_DrawMinimunLineWidth = 100;
		
	g_DefaultTextLabelSize = m_DefaultLabelSizeCtrl->GetValue();
	if ( g_DefaultTextLabelSize < 0 ) g_DefaultTextLabelSize = 0;
	if ( g_DefaultTextLabelSize > 1000 ) g_DefaultTextLabelSize = 1000;

	msg = m_DeltaStepCtrl_X->GetValue();
	g_RepeatStep.x = 
		ReturnValueFromString( g_UnitMetric, msg, m_Parent->m_InternalUnits);
	msg = m_DeltaStepCtrl_Y->GetValue();
	g_RepeatStep.y =
		ReturnValueFromString( g_UnitMetric, msg, m_Parent->m_InternalUnits);

	g_RepeatDeltaLabel = m_DeltaLabelCtrl->GetValue();

	if ( m_Show_Page_Limits->GetSelection() == 0 ) g_ShowPageLimits = TRUE;
	else g_ShowPageLimits = FALSE;

	if ( m_SelDirWires->GetSelection() == 0 ) g_HVLines = 1;
	else g_HVLines = 0;

	if ( m_Selunits->GetSelection() == 0 ) g_UnitMetric = 1;
	else  g_UnitMetric = 0;

	if ( m_SelShowPins->GetSelection() == 0 ) g_ShowAllPins = FALSE;
	else g_ShowAllPins = TRUE;

	g_ShowGrid = m_Parent->m_Draw_Grid = m_ShowGridOpt->GetValue();
	m_Parent->DrawPanel->m_AutoPAN_Enable = m_AutoPANOpt->GetValue();

	m_Parent->m_Draw_Grid = m_ShowGridOpt->GetValue();
	switch( m_SelGridSize->GetSelection() )
		{
		default:
			setgrid = FALSE;
			break;
		
		case 0:
			grid = wxSize(50,50);
			break;
		case 1:
			grid = wxSize(25,25);
			break;

		case 2:
			grid = wxSize(10,10);
			break;
		}

	if ( m_Parent->m_CurrentScreen )
	{
		if ( setgrid ) m_Parent->m_CurrentScreen->SetGrid(grid);
		m_Parent->m_CurrentScreen->SetRefreshReq();
	}
}

