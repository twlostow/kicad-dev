///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "dialog_find_filter.h"

///////////////////////////////////////////////////////////////////////////

DIALOG_FIND_FILTER::DIALOG_FIND_FILTER( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText1 = new wxStaticText( this, wxID_ANY, wxT("Find and Filter:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	bSizer1->Add( m_staticText1, 0, wxALL, 5 );
	
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_btnAll = new wxButton( this, wxID_ANY, wxT("All"), wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_btnAll->SetMaxSize( wxSize( 50,-1 ) );
	
	fgSizer2->Add( m_btnAll, 0, wxALL, 5 );
	
	m_btnNone = new wxButton( this, wxID_ANY, wxT("None"), wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_btnNone->SetMaxSize( wxSize( 50,-1 ) );
	
	fgSizer2->Add( m_btnNone, 0, wxALL, 5 );
	
	
	bSizer1->Add( fgSizer2, 0, wxEXPAND, 5 );
	
	m_chkTracks = new wxCheckBox( this, wxID_ANY, wxT("Tracks"), wxDefaultPosition, wxDefaultSize, 0 );
	m_chkTracks->SetValue(true); 
	bSizer1->Add( m_chkTracks, 0, wxALL, 5 );
	
	m_chkVias = new wxCheckBox( this, wxID_ANY, wxT("Vias"), wxDefaultPosition, wxDefaultSize, 0 );
	m_chkVias->SetValue(true); 
	bSizer1->Add( m_chkVias, 0, wxALL, 5 );
	
	m_chkPads = new wxCheckBox( this, wxID_ANY, wxT("Pads"), wxDefaultPosition, wxDefaultSize, 0 );
	m_chkPads->SetValue(true); 
	bSizer1->Add( m_chkPads, 0, wxALL, 5 );
	
	m_chkComps = new wxCheckBox( this, wxID_ANY, wxT("Components"), wxDefaultPosition, wxDefaultSize, 0 );
	m_chkComps->SetValue(true); 
	bSizer1->Add( m_chkComps, 0, wxALL, 5 );
	
	m_chkCompText = new wxCheckBox( this, wxID_ANY, wxT("Component Text"), wxDefaultPosition, wxDefaultSize, 0 );
	m_chkCompText->SetValue(true); 
	bSizer1->Add( m_chkCompText, 0, wxALL, 5 );
	
	m_chkZones = new wxCheckBox( this, wxID_ANY, wxT("Zones"), wxDefaultPosition, wxDefaultSize, 0 );
	m_chkZones->SetValue(true); 
	bSizer1->Add( m_chkZones, 0, wxALL, 5 );
	
	m_chkGraphics = new wxCheckBox( this, wxID_ANY, wxT("Graphics"), wxDefaultPosition, wxDefaultSize, 0 );
	m_chkGraphics->SetValue(true); 
	bSizer1->Add( m_chkGraphics, 0, wxALL, 5 );
	
	m_chkMech = new wxCheckBox( this, wxID_ANY, wxT("Mechanical"), wxDefaultPosition, wxDefaultSize, 0 );
	m_chkMech->SetValue(true); 
	bSizer1->Add( m_chkMech, 0, wxALL, 5 );
	
	m_chkDrc = new wxCheckBox( this, wxID_ANY, wxT("DRC Markers"), wxDefaultPosition, wxDefaultSize, 0 );
	m_chkDrc->SetValue(true); 
	bSizer1->Add( m_chkDrc, 0, wxALL, 5 );
	
	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer3->AddGrowableCol( 1 );
	fgSizer3->SetFlexibleDirection( wxBOTH );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_chkNets = new wxCheckBox( this, wxID_ANY, wxT("Nets:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_chkNets->SetValue(true); 
	fgSizer3->Add( m_chkNets, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_nets = new wxTextCtrl( this, wxID_ANY, wxT("NetA"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer3->Add( m_nets, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_chkText = new wxCheckBox( this, wxID_ANY, wxT("Text:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_chkText->SetValue(true); 
	fgSizer3->Add( m_chkText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_text = new wxTextCtrl( this, wxID_ANY, wxT("C1"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer3->Add( m_text, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bSizer1->Add( fgSizer3, 0, wxEXPAND, 5 );
	
	m_staticline4 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer1->Add( m_staticline4, 0, wxEXPAND | wxALL, 5 );
	
	m_chkIncludeLocked = new wxCheckBox( this, wxID_ANY, wxT("Include locked items"), wxDefaultPosition, wxDefaultSize, 0 );
	m_chkIncludeLocked->SetValue(true); 
	bSizer1->Add( m_chkIncludeLocked, 0, wxALL, 5 );
	
	m_chkMaskEnabled = new wxCheckBox( this, wxID_ANY, wxT("Hide filtered items"), wxDefaultPosition, wxDefaultSize, 0 );
	m_chkMaskEnabled->SetValue(true); 
	bSizer1->Add( m_chkMaskEnabled, 0, wxALL, 5 );
	
	wxGridSizer* gSizer1;
	gSizer1 = new wxGridSizer( 1, 2, 0, 0 );
	
	m_btnFindNext = new wxButton( this, wxID_ANY, wxT("Find/Next"), wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_btnFindNext->SetMaxSize( wxSize( 50,-1 ) );
	
	gSizer1->Add( m_btnFindNext, 1, wxALL|wxEXPAND, 5 );
	
	m_btnBack = new wxButton( this, wxID_ANY, wxT("Back"), wxDefaultPosition, wxSize( 50,-1 ), 0 );
	m_btnBack->SetMaxSize( wxSize( 50,-1 ) );
	
	gSizer1->Add( m_btnBack, 1, wxALL|wxEXPAND, 5 );
	
	
	bSizer1->Add( gSizer1, 0, wxEXPAND, 5 );
	
	m_staticText2 = new wxStaticText( this, wxID_ANY, wxT("XXX matching items."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	bSizer1->Add( m_staticText2, 0, wxALL, 5 );
	
	
	this->SetSizer( bSizer1 );
	this->Layout();
}

DIALOG_FIND_FILTER::~DIALOG_FIND_FILTER()
{
}
