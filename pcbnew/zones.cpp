/////////////////////////////////////////////////////////////////////////////
// Name:        zones.cpp
// Purpose:
// Author:      jean-pierre Charras
// Modified by:
// Created:     25/01/2006 11:35:19
// RCS-ID:
// Copyright:   GNU License
// Licence:     GNU License
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 25/01/2006 11:35:19

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "zones.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "zones.h"

////@begin XPM images
////@end XPM images

/* Routines Locales */
static void Display_Zone_Netname(WinEDA_PcbFrame *frame);
static void Exit_Zones(WinEDA_DrawPanel * Panel, wxDC *DC);
static void Show_Zone_Edge_While_MoveMouse(WinEDA_DrawPanel * panel, wxDC * DC, bool erase);
static void Genere_Segments_Zone(WinEDA_PcbFrame *frame, wxDC * DC, int net_code);
static bool Genere_Pad_Connexion(WinEDA_PcbFrame *frame, wxDC * DC, int layer);

/* Variables locales */
static bool Zone_Debug = FALSE;
static bool Zone_45_Only = FALSE;
static bool Zone_Exclude_Pads = TRUE;
static bool Zone_Genere_Freins_Thermiques = TRUE;

static int TimeStamp;			/* signature temporelle pour la zone generee */

/*!
 * WinEDA_ZoneFrame type definition
 */

IMPLEMENT_DYNAMIC_CLASS( WinEDA_ZoneFrame, wxDialog )

/*!
 * WinEDA_ZoneFrame event table definition
 */

BEGIN_EVENT_TABLE( WinEDA_ZoneFrame, wxDialog )

////@begin WinEDA_ZoneFrame event table entries
    EVT_BUTTON( ID_FILL_ZONE, WinEDA_ZoneFrame::ExecFillZone )

    EVT_BUTTON( wxID_CANCEL, WinEDA_ZoneFrame::OnCancelClick )

    EVT_BUTTON( ID_SET_OPTIONS_ZONE, WinEDA_ZoneFrame::ExecFillZone )

////@end WinEDA_ZoneFrame event table entries

END_EVENT_TABLE()

/*!
 * WinEDA_ZoneFrame constructors
 */

WinEDA_ZoneFrame::WinEDA_ZoneFrame( )
{
}

WinEDA_ZoneFrame::WinEDA_ZoneFrame( WinEDA_PcbFrame* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
	m_Parent = parent;
    Create(parent, id, caption, pos, size, style);
}

/*!
 * WinEDA_ZoneFrame creator
 */

bool WinEDA_ZoneFrame::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin WinEDA_ZoneFrame member initialisation
    m_GridCtrl = NULL;
    m_ClearanceValueTitle = NULL;
    m_ZoneClearanceCtrl = NULL;
    m_FillOpt = NULL;
    m_OrientEdgesOpt = NULL;
////@end WinEDA_ZoneFrame member initialisation

////@begin WinEDA_ZoneFrame creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
////@end WinEDA_ZoneFrame creation

	return true;
}

/*!
 * Control creation for WinEDA_ZoneFrame
 */

