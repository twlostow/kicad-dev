/*
 * KiRouter - a push-and-(sometimes-)shove PCB router
 *
 * Copyright (C) 2014  CERN
 * Author: Maciej Suminski <maciej.suminski@cern.ch>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.or/licenses/>.
 */

/**
 * Push and Shove router track width and via size dialog.
 */

#include "dialog_net_visibility_and_colors.h"
#include <class_board.h>
#include <wx/colordlg.h>
#include <connectivity.h>
#include <wxPcbStruct.h>
#include <class_netinfo.h>

class NET_COLOR_NODE;
WX_DEFINE_ARRAY_PTR( NET_COLOR_NODE*, NET_COLOR_NODE_ARRAY );

class NET_COLOR_NODE
{
public:
    NET_COLOR_NODE( NET_COLOR_NODE* parent,  NET_RATSNEST_PREFS_PTR aPrefs, const COLOR4D& aDefaultColor = COLOR4D(), wxString aName = _("") )
    {
        m_parent = parent;
        m_prefs = aPrefs;
        m_container = false;
        m_name = aName;
        m_defaultColor = aDefaultColor;
    }


    ~NET_COLOR_NODE()
    {
        // free all our children nodes
        size_t count = m_children.GetCount();
        for (size_t i = 0; i < count; i++)
        {
            auto child = m_children[i];
            //delete child;
        }
    }

    bool IsContainer() const
        { return m_container; }

    NET_COLOR_NODE* GetParent()
        { return m_parent; }
    NET_COLOR_NODE_ARRAY& GetChildren()
        { printf("GetCH!!!!\n"); return m_children; }
    NET_COLOR_NODE* GetNthChild( unsigned int n )
        { return m_children.Item( n ); }
    void Insert( NET_COLOR_NODE* child, unsigned int n)
        { m_children.Insert( child, n); }
    void Append( NET_COLOR_NODE* child )
        { m_children.Add( child ); }
    unsigned int GetChildCount() const
        {
            printf("GetCH %d\n", m_children.GetCount());
            return m_children.GetCount();

            }

public:     // public to avoid getters/setters
    NET_RATSNEST_PREFS_PTR m_prefs;
    bool m_container;

    wxString m_name;
    COLOR4D m_defaultColor;

private:
    NET_COLOR_NODE         *m_parent;
    NET_COLOR_NODE_ARRAY   m_children;
};

static wxBitmap makeBitmap( const COLOR4D aColor, int w, int h )
{
    wxMemoryDC dc;
    auto bmp = wxBitmap(w,h,24);
    dc.SelectObject(bmp);
    wxBrush brush ( wxColour ( aColor.r * 255.0, aColor.g * 255.0, aColor.b * 255.0 ) );

    dc.SetBrush(brush);
    dc.DrawRectangle(0, 0, w, h);
    dc.SelectObject(wxNullBitmap);
    return bmp;
}

class NET_COLOR_MODEL: public wxDataViewModel
{
public:
    NET_COLOR_MODEL()
    {
        m_root = new NET_COLOR_NODE( NULL, nullptr, COLOR4D(), "Board" );
        m_root->m_container = true;
        m_nets = new NET_COLOR_NODE( m_root, nullptr, COLOR4D(), "Nets" );
        m_nets->m_container = true;
        m_root->Append( m_nets );

        m_classes = new NET_COLOR_NODE( m_root, nullptr, COLOR4D(), "Classes" );
        m_classes->m_container = true;
        m_root->Append( m_classes );
    }

    ~NET_COLOR_MODEL()
    {
        delete m_nets;
        delete m_classes;
        delete m_root;
    }

    // helper method for wxLog

    // helper methods to change the model

    void Add( BOARD *aBoard, NETCLASSPTR aNetclass )
    {
        wxASSERT(m_root);

        auto defaultColor = aBoard->GetColorsSettings()->GetLayerColor( LAYER_RATSNEST );
        auto child_node =
            new NET_COLOR_NODE( m_classes, aNetclass->RatsnestPrefs(), defaultColor, aNetclass->GetName() );
        m_classes->Append( child_node );
    }

    void Add( BOARD *aBoard,  NETINFO_ITEM* aItem )
    {
        wxASSERT(m_root);
        auto defaultColor = aBoard->GetColorsSettings()->GetLayerColor( LAYER_RATSNEST );

        auto child_node =
            new NET_COLOR_NODE( m_nets, aItem->RatsnestPrefs(), defaultColor, aItem->GetShortNetname() );
        m_nets->Append( child_node );
    }

