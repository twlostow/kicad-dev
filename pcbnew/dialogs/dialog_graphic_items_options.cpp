/************************************/
/* dialog_graphic_items_options.cpp */
/************************************/


#include <fctsys.h>

#include <pcbnew.h>
#include <wxPcbStruct.h>
#include <base_units.h>

#include <pcbnew_id.h>
#include <module_editor_frame.h>
#include <class_board.h>

#include <dialog_graphic_items_options.h>


void PCB_EDIT_FRAME::OnConfigurePcbOptions( wxCommandEvent& aEvent )
{
    DIALOG_GRAPHIC_ITEMS_OPTIONS dlg( this );

    dlg.ShowModal();
}


void FOOTPRINT_EDIT_FRAME::InstallOptionsFrame( const wxPoint& pos )
{
    DIALOG_GRAPHIC_ITEMS_OPTIONS dlg( this );
    dlg.ShowModal();
}


/*
 * DIALOG_GRAPHIC_ITEMS_OPTIONS constructor
 */

DIALOG_GRAPHIC_ITEMS_OPTIONS::DIALOG_GRAPHIC_ITEMS_OPTIONS( PCB_BASE_FRAME* parent )
    : DIALOG_GRAPHIC_ITEMS_OPTIONS_BASE( parent )
{
    m_parent = parent;
    m_brdSettings = m_parent->GetDesignSettings();
    initValues(  );

    m_sdbSizer1OK->SetDefault();
    GetSizer()->SetSizeHints( this );

    Centre();
}

DIALOG_GRAPHIC_ITEMS_OPTIONS::~DIALOG_GRAPHIC_ITEMS_OPTIONS()
{
}


void DIALOG_GRAPHIC_ITEMS_OPTIONS::initValues()
{
    SetFocus();

    /* Drawings width */
    AddUnitSymbol( *m_GraphicSegmWidthTitle, g_PcbUnits.GetUserUnit() );
    m_OptPcbSegmWidth->SetValue( g_PcbUnits.StringFromValue( m_brdSettings.m_DrawSegmentWidth ) );

    /* Edges width */
    AddUnitSymbol( *m_BoardEdgesWidthTitle, g_PcbUnits.GetUserUnit() );
    m_OptPcbEdgesWidth->SetValue( g_PcbUnits.StringFromValue( m_brdSettings.m_EdgeSegmentWidth ) );

    /* Pcb Textes (Size & Width) */
    AddUnitSymbol( *m_CopperTextWidthTitle, g_PcbUnits.GetUserUnit() );
    m_OptPcbTextWidth->SetValue( g_PcbUnits.StringFromValue( m_brdSettings.m_PcbTextWidth ) );

    AddUnitSymbol( *m_TextSizeVTitle, g_PcbUnits.GetUserUnit() );
    m_OptPcbTextVSize->SetValue( g_PcbUnits.StringFromValue( m_brdSettings.m_PcbTextSize.y ) );

    AddUnitSymbol( *m_TextSizeHTitle, g_PcbUnits.GetUserUnit() );
    m_OptPcbTextHSize->SetValue( g_PcbUnits.StringFromValue( m_brdSettings.m_PcbTextSize.x ) );


    /* Modules: Edges width */
    AddUnitSymbol( *m_EdgeModWidthTitle, g_PcbUnits.GetUserUnit() );
    m_OptModuleEdgesWidth->SetValue( g_PcbUnits.StringFromValue( m_brdSettings.m_ModuleSegmentWidth ) );

    /* Modules: Texts: Size & width */
    AddUnitSymbol( *m_TextModWidthTitle, g_PcbUnits.GetUserUnit() );
    m_OptModuleTextWidth->SetValue( g_PcbUnits.StringFromValue( m_brdSettings.m_ModuleTextWidth ) );

    AddUnitSymbol( *m_TextModSizeVTitle, g_PcbUnits.GetUserUnit() );
    m_OptModuleTextVSize->SetValue( g_PcbUnits.StringFromValue( m_brdSettings.m_ModuleTextSize.y ) );

    AddUnitSymbol( *m_TextModSizeHTitle, g_PcbUnits.GetUserUnit() );
    m_OptModuleTextHSize->SetValue( g_PcbUnits.StringFromValue( m_brdSettings.m_ModuleTextSize.x ) );

    AddUnitSymbol( *m_DefaultPenSizeTitle, g_PcbUnits.GetUserUnit() );
    m_DefaultPenSizeCtrl->SetValue( g_PcbUnits.StringFromValue( g_DrawDefaultLineThickness ) );
}


void DIALOG_GRAPHIC_ITEMS_OPTIONS::OnOkClick( wxCommandEvent& event )
{
    m_brdSettings.m_DrawSegmentWidth = g_PcbUnits.ValueFromString( m_OptPcbSegmWidth->GetValue() );
    m_brdSettings.m_EdgeSegmentWidth = g_PcbUnits.ValueFromString( m_OptPcbEdgesWidth->GetValue() );
    m_brdSettings.m_PcbTextWidth = g_PcbUnits.ValueFromString( m_OptPcbTextWidth->GetValue() );
    m_brdSettings.m_PcbTextSize.y = g_PcbUnits.ValueFromString( m_OptPcbTextVSize->GetValue() );
    m_brdSettings.m_PcbTextSize.x = g_PcbUnits.ValueFromString( m_OptPcbTextHSize->GetValue() );

    m_parent->GetBoard()->SetDesignSettings( m_brdSettings );

    m_brdSettings.m_ModuleSegmentWidth = g_PcbUnits.ValueFromString( m_OptModuleEdgesWidth->GetValue() );
    m_brdSettings.m_ModuleTextWidth = g_PcbUnits.ValueFromString( m_OptModuleTextWidth->GetValue() );
    m_brdSettings.m_ModuleTextSize.y = g_PcbUnits.ValueFromString( m_OptModuleTextVSize->GetValue() );
    m_brdSettings.m_ModuleTextSize.x = g_PcbUnits.ValueFromString( m_OptModuleTextHSize->GetValue() );

    g_DrawDefaultLineThickness = g_PcbUnits.ValueFromString( m_DefaultPenSizeCtrl->GetValue() );

    if( g_DrawDefaultLineThickness < 0 )
        g_DrawDefaultLineThickness = 0;

    m_parent->SetDesignSettings( m_brdSettings );

    EndModal( wxID_OK );
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void DIALOG_GRAPHIC_ITEMS_OPTIONS::OnCancelClick( wxCommandEvent& event )
{
    event.Skip();
}