void WinEDA_ZoneFrame::CreateControls()
{
	SetFont(*g_DialogFont);

////@begin WinEDA_ZoneFrame content construction
    // Generated by DialogBlocks, 03/03/2006 13:36:21 (unregistered)

    WinEDA_ZoneFrame* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 5);

    wxString m_GridCtrlStrings[] = {
        _("0.00000"),
        _("0.00000"),
        _("0.00000"),
        _("0.00000")
    };
    m_GridCtrl = new wxRadioBox( itemDialog1, ID_RADIOBOX, _("Grid size:"), wxDefaultPosition, wxDefaultSize, 4, m_GridCtrlStrings, 1, wxRA_SPECIFY_COLS );
    itemBoxSizer3->Add(m_GridCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_ClearanceValueTitle = new wxStaticText( itemDialog1, wxID_STATIC, _("Zone clearance value (mm):"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_ClearanceValueTitle, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_ZoneClearanceCtrl = new wxTextCtrl( itemDialog1, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_ZoneClearanceCtrl, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    itemBoxSizer2->Add(5, 5, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer8, 0, wxGROW|wxALL, 5);

    wxString m_FillOptStrings[] = {
        _("Include Pads"),
        _("Thermal"),
        _("Exclude Pads")
    };
    m_FillOpt = new wxRadioBox( itemDialog1, ID_RADIOBOX1, _("Pad options:"), wxDefaultPosition, wxDefaultSize, 3, m_FillOptStrings, 1, wxRA_SPECIFY_COLS );
    itemBoxSizer8->Add(m_FillOpt, 0, wxALIGN_LEFT|wxALL, 5);

    wxString m_OrientEdgesOptStrings[] = {
        _("Any"),
        _("H , V and 45 deg")
    };
    m_OrientEdgesOpt = new wxRadioBox( itemDialog1, ID_RADIOBOX2, _("Zone edges orient:"), wxDefaultPosition, wxDefaultSize, 2, m_OrientEdgesOptStrings, 1, wxRA_SPECIFY_COLS );
    itemBoxSizer8->Add(m_OrientEdgesOpt, 0, wxALIGN_RIGHT|wxALL, 5);

    itemBoxSizer2->Add(5, 5, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_TOP|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemDialog1, ID_FILL_ZONE, _("Fill"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton13->SetDefault();
    itemButton13->SetForegroundColour(wxColour(204, 0, 0));
    itemBoxSizer12->Add(itemButton13, 0, wxGROW|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton14->SetForegroundColour(wxColour(0, 0, 255));
    itemBoxSizer12->Add(itemButton14, 0, wxGROW|wxALL, 5);

    wxButton* itemButton15 = new wxButton( itemDialog1, ID_SET_OPTIONS_ZONE, _("Update Options"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton15->SetForegroundColour(wxColour(0, 100, 0));
    itemBoxSizer12->Add(itemButton15, 0, wxGROW|wxALL, 5);

    itemBoxSizer2->Add(5, 5, 0, wxGROW|wxALL, 5);

////@end WinEDA_ZoneFrame content construction
wxString title = _("Zone clearance value:") + ReturnUnitSymbol(g_UnitMetric);
    m_ClearanceValueTitle->SetLabel( title );

	title = _("Grid :") + ReturnUnitSymbol(g_UnitMetric);;
    m_GridCtrl->SetLabel(title);

	if ( g_DesignSettings.m_ZoneClearence == 0 )
		g_DesignSettings.m_ZoneClearence = g_DesignSettings.m_TrackClearence;
	title = ReturnStringFromValue( g_UnitMetric, g_DesignSettings.m_ZoneClearence, m_Parent->m_InternalUnits);
	m_ZoneClearanceCtrl->SetValue( title );

	if ( Zone_45_Only ) m_OrientEdgesOpt->SetSelection(1);

int GridList[4] = { 50,100,250,500}, selection = 0;
	for ( unsigned int ii = 0; ii < m_GridCtrl->GetCount(); ii++ )
	{
		wxString msg = ReturnStringFromValue(g_UnitMetric, GridList[ii], m_Parent->m_InternalUnits);
		m_GridCtrl->SetString(ii,msg);
		if ( g_GridRoutingSize == GridList[ii] ) selection = ii;
	}
	m_GridCtrl->SetSelection(selection);

 	if ( Zone_Exclude_Pads)
	{
		if ( Zone_Genere_Freins_Thermiques ) m_FillOpt->SetSelection(1);
		else m_FillOpt->SetSelection(2);
	}

}

/*!
 * Should we show tooltips?
 */

bool WinEDA_ZoneFrame::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap WinEDA_ZoneFrame::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin WinEDA_ZoneFrame bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end WinEDA_ZoneFrame bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon WinEDA_ZoneFrame::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin WinEDA_ZoneFrame icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end WinEDA_ZoneFrame icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void WinEDA_ZoneFrame::OnCancelClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in WinEDA_ZoneFrame.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in WinEDA_ZoneFrame.
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2
 */
/***********************************************************/
void WinEDA_ZoneFrame::ExecFillZone( wxCommandEvent & event)
/***********************************************************/
{
	switch ( m_FillOpt->GetSelection() )
		{
		case 0:
			Zone_Exclude_Pads = FALSE;
			Zone_Genere_Freins_Thermiques = FALSE;
			break;

		case 1:
			Zone_Exclude_Pads = TRUE;
			Zone_Genere_Freins_Thermiques = TRUE;
			break;

		case 2:
			Zone_Exclude_Pads = TRUE;
			Zone_Genere_Freins_Thermiques = FALSE;
			break;
		}

	switch ( m_GridCtrl->GetSelection() )
		{
		case 0:
			g_GridRoutingSize = 50;
			break;

		case 1:
			g_GridRoutingSize = 100;
			break;

		case 2:
			g_GridRoutingSize = 250;
			break;

		case 3:
			g_GridRoutingSize = 500;
			break;
		}

	wxString txtvalue = m_ZoneClearanceCtrl->GetValue();
	g_DesignSettings.m_ZoneClearence =
		ReturnValueFromString( g_UnitMetric, txtvalue, m_Parent->m_InternalUnits);
	if ( m_OrientEdgesOpt->GetSelection() == 0) Zone_45_Only = FALSE;
	else Zone_45_Only = TRUE;

	if ( event.GetId() == ID_SET_OPTIONS_ZONE ) EndModal(1);
	else EndModal(0);
}

/**************************************************************/
void WinEDA_PcbFrame::Edit_Zone_Width(wxDC * DC, SEGZONE * Zone)
/**************************************************************/
/* Edite (change la largeur des segments) la zone Zone.
	La zone est constituee des segments zones de meme TimeStamp
*/
{
SEGZONE * pt_segm, * NextS ;
int TimeStamp;
bool modify = FALSE;
double f_new_width;
int w_tmp;
wxString Line;
wxString Msg( _("New zone segment width: ") );

	if ( Zone == NULL ) return;

	f_new_width = To_User_Unit(g_UnitMetric, Zone->m_Width, GetScreen()->GetInternalUnits());
	Line.Printf(wxT("%.4f"), f_new_width);
	Msg += g_UnitMetric ? wxT("(mm)") : wxT("(\")");
	if ( Get_Message(Msg, Line, this) != 0 ) return;

	w_tmp = g_DesignSettings.m_CurrentTrackWidth;
	Line.ToDouble( &f_new_width);
	g_DesignSettings.m_CurrentTrackWidth = From_User_Unit(g_UnitMetric, f_new_width, GetScreen()->GetInternalUnits());

	TimeStamp = Zone->m_TimeStamp;

	for( pt_segm = (SEGZONE*)m_Pcb->m_Zone; pt_segm != NULL; pt_segm = NextS)
	{
		NextS = (SEGZONE*) pt_segm->Pnext;
		if(pt_segm->m_TimeStamp == TimeStamp)
		{
			modify = TRUE;
			Edit_TrackSegm_Width(DC, pt_segm);
		}
	}

	g_DesignSettings.m_CurrentTrackWidth = w_tmp;
	if ( modify )
	{
		GetScreen()->SetModify();
		DrawPanel->Refresh();
	}
}



/**********************************************************/
void WinEDA_PcbFrame::Delete_Zone(wxDC * DC, SEGZONE * Zone)
/**********************************************************/
/* Efface la zone Zone.
	La zone est constituee des segments zones de meme TimeStamp
*/
{
SEGZONE * pt_segm, * NextS ;
int TimeStamp;
int nb_segm = 0;
bool modify = FALSE;

	TimeStamp = Zone->m_TimeStamp;

	for( pt_segm = (SEGZONE*)m_Pcb->m_Zone; pt_segm != NULL; pt_segm = NextS)
	{
		NextS = (SEGZONE*) pt_segm->Pnext;
		if(pt_segm->m_TimeStamp == TimeStamp)
		{
			modify = TRUE;
			/* effacement des segments a l'ecran */
			Trace_Une_Piste(DrawPanel, DC, pt_segm, nb_segm, GR_XOR);
			DeleteStructure(pt_segm);
		}
	}

	if ( modify )
	{
		GetScreen()->SetModify();
		GetScreen()->SetRefreshReq();
	}
}


/*****************************************************************************/
EDGE_ZONE * WinEDA_PcbFrame::Del_SegmEdgeZone(wxDC * DC, EDGE_ZONE * edge_zone)
/*****************************************************************************/
/* Routine d'effacement du segment de limite zone en cours de trace */
{
EDGE_ZONE * Segm, * previous_segm;

	if (m_Pcb->m_CurrentLimitZone) Segm = m_Pcb->m_CurrentLimitZone;
	else Segm = edge_zone;

	if( Segm == NULL) return NULL;

	Trace_DrawSegmentPcb(DrawPanel, DC, Segm, GR_XOR);

	previous_segm = (EDGE_ZONE *)Segm->Pback;
	delete Segm;

	Segm = previous_segm;
	m_Pcb->m_CurrentLimitZone = Segm;
	GetScreen()->m_CurrentItem = Segm;

	if( Segm )
		{
		Segm->Pnext = NULL;
		if( DrawPanel->ManageCurseur)
			DrawPanel->ManageCurseur(DrawPanel, DC, TRUE);
		}
	else
		{
		DrawPanel->ManageCurseur = NULL;
		DrawPanel->ForceCloseManageCurseur = NULL;
		GetScreen()->m_CurrentItem = NULL;
		}
	return Segm;
}


/*********************************************/
void WinEDA_PcbFrame::CaptureNetName(wxDC * DC)
/*********************************************/
/* routine permettant de capturer le nom net net (netcode) d'un pad
 ou d'une piste pour l'utiliser comme netcode de zone
*/
{
D_PAD* pt_pad = 0;
TRACK * adrpiste ;
MODULE * Module;
int masquelayer = g_TabOneLayerMask[GetScreen()->m_Active_Layer];
int netcode;

	netcode = -1;
	MsgPanel->EraseMsgBox();
	adrpiste = Locate_Pistes(m_Pcb->m_Track, masquelayer,CURSEUR_OFF_GRILLE);
	if ( adrpiste == NULL )
		{
		pt_pad = Locate_Any_Pad(m_Pcb, CURSEUR_OFF_GRILLE);

		if(pt_pad)  /* Verif qu'il est bien sur la couche active */
			{
			Module = (MODULE*) pt_pad->m_Parent;
			pt_pad = Locate_Pads(Module,g_TabOneLayerMask[GetScreen()->m_Active_Layer],
												CURSEUR_OFF_GRILLE);
			}
		if( pt_pad )
			{
			pt_pad->Display_Infos(this);
			netcode = pt_pad->m_NetCode;
			}
		}
	else
		{
		Affiche_Infos_Piste(this, adrpiste) ;
		netcode = adrpiste->m_NetCode;
		}

	// Mise en surbrillance du net
	if(g_HightLigt_Status) Hight_Light(DC);
	g_HightLigth_NetCode = netcode;

	if ( g_HightLigth_NetCode >= 0 )
		{
		Hight_Light(DC);
		}

	/* Affichage du net selectionne pour la zone a tracer */
	Display_Zone_Netname(this);
}

/*******************************************************/
static void Display_Zone_Netname(WinEDA_PcbFrame *frame)
/*******************************************************/
/*
	Affiche le net_code et le nom de net couramment selectionne
*/
{
EQUIPOT * pt_equipot;
wxString line;

	pt_equipot = frame->m_Pcb->m_Equipots;

	if( g_HightLigth_NetCode > 0 )
	{
		for( ; pt_equipot != NULL; pt_equipot = (EQUIPOT*)pt_equipot->Pnext)
		{
			if( pt_equipot->m_NetCode == g_HightLigth_NetCode) break;
		}
		if( pt_equipot )
		{
			line.Printf( wxT("Zone: Net[%d] <%s>"),g_HightLigth_NetCode,
												pt_equipot->m_Netname.GetData());
		}
		else line.Printf( wxT("Zone: NetCode[%d], Equipot not found"),
												g_HightLigth_NetCode);
	}

	line  = _("Zone: No net selected");

	frame->Affiche_Message(line);
}

/********************************************************/
static void Exit_Zones(WinEDA_DrawPanel * Panel, wxDC *DC)
/********************************************************/
/* routine d'annulation de la Commande Begin_Zone si une piste est en cours
	de tracage, ou de sortie de l'application SEGZONES.
	Appel par la touche ESC
 */
{
WinEDA_PcbFrame* pcbframe = (WinEDA_PcbFrame*)Panel->m_Parent;

	if( pcbframe->m_Pcb->m_CurrentLimitZone )
		{
		if( Panel->ManageCurseur ) /* trace en cours */
			{
			Panel->ManageCurseur(Panel, DC, 0);
			}
		pcbframe->DelLimitesZone(DC, FALSE);
		}

	Panel->ManageCurseur = NULL;
	Panel->ForceCloseManageCurseur = NULL;
	pcbframe->GetScreen()->m_CurrentItem = NULL;

}


/**************************************************************/
void WinEDA_BasePcbFrame::DelLimitesZone(wxDC *DC, bool Redraw)
/**************************************************************/
/* Supprime la liste des segments constituant la frontiere courante
	Libere la memoire correspondante
*/
{
EDGE_ZONE * segment, * Next;

	if( m_Pcb->m_CurrentLimitZone == NULL ) return;

	if ( ! IsOK(this, _("Delete Current Zone Edges")) ) return;
	/* efface ancienne limite de zone */
	segment = m_Pcb->m_CurrentLimitZone;
	for( ; segment != NULL; segment = Next)
		{
		Next = (EDGE_ZONE*) segment->Pback;
		if ( Redraw ) Trace_DrawSegmentPcb(DrawPanel, DC, segment,GR_XOR);
		segment->Pnext = NULL; delete segment;
		}

	GetScreen()->m_CurrentItem = NULL;
	m_Pcb->m_CurrentLimitZone = NULL;
}

/********************************************/
EDGE_ZONE * WinEDA_PcbFrame::Begin_Zone(void)
/********************************************/
/*
Routine d'initialisation d'un trace de Limite de Zone ou
	de placement d'un point intermediaire
*/
{
EDGE_ZONE * oldedge, * newedge = NULL;

	oldedge = m_Pcb->m_CurrentLimitZone;

	if( (m_Pcb->m_CurrentLimitZone == NULL ) || /* debut reel du trace */
		(DrawPanel->ManageCurseur == NULL) ) /* reprise d'un trace complementaire */
		{
		m_Pcb->m_CurrentLimitZone = newedge = new EDGE_ZONE( m_Pcb );

		newedge->m_Flags = IS_NEW | STARTPOINT | IS_MOVED;
		newedge->Pback = oldedge;
		if(oldedge) oldedge->Pnext = newedge;
		newedge->m_Layer = GetScreen()->m_Active_Layer;
		newedge->m_Width = 2 ;		/* Largeur minimum tracable */
		newedge->m_Start = newedge->m_End = GetScreen()->m_Curseur;

		m_Pcb->m_CurrentLimitZone = newedge;
		DrawPanel->ManageCurseur = Show_Zone_Edge_While_MoveMouse;
		DrawPanel->ForceCloseManageCurseur = Exit_Zones;
		}

	else	/* piste en cours : les coord du point d'arrivee ont ete mises
			a jour par la routine Show_Zone_Edge_While_MoveMouse*/
		{
		if( (oldedge->m_Start.x != oldedge->m_End.x) ||
			(oldedge->m_Start.y != oldedge->m_End.y) )
			{
			newedge = new EDGE_ZONE( oldedge);
			newedge->Pback = oldedge;
			oldedge->Pnext = newedge;
			newedge->m_Flags = IS_NEW | IS_MOVED;
			newedge->m_Start = newedge->m_End = oldedge->m_End;
			newedge->m_Layer = GetScreen()->m_Active_Layer;
			m_Pcb->m_CurrentLimitZone = newedge;
			}
		}

	return newedge;
}

/*********************************************/
void WinEDA_PcbFrame::End_Zone(wxDC * DC)
/*********************************************/
/*
	Routine de fin de trace d'une zone (succession de segments)
*/
{
EDGE_ZONE * PtLim;

	if( m_Pcb->m_CurrentLimitZone )
		{
		Begin_Zone();

		/* le dernier point genere est de longueur tj nulle donc inutile. */
		/* il sera raccorde au point de depart */
		PtLim =  m_Pcb->m_CurrentLimitZone;
		PtLim->m_Flags &= ~(IS_NEW|IS_MOVED);
		while( PtLim && PtLim->Pback)
			{
			PtLim = (EDGE_ZONE*) PtLim->Pback;
			if ( PtLim->m_Flags & STARTPOINT) break;
			PtLim->m_Flags &= ~(IS_NEW|IS_MOVED);
			}

		if( PtLim )
			{
			PtLim->m_Flags &= ~(IS_NEW|IS_MOVED);
			m_Pcb->m_CurrentLimitZone->m_End = PtLim->m_Start;
			}
		Trace_DrawSegmentPcb(DrawPanel, DC, m_Pcb->m_CurrentLimitZone,GR_XOR);
		}

	DrawPanel->ManageCurseur = NULL;
	DrawPanel->ForceCloseManageCurseur = NULL;
}


/******************************************************************************************/
static void Show_Zone_Edge_While_MoveMouse(WinEDA_DrawPanel * panel, wxDC * DC, bool erase)
/******************************************************************************************/
/* redessin du contour de la piste  lors des deplacements de la souris
*/
{
EDGE_ZONE * PtLim, * edgezone;
WinEDA_PcbFrame * pcbframe = (WinEDA_PcbFrame *) panel->m_Parent;

	if( pcbframe->m_Pcb->m_CurrentLimitZone == NULL ) return ;

	/* efface ancienne position si elle a ete deja dessinee */
	if( erase )
	{
		PtLim = pcbframe->m_Pcb->m_CurrentLimitZone;
		for( ; PtLim != NULL; PtLim = (EDGE_ZONE*) PtLim->Pback)
		{
			Trace_DrawSegmentPcb(panel, DC, PtLim,GR_XOR);
		}
	}

	/* mise a jour de la couche */
	edgezone = PtLim = pcbframe->m_Pcb->m_CurrentLimitZone;
	for( ; PtLim != NULL; PtLim = (EDGE_ZONE*) PtLim->Pback)
	{
		PtLim->m_Layer = pcbframe->GetScreen()->m_Active_Layer;
	}

	/* dessin de la nouvelle piste : mise a jour du point d'arrivee */
	if (Zone_45_Only)
	{/* Calcul de l'extremite de la piste pour orientations permises:
										horiz,vertical ou 45 degre */
		edgezone->m_End = pcbframe->GetScreen()->m_Curseur;
		Calcule_Coord_Extremite_45(edgezone->m_Start.x, edgezone->m_Start.y,
						&edgezone->m_End.x, &edgezone->m_End.y);
	}
	else	/* ici l'angle d'inclinaison est quelconque */
	{
		edgezone->m_End = pcbframe->GetScreen()->m_Curseur;
	}

	PtLim = edgezone;
	for( ; PtLim != NULL; PtLim = (EDGE_ZONE*) PtLim->Pback)
	{
		Trace_DrawSegmentPcb(panel, DC, PtLim,GR_XOR);
	}
}


/**********************************************/
void WinEDA_PcbFrame::Fill_Zone(wxDC * DC)
/**********************************************/
/*
Fonction generale de creation de zone
	Un contour de zone doit exister, sinon l'ensemble du PCB est utilise

	ce qui permet de creer des obstacles et donc des parties non remplies.

	Le remplissage s'effectue a partir du point d'ancrage, jusque ves les limites


	On place la zone sur la couche (layer) active.


	"Hight Light" la zone fera partie de ce net
*/
{
int ii, jj;
EDGE_ZONE* PtLim;
int lp_tmp, lay_tmp_TOP, lay_tmp_BOTTOM;
EQUIPOT* pt_equipot;
int save_isol = g_DesignSettings.m_TrackClearence;
wxPoint ZoneStartFill;
wxString msg;

	MsgPanel->EraseMsgBox();
	if ( m_Pcb->ComputeBoundaryBox() == FALSE )
	{
		DisplayError(this, wxT("Board is empty!"), 10);
		return;
	}
	DrawPanel->m_IgnoreMouseEvents = TRUE;
	WinEDA_ZoneFrame * frame = new WinEDA_ZoneFrame(this);
	ii = frame->ShowModal(); frame->Destroy();
	DrawPanel->MouseToCursorSchema();
	DrawPanel->m_IgnoreMouseEvents = FALSE;
	if ( ii ) return;

	g_DesignSettings.m_TrackClearence = g_DesignSettings.m_ZoneClearence;

	/* mise a jour de la couche */
	PtLim = m_Pcb->m_CurrentLimitZone;
	for( ; PtLim != NULL; PtLim = (EDGE_ZONE*) PtLim->Pback)
	{
		Trace_DrawSegmentPcb(DrawPanel, DC, PtLim, GR_XOR);
		PtLim->m_Layer = GetScreen()->m_Active_Layer;
		Trace_DrawSegmentPcb(DrawPanel, DC, PtLim, GR_XOR);
	}

	TimeStamp = time( NULL );

	/* Calcul du pas de routage fixe a 5 mils et plus */
	E_scale = g_GridRoutingSize / 50 ; if (g_GridRoutingSize < 1 ) g_GridRoutingSize = 1 ;

	/* calcule de Ncols et Nrow, taille de la matrice de routage */
	ComputeMatriceSize(this, g_GridRoutingSize);

	/* Determination de la cellule pointee par la souris */
	ZoneStartFill.x = (GetScreen()->m_Curseur.x - m_Pcb->m_BoundaryBox.m_Pos.x + (g_GridRoutingSize/2) ) / g_GridRoutingSize;
	ZoneStartFill.y = (GetScreen()->m_Curseur.y - m_Pcb->m_BoundaryBox.m_Pos.y + (g_GridRoutingSize/2) ) / g_GridRoutingSize;
	if(ZoneStartFill.x < 0) ZoneStartFill.x = 0;
	if(ZoneStartFill.x >= Ncols) ZoneStartFill.x = Ncols-1;
	if(ZoneStartFill.y < 0) ZoneStartFill.y = 0;
	if(ZoneStartFill.y >= Nrows) ZoneStartFill.y = Nrows-1;

	/* Creation du mapping de la matrice de routage */
	Nb_Sides = ONE_SIDE;
	if( Board.InitBoard() < 0)
		{
		DisplayError(this, wxT("Mo memory for creating zones"));
		return;
		}

	msg.Printf( wxT("%d"),Ncols);
	Affiche_1_Parametre(this, 1, wxT("Cols"),msg,GREEN);
	msg.Printf( wxT("%d"),Nrows);
	Affiche_1_Parametre(this, 7, wxT("Lines"),msg,GREEN);
 	msg.Printf( wxT("%d"), Board.m_MemSize / 1024 );
	Affiche_1_Parametre(this, 14, wxT("Mem(Ko)"),msg,CYAN);

	lay_tmp_BOTTOM = Route_Layer_BOTTOM;
	lay_tmp_TOP = Route_Layer_TOP;

	Route_Layer_BOTTOM = Route_Layer_TOP = GetScreen()->m_Active_Layer;
	lp_tmp = g_DesignSettings.m_CurrentTrackWidth; g_DesignSettings.m_CurrentTrackWidth = g_GridRoutingSize;

	/* Affichage du NetName */
	if(g_HightLigth_NetCode > 0)
	{
		pt_equipot = GetEquipot(m_Pcb, g_HightLigth_NetCode);
		if( pt_equipot == NULL)
		{
			if(g_HightLigth_NetCode > 0 ) DisplayError(this, wxT("Equipot Error"));
		}
		else msg = pt_equipot->m_Netname;
	}
	else msg = _("No Net");

	Affiche_1_Parametre(this, 22,_("NetName"),msg,RED) ;

	/* Init des points d'accrochage possibles de la zone:
		les pistes du net sont des points d'accrochage convenables*/
	TRACK * pt_segm = m_Pcb->m_Track;
	for( ; pt_segm != NULL; pt_segm = (TRACK*) pt_segm->Pnext)
	{
		if(g_HightLigth_NetCode != pt_segm->m_NetCode) continue;
		if ( pt_segm->m_Layer != GetScreen()->m_Active_Layer ) continue;
		if (pt_segm->m_StructType != TYPETRACK ) continue;
		TraceSegmentPcb(m_Pcb, pt_segm, CELL_is_FRIEND, 0, WRITE_CELL );
	}

	/* Trace des contours du PCB sur la matrice de routage: */
	Route_Layer_BOTTOM = Route_Layer_TOP = EDGE_N;
	PlaceCells(m_Pcb, -1, 0);
	Route_Layer_BOTTOM = Route_Layer_TOP = GetScreen()->m_Active_Layer;

	/* Trace des limites de la zone sur la matrice de routage: */
	PtLim = m_Pcb->m_CurrentLimitZone;
	for( ; PtLim != NULL; PtLim = (EDGE_ZONE*)PtLim->Pback)
	{
		int ux0, uy0, ux1, uy1;
		ux0 = PtLim->m_Start.x - m_Pcb->m_BoundaryBox.m_Pos.x;
		uy0 = PtLim->m_Start.y - m_Pcb->m_BoundaryBox.m_Pos.y;
		ux1 = PtLim->m_End.x - m_Pcb->m_BoundaryBox.m_Pos.x;
		uy1 = PtLim->m_End.y - m_Pcb->m_BoundaryBox.m_Pos.y;
		TraceLignePcb(ux0,uy0,ux1,uy1,-1,HOLE|CELL_is_EDGE,WRITE_CELL);
	}


	OrCell(ZoneStartFill.y,ZoneStartFill.x, BOTTOM, CELL_is_ZONE);

	/* Marquage des cellules faisant partie de la zone*/
	ii = 1; jj = 1;
	while( ii )
	{
		msg.Printf( wxT("%d"), jj++ );
		Affiche_1_Parametre(this, 50, wxT("Iter."),msg,CYAN);
		ii = Propagation(this);
	}

	/* Selection des cellules convenables pour les points d'ancrage de la zone */
	for( ii = 0; ii < Nrows ; ii++)
	{
		for( jj = 0; jj < Ncols ; jj++)
		{
			long cell = GetCell(ii,jj,BOTTOM);
			if( (cell & CELL_is_ZONE) )
			{
				if ( (cell & CELL_is_FRIEND) == 0)
					AndCell(ii,jj,BOTTOM, ~(CELL_is_FRIEND|CELL_is_ZONE) );
			}
		}
	}


    /* Maintenant, toutes les cellules candidates sont marquees */
	/* Placement des cellules (pads, tracks, vias, edges pcb ou segments)
		faisant des obsctacles sur la matrice de routage */
	ii = 0;
	if( Zone_Exclude_Pads ) ii = FORCE_PADS;
	Affiche_1_Parametre(this, 42, wxT("GenZone"),wxEmptyString,RED);
	PlaceCells(m_Pcb, g_HightLigth_NetCode, ii);
	Affiche_1_Parametre(this, -1, wxEmptyString, _("Ok"),RED);

	/* Trace des limites de la zone sur la matrice de routage
	(a pu etre detruit par PlaceCells()) : */
	PtLim = m_Pcb->m_CurrentLimitZone;
	for( ; PtLim != NULL; PtLim = (EDGE_ZONE*)PtLim->Pback)
	{
		int ux0, uy0, ux1, uy1;
		ux0 = PtLim->m_Start.x - m_Pcb->m_BoundaryBox.m_Pos.x;
		uy0 = PtLim->m_Start.y - m_Pcb->m_BoundaryBox.m_Pos.y;
		ux1 = PtLim->m_End.x - m_Pcb->m_BoundaryBox.m_Pos.x;
		uy1 = PtLim->m_End.y - m_Pcb->m_BoundaryBox.m_Pos.y;
		TraceLignePcb(ux0,uy0,ux1,uy1,-1,HOLE|CELL_is_EDGE,WRITE_CELL);
	}


	/* Init du point d'accrochage de la zone donné par la position souris
	(a pu etre detruit par PlaceCells()) : */
	OrCell(ZoneStartFill.y,ZoneStartFill.x, BOTTOM, CELL_is_ZONE);

	if(Zone_Debug) DisplayBoard(DrawPanel, DC);

	/* Remplissage des cellules (creation effective de la zone)*/
	ii = 1; jj = 1;
	while( ii )
	{
		msg.Printf( wxT("%d"), jj++ );
		Affiche_1_Parametre(this, 50, wxT("Iter."),msg,CYAN);
		ii = Propagation(this);
	}

	if(Zone_Debug) DisplayBoard(DrawPanel, DC);

	/* Generation des segments de piste type Zone correspondants*/
	if(g_HightLigth_NetCode < 0 )
			Genere_Segments_Zone(this, DC, 0);
	else	Genere_Segments_Zone(this, DC, g_HightLigth_NetCode);

	/* Trace des connexions type frein thermique */
	g_DesignSettings.m_CurrentTrackWidth = lp_tmp;
	if ( Zone_Exclude_Pads && Zone_Genere_Freins_Thermiques)
			Genere_Pad_Connexion(this, DC, GetScreen()->m_Active_Layer);

	g_DesignSettings.m_TrackClearence = save_isol;

	GetScreen()->SetModify();

	/* Liberation de la memoire */
	Board.UnInitBoard();

	/* Reprise des conditions initiales */
	Route_Layer_TOP = lay_tmp_TOP;
	Route_Layer_BOTTOM = lay_tmp_BOTTOM;
}


/*******************************************************************************/
static void Genere_Segments_Zone(WinEDA_PcbFrame *frame, wxDC * DC, int net_code)
/*******************************************************************************/
/* Genere les segments de piste dans les limites de la zone a remplir
	Algorithme:
		procede en 2 balayages
			- Gauche->droite
			- Haut->Bas
	Parametres:
		net_code = net_code a attribuer au segment de zone
		TimeStamp(global): signature temporelle d'identification
			(mis en .start)
*/
{
int row, col;
long current_cell, old_cell;
int ux0 = 0, uy0 = 0, ux1 = 0, uy1 = 0;
int Xmin = frame->m_Pcb->m_BoundaryBox.m_Pos.x;
int Ymin = frame->m_Pcb->m_BoundaryBox.m_Pos.y;
SEGZONE * pt_track;
int layer = frame->GetScreen()->m_Active_Layer;
int nbsegm = 0;
wxString msg;

	/* balayage Gauche-> droite */
	Affiche_1_Parametre(frame, 64, wxT("Segm H"), wxT("0"),BROWN);
	for( row = 0; row < Nrows ; row++)
		{
		old_cell = 0;
		uy0 = uy1 = (row * g_GridRoutingSize) + Ymin;
		for( col = 0; col < Ncols ; col++)
			{
			current_cell = GetCell(row,col,BOTTOM) & CELL_is_ZONE;
			if(current_cell) /* ce point doit faire partie d'un segment */
				{
				ux1 = (col * g_GridRoutingSize) + Xmin;
				if( old_cell == 0 ) ux0 = ux1;
				}

			if( ! current_cell || (col == Ncols-1) ) /* peut etre fin d'un segment */
				{
				if( (old_cell) && (ux0 != ux1) )
					{	  /* un segment avait debute de longueur > 0 */
					pt_track = new SEGZONE(frame->m_Pcb);
					pt_track->m_Layer = layer;
					pt_track->m_NetCode = net_code;
					pt_track->m_Width = g_GridRoutingSize;
					pt_track->m_Start.x = ux0; pt_track->m_Start.y = uy0;
					pt_track->m_End.x = ux1; pt_track->m_End.y = uy1;
					pt_track->m_TimeStamp = TimeStamp;
					pt_track->Insert(frame->m_Pcb, NULL);
					pt_track->Draw(frame->DrawPanel, DC, GR_OR);
					nbsegm++;
					}
				}
			old_cell = current_cell;
			}
		msg.Printf( wxT("%d"),nbsegm);
		Affiche_1_Parametre(frame, -1, wxEmptyString,msg,BROWN);
		}

	Affiche_1_Parametre(frame, 72, wxT("Segm V"), wxT("0"),BROWN);
	for( col = 0; col < Ncols ; col++)
		{
		old_cell = 0;
		ux0 = ux1 = (col * g_GridRoutingSize) + Xmin;
		for( row = 0; row < Nrows ; row++)
			{
			current_cell = GetCell(row,col,BOTTOM) & CELL_is_ZONE;
			if(current_cell) /* ce point doit faire partie d'un segment */
				{
				uy1 = (row * g_GridRoutingSize) + Ymin;
				if( old_cell == 0 ) uy0 = uy1;
				}
			if( ! current_cell || (row == Nrows-1) )	/* peut etre fin d'un segment */
				{
				if( (old_cell) && (uy0 != uy1) )
					{	  /* un segment avait debute de longueur > 0 */
					pt_track = new SEGZONE(frame->m_Pcb);
					pt_track->m_Layer = layer;
					pt_track->m_Width = g_GridRoutingSize;
					pt_track->m_NetCode = net_code;
					pt_track->m_Start.x = ux0; pt_track->m_Start.y = uy0;
					pt_track->m_End.x = ux1; pt_track->m_End.y = uy1;
					pt_track->m_TimeStamp = TimeStamp;
					pt_track->Insert(frame->m_Pcb, NULL);
					pt_track->Draw(frame->DrawPanel, DC, GR_OR);
					nbsegm++;
					}
				}
			old_cell = current_cell;
			}
		msg.Printf( wxT("%d"),nbsegm);
		Affiche_1_Parametre(frame, -1, wxEmptyString,msg,BROWN);
		}
}


/********************************************/
int Propagation(WinEDA_PcbFrame * frame)
/********************************************/
/* Determine les cellules inscrites dans les limites de la zone a remplir
	Algorithme:
	Si une cellule disponible a un voisin faisant partie de la zone, elle
	devient elle meme partie de la zone
	On procede en 4 balayages de la matrice des cellules
			- Gauche->droite de Haut->bas
			- Droite->gauche de Haut->bas
			- Bas->Haut de Droite->gauche
			- Bas->Haut de Gauche->Droite
		et pour chaque balayage, on considere des 2 cellules voisines de
	la cellule courants: cellule precedente sur la ligne et cellule precedente
	sur la colonne.

	La routine peut demander plusieurs iterations
	les iterations doivent continuer juqu'a ce que la routine ne trouve plus
	de cellules a modifier.
	Retourne:
	Nombre de cellules modifiees (c.a.d mises a la valeur CELL_is_ZONE.
*/
{
int row, col, nn;
long current_cell, old_cell_H;
int long * pt_cell_V;
int nbpoints = 0;
#define NO_CELL_ZONE (HOLE | CELL_is_EDGE | CELL_is_ZONE)
wxString msg;

	Affiche_1_Parametre(frame, 57, wxT("Detect"),msg,CYAN);
	/* balayage Gauche-> droite de Haut->bas */
	Affiche_1_Parametre(frame, -1, wxEmptyString, wxT("1"),CYAN);

	// Reservation memoire pour stockahe de 1 ligne ou une colonne de cellules
	nn = MAX(Nrows, Ncols) * sizeof(*pt_cell_V);
	pt_cell_V = (long *) MyMalloc(nn);
	memset(pt_cell_V, 0, nn);
	for( row = 0; row < Nrows ; row++)
		{
		old_cell_H = 0;
		for( col = 0; col < Ncols ; col++)
			{
			current_cell = GetCell(row,col,BOTTOM) & NO_CELL_ZONE;
			if(current_cell == 0 )  /* une cellule libre a ete trouvee */
				{
				if( (old_cell_H & CELL_is_ZONE)
					|| (pt_cell_V[col] & CELL_is_ZONE) )
					{
					OrCell(row,col,BOTTOM,CELL_is_ZONE);
					current_cell = CELL_is_ZONE;
					nbpoints++;
					}
				}
			pt_cell_V[col] = old_cell_H = current_cell;
			}
		}

	/* balayage Droite-> gauche de Haut->bas */
	Affiche_1_Parametre(frame, -1, wxEmptyString, wxT("2"),CYAN);
	memset(pt_cell_V, 0, nn);
	for( row = 0; row < Nrows ; row++)
		{
		old_cell_H = 0;
		for( col = Ncols -1 ; col >= 0 ; col--)
			{
			current_cell = GetCell(row,col,BOTTOM) & NO_CELL_ZONE ;
			if(current_cell == 0 )  /* une cellule libre a ete trouvee */
				{
				if( (old_cell_H & CELL_is_ZONE)
					|| (pt_cell_V[col] & CELL_is_ZONE) )
					{
					OrCell(row,col,BOTTOM,CELL_is_ZONE);
					current_cell = CELL_is_ZONE;
					nbpoints++;
					}
				}
			pt_cell_V[col] = old_cell_H = current_cell;
			}
		}

	/* balayage Bas->Haut de Droite->gauche */
	Affiche_1_Parametre(frame, -1, wxEmptyString, wxT("3"),CYAN);
	memset(pt_cell_V, 0, nn);
	for( col = Ncols -1 ; col >= 0 ; col--)
		{
		old_cell_H = 0;
		for( row = Nrows-1; row >= 0 ; row--)
			{
			current_cell = GetCell(row,col,BOTTOM) & NO_CELL_ZONE ;
			if(current_cell == 0 )  /* une cellule libre a ete trouvee */
				{
				if( (old_cell_H & CELL_is_ZONE)
					|| (pt_cell_V[row] & CELL_is_ZONE) )
					{
					OrCell(row,col,BOTTOM,CELL_is_ZONE);
					current_cell = CELL_is_ZONE;
					nbpoints++;
					}
				}
			pt_cell_V[row] = old_cell_H = current_cell;
			}
		}

	/* balayage  Bas->Haut de Gauche->Droite*/
	Affiche_1_Parametre(frame, -1, wxEmptyString, wxT("4"),CYAN);
	memset(pt_cell_V, 0, nn);
	for( col = 0 ; col < Ncols ; col++)
		{
		old_cell_H = 0;
		for( row = Nrows-1; row >= 0 ; row--)
			{
			current_cell = GetCell(row,col,BOTTOM) & NO_CELL_ZONE ;
			if(current_cell == 0 )  /* une cellule libre a ete trouvee */
				{
				if( (old_cell_H & CELL_is_ZONE)
					|| (pt_cell_V[row] & CELL_is_ZONE) )
					{
					OrCell(row,col,BOTTOM,CELL_is_ZONE) ;
					current_cell = CELL_is_ZONE;
					nbpoints++;
					}
				}
			pt_cell_V[row] = old_cell_H = current_cell;
			}
		}

	MyFree(pt_cell_V);

	return(nbpoints);
}



/*****************************************************************************/
static bool Genere_Pad_Connexion(WinEDA_PcbFrame *frame, wxDC * DC, int layer)
/*****************************************************************************/
/* Generation des segments de zone de connexion zone / pad pour constitution
de freins thermiques
*/
{
int ii, jj, Npads;
D_PAD * pt_pad;
LISTE_PAD * pt_liste_pad;
TRACK * pt_track, * loctrack;
int angle;
int cX, cY, dx, dy;
int sommet[4][2];
wxString msg;

	if( frame->m_Pcb->m_Zone == NULL ) return FALSE;	/* pas de zone */
	if( frame->m_Pcb->m_Zone->m_TimeStamp != TimeStamp ) /* c'est une autre zone */
		return FALSE;

	/* Calcul du nombre de pads a traiter et affichage */
	Affiche_1_Parametre(frame, 50, wxT("NPads"),wxT("    "),CYAN);
	pt_liste_pad = (LISTE_PAD*) frame->m_Pcb->m_Pads;
	for ( ii = 0, Npads = 0; ii < frame->m_Pcb->m_NbPads; ii++, pt_liste_pad++)
		{
		pt_pad = *pt_liste_pad;
		/* la pastille doit etre du meme net */
		if(pt_pad->m_NetCode != g_HightLigth_NetCode) continue;
		/* la pastille doit exister sur la couche */
		if( (pt_pad->m_Masque_Layer & g_TabOneLayerMask[layer]) == 0 ) continue;
		Npads++;
		}
	msg.Printf( wxT("%d"), Npads );
	Affiche_1_Parametre(frame, -1, wxEmptyString,msg,CYAN);

	Affiche_1_Parametre(frame, 57, wxT("Pads"), wxT("     "),CYAN);
	pt_liste_pad = (LISTE_PAD*) frame->m_Pcb->m_Pads;
	for ( ii = 0, Npads = 0; ii < frame->m_Pcb->m_NbPads; ii++, pt_liste_pad++)
		{
		pt_pad = *pt_liste_pad;

		/* la pastille doit etre du meme net */
		if(pt_pad->m_NetCode != g_HightLigth_NetCode) continue;
		/* la pastille doit exister sur la couche */
		if( (pt_pad->m_Masque_Layer & g_TabOneLayerMask[layer]) == 0 ) continue;

		/* traitement du pad en cours */
		Npads++; msg.Printf( wxT("%d"), Npads );
		Affiche_1_Parametre(frame, -1, wxEmptyString,msg,CYAN);
		cX = pt_pad->m_Pos.x;	cY = pt_pad->m_Pos.y;
		dx = pt_pad->m_Size.x / 2;
		dy = pt_pad->m_Size.y / 2;
		dx += g_DesignSettings.m_TrackClearence + g_GridRoutingSize;
		dy += g_DesignSettings.m_TrackClearence + g_GridRoutingSize;

		if(pt_pad->m_PadShape == TRAPEZE)
			{
			dx += abs(pt_pad->m_DeltaSize.y) / 2;
			dy += abs(pt_pad->m_DeltaSize.x) / 2;
			}

		/* calcul des coord des 4 segments a rajouter a partir du centre cX,cY */
		sommet[0][0] = 0; sommet[0][1] = -dy;
		sommet[1][0] = -dx; sommet[1][1] = 0;
		sommet[2][0] = 0; sommet[2][1] = dy;
		sommet[3][0] = dx; sommet[3][1] = 0;

		angle = pt_pad->m_Orient;
		for(jj = 0; jj < 4; jj++)
			{
			RotatePoint(&sommet[jj][0], &sommet[jj][1], angle);

			pt_track = new SEGZONE(frame->m_Pcb);

			pt_track->m_Layer = layer;
			pt_track->m_Width = g_DesignSettings.m_CurrentTrackWidth;
			pt_track->m_NetCode = g_HightLigth_NetCode;
			pt_track->start = pt_pad;
			pt_track->m_Start.x = cX; pt_track->m_Start.y = cY;
			pt_track->m_End.x = cX + sommet[jj][0];
			pt_track->m_End.y = cY + sommet[jj][1];
			pt_track->m_TimeStamp = TimeStamp;

			/* tst si trace possible */
			if( Drc(frame, DC, pt_track,frame->m_Pcb->m_Track,0) == BAD_DRC )
				{
				delete pt_track; continue;
				}

			/* on doit pouvoir se connecter sur la zone */
			loctrack = Locate_Zone(frame->m_Pcb->m_Zone,pt_track->m_End, layer);
			if( (loctrack == NULL) || (loctrack->m_TimeStamp != TimeStamp) )
				{
				delete pt_track; continue;
				}

			pt_track->Insert(frame->m_Pcb, NULL);
			pt_track->Draw(frame->DrawPanel, DC, GR_OR);
			}
		}
	return TRUE;
}