    void Delete( const wxDataViewItem &item )
    {

        NET_COLOR_NODE *node = (NET_COLOR_NODE*) item.GetID();
        if (!node)      // happens if item.IsOk()==false
            return;

        printf("delete %p\n", node);

        wxDataViewItem parent( node->GetParent() );
        if (!parent.IsOk())
        {
            wxASSERT(node == m_root);

            // don't make the control completely empty:
            wxLogError( "Cannot remove the root item!" );
            return;
        }

        // is the node one of those we keep stored in special pointers?
        if (node == m_nets)
            m_nets = NULL;
        else if (node == m_classes)
            m_classes = NULL;


        // first remove the node from the parent's array of children;
        // NOTE: NET_COLOR_NODEPtrArray is only an array of _pointers_
        //       thus removing the node from it doesn't result in freeing it
        node->GetParent()->GetChildren().Remove( node );

        // free the node
        //delete node;
    }


    // override sorting to always sort branches ascendingly

    int Compare( const wxDataViewItem &item1, const wxDataViewItem &item2,
                 unsigned int column, bool ascending ) const
                 {
                     return 0;
                 }

    // implementation of base class virtuals to define model

    virtual unsigned int GetColumnCount() const
    {
        return 3;
    }

    virtual wxString GetColumnType( unsigned int col ) const
    {
        if (col == 2)
            return wxT("long");

        return wxT("string");
    }

    virtual void GetValue( wxVariant &variant,
                           const wxDataViewItem &item, unsigned int col ) const
                           {
                               auto node = (NET_COLOR_NODE*) item.GetID();
                               auto prefs = node->m_prefs;

                               switch( col )
                               {
                                   case 0:
                                   {
                                    variant = node->m_name;
                                    break;
                                }
                                   case 1:
                                   {

                                     variant = prefs->m_showRatsnest;
                                     break;
                                    }
                                 case 2:
                                 {
                                     COLOR4D color;
                                     wxBitmap bmp;

                                     if (prefs && prefs->m_useCustomColor)
                                         bmp = makeBitmap( prefs->m_color, 16, 16 );
                                     else
                                        bmp = makeBitmap( node->m_defaultColor, 16, 16 );


                                     variant << bmp;
                                     break;
                               }

                               }
                           }


    virtual bool SetValue( const wxVariant &variant,
                           const wxDataViewItem &item, unsigned int col )
                           {
                               //printf("SetValue!\n");

                               auto node = reinterpret_cast<NET_COLOR_NODE*> ( item.GetID() );
                               auto prefs = node->m_prefs;

                               switch(col)
                               {
                                   case 1:
                                   {
                                       prefs->m_showRatsnest = (bool) variant;
                                       break;
                                   }

                                   case 2:
                                   {
                                       printf("SetColorVal!\n");
                                       break;
                                   }

                               }


                               return true;
                           }

    virtual bool IsEnabled( const wxDataViewItem &item,
                            unsigned int col ) const
                            {
                                return true;
                            }

    virtual wxDataViewItem GetParent( const wxDataViewItem &item ) const
    {
        // the invisible root node has no parent
        if (!item.IsOk())
            return wxDataViewItem(0);

        NET_COLOR_NODE *node = (NET_COLOR_NODE*) item.GetID();

        // "MyMusic" also has no parent
        if (node == m_root)
            return wxDataViewItem(0);

        return wxDataViewItem( (void*) node->GetParent() );
    }

    virtual bool IsContainer( const wxDataViewItem &item ) const
    {
        if (!item.IsOk())
            return true;

        NET_COLOR_NODE *node = (NET_COLOR_NODE*) item.GetID();
        return node->IsContainer();
    }

    virtual unsigned int GetChildren( const wxDataViewItem &parent,
                                      wxDataViewItemArray &array ) const
                                      {
                                          NET_COLOR_NODE *node = (NET_COLOR_NODE*) parent.GetID();

                                          if (!node)
                                          {
                                              array.Add( wxDataViewItem( (void*) m_root ) );
                                              return 1;
                                          }


                                          if (node->GetChildCount() == 0)
                                          {
                                              return 0;
                                          }

                                          unsigned int count = node->GetChildren().GetCount();
                                          for (unsigned int pos = 0; pos < count; pos++)
                                          {

                                              NET_COLOR_NODE *child = node->GetChildren().Item( pos );
                                              array.Add( wxDataViewItem( (void*) child ) );
                                          }

                                          return count;
                                      }

//private:
    NET_COLOR_NODE*   m_root;

    // pointers to some "special" nodes of the tree:
    NET_COLOR_NODE*   m_nets = nullptr;
    NET_COLOR_NODE*   m_classes = nullptr;
};



DIALOG_NET_VISIBILITY_AND_COLORS::DIALOG_NET_VISIBILITY_AND_COLORS( PCB_EDIT_FRAME* aParent ) :
    m_frame(aParent),
    DIALOG_NET_VISIBILITY_AND_COLORS_BASE( aParent )
{
    // Load router settings to dialog fields
    m_StdButtonsOK->SetDefault();

    // Now all widgets have the size fixed, call FinishDialogSettings
    FinishDialogSettings();

    // Pressing ENTER when any of the text input fields is active applies changes
    Connect( wxEVT_TEXT_ENTER, wxCommandEventHandler( DIALOG_NET_VISIBILITY_AND_COLORS::onOkClick ), NULL, this );

    //m_colColor\

    {
        wxDataViewTextRenderer *tr =
            new wxDataViewTextRenderer( "string", wxDATAVIEW_CELL_INERT );
        m_colName =
            new wxDataViewColumn( _("Name"), tr, 0, 400, wxALIGN_LEFT,
                                  wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE );
        m_dataView->AppendColumn( m_colName );
    }

    {
        wxDataViewToggleRenderer *tr =
            new wxDataViewToggleRenderer( "bool", wxDATAVIEW_CELL_ACTIVATABLE  );
        m_colName =
            new wxDataViewColumn( _("Visible"), tr, 1, 100, wxALIGN_CENTER, 0);

        m_dataView->AppendColumn( m_colName );
    }
    {
        auto tr = new wxDataViewBitmapRenderer( "wxBitmap", wxDATAVIEW_CELL_ACTIVATABLE  );
        m_colName =
            new wxDataViewColumn( _("Colour"), tr, 2, 100, wxALIGN_CENTER, 0);

        m_dataView->AppendColumn( m_colName );
    }

}


void DIALOG_NET_VISIBILITY_AND_COLORS::onClose( wxCloseEvent& aEvent )
{
    EndModal( 0 );
}


void DIALOG_NET_VISIBILITY_AND_COLORS::onOkClick( wxCommandEvent& aEvent )
{
    EndModal( 1 );
}


void DIALOG_NET_VISIBILITY_AND_COLORS::onCancelClick( wxCommandEvent& aEvent )
{
    EndModal( 0 );
}

void PCB_EDIT_FRAME::InstallNetVisibilityAndColorsDialog( )
{
    DIALOG_NET_VISIBILITY_AND_COLORS dlg( this );
    dlg.ShowModal();
}

bool DIALOG_NET_VISIBILITY_AND_COLORS::TransferDataFromWindow()
{
        return true;
}

bool DIALOG_NET_VISIBILITY_AND_COLORS::TransferDataToWindow()
{

    const auto& netinfo = m_frame->GetBoard()->GetNetInfo();

    m_model = new NET_COLOR_MODEL();
    m_dataView->AssociateModel( m_model );

    for (int i =1 ;i<netinfo.GetNetCount(); i++)
        m_model->Add( m_frame->GetBoard(), netinfo.GetNetItem(i) );

    const auto& classes = m_frame->GetBoard()->GetDesignSettings().m_NetClasses;

    for ( auto cl : classes )
        m_model->Add( m_frame->GetBoard(), cl.second );


    m_dataView->Expand ( wxDataViewItem( m_model->m_root ) );
    m_dataView->Expand ( wxDataViewItem( m_model->m_nets ) );
    m_dataView->Expand ( wxDataViewItem( m_model->m_classes ) );

    return true;
}

void DIALOG_NET_VISIBILITY_AND_COLORS::onCtrlItemActivated( wxDataViewEvent& event )
{
    printf("Activate!\n");

    wxColourDialog dlg ( this );

    dlg.ShowModal();


}

void DIALOG_NET_VISIBILITY_AND_COLORS::onCtrlItemContextMenu( wxDataViewEvent& event )
{
    wxMenu menu;
    menu.Append( 1, "Change color" );
    menu.Append( 2, "Ratnsest On" );
    menu.Append( 2, "Ratnsest Off" );
    menu.AppendSeparator();
    menu.Append( 3, "Reset to defaults" );

    m_dataView->PopupMenu(&menu);
}
