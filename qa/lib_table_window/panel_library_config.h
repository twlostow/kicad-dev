#ifndef __PANEL_LIBRARY_CONFIG_H
#define __PANEL_LIBRARY_CONFIG_H

#include <vector>
#include <set>
#include <bitmaps.h>

#include "panel_library_config_base.h"

class LIB_LIST_MODEL : public wxDataViewVirtualListModel
{
public:
    enum
    {
        COL_LIBRARY_WRITABLE = 0,
        COL_LIBRARY_NAME,
        COL_LIBRARY_PATH,
        COL_LIBRARY_TYPE,
        COL_LIBRARY_ENABLED,
        COL_MAX
    };

    struct ENTRY
    {
        int id;
        wxString    name;
        wxString    path;
        wxString    pluginType;
        bool    enabled;
        bool    writable;
    };

    typedef std::vector<ENTRY> ENTRIES;

    LIB_LIST_MODEL() {};
    virtual ~LIB_LIST_MODEL() {};

    ENTRIES& Entries()
    {
        return m_entries;
    }

    const ENTRIES& Entries() const
    {
        return m_entries;
    }

    virtual unsigned int GetColumnCount() const
    {
        return COL_MAX;
    }

    virtual unsigned int GetRowCount() const
    {
        // printf("GRC %d\n", m_entries.size() );
        return m_entries.size();
    }

    virtual unsigned int GetCount() const
    {
        // printf("GC %d\n", m_entries.size() );
        return m_entries.size();
    }

    virtual wxString GetColumnType( unsigned int col ) const
    {
        switch( col )
        {
        case COL_LIBRARY_WRITABLE:
            return wxT( "wxBitmap" );

        case COL_LIBRARY_PATH:
        case COL_LIBRARY_TYPE:
        case COL_LIBRARY_NAME:
            return wxT( "string" );

        case COL_LIBRARY_ENABLED:
            return wxT( "bool" );

        default:
            return wxT( "string" );
        }
    }

    virtual void GetValueByRow( wxVariant& variant,
            unsigned int row, unsigned int col ) const
    {
        if( row >= m_entries.size() )
            return;

        switch( col )
        {
        case COL_LIBRARY_NAME:
            variant = m_entries[row].name;
            return;

        case COL_LIBRARY_TYPE:
            variant = m_entries[row].pluginType;
            return;

        case COL_LIBRARY_PATH:
            variant = wxString( m_entries[row].path );
            return;

        case COL_LIBRARY_ENABLED:
            variant = m_entries[row].enabled;
            return;

        case COL_LIBRARY_WRITABLE:
        {
            variant <<
            (m_entries[row].writable ? KiBitmap( empty_bitmap_xpm ) : KiBitmap( read_only_xpm ) );
            return;
        }
        }
    }

    virtual bool GetAttrByRow( unsigned int row, unsigned int col,
            wxDataViewItemAttr& attr ) const
    {
        attr = wxDataViewItemAttr();
        return true;
    };

    virtual bool SetValueByRow( const wxVariant& variant,
            unsigned int row, unsigned int col )
    {
        if( col == COL_LIBRARY_ENABLED )
        {
            m_entries[row].enabled = (bool) variant;
            return true;
        }

        return false;
    };

    void Add( const ENTRY& aEntry )
    {
        m_entries.push_back( aEntry );
    }

    wxArrayInt getRowsForItems( const wxDataViewItemArray& aItems  ) const
    {
        wxArrayInt rows;

        for( int i = 0; i< aItems.GetCount(); i++ )
        {
            int row = GetRow( aItems[i] );

            if( row < 0 || row >= m_entries.size() )
                continue;

            rows.Add( row );
        }

        return rows;
    }

    void Remove( wxArrayInt rows )
    {
        rows.Sort( my_sort_reverse );

        for( int i = 0; i < rows.GetCount(); i++ )
            m_entries.erase( m_entries.begin() + rows[i] );

        rows.Sort( my_sort );
        RowsDeleted( rows );
    }

    void Remove( const wxDataViewItemArray& aItems )
    {
        Remove( getRowsForItems( aItems ) );
    }

    void MoveRows( wxDataViewItemArray& aItems, bool aUp )
    {
        auto rows = getRowsForItems( aItems );

        if( rows.GetCount() > 1 )
        {
            int first = rows[0];

            for( int i = 1; i < rows.GetCount(); i++ )
                if( rows[i] != first + i )
                {
                    return;
                }
        }

        int first   = rows[0];
        int last    = rows[rows.GetCount() - 1];


        if( aUp && first == 0 )
            return;

        if( !aUp && last == m_entries.size() - 1 )
            return;

        aItems.Empty();

        if( aUp )
        {
            for( int i = first; i <= last; i++ )
                std::swap( m_entries[i], m_entries[i - 1] );
        }
        else
        {
            for( int i = last; i >= first; i-- )
                std::swap( m_entries[i], m_entries[i + 1] );
        }

        if( aUp )
        {
            for( int i = first - 1; i <= last; i++ )
                RowChanged( i );
        }
        else
        {
            for( int i = first; i <= last + 1; i++ )
                RowChanged( i );
        }

        if( aUp )
        {
            first--;
            last--;
        }
        else
        {
            first++;
            last++;
        }

        for( int i = first; i <= last; i++ )
            aItems.Add( GetItem( i ) );
    }

private:
    static int my_sort_reverse( int* v1, int* v2 )
    {
        return *v2 - *v1;
    }

    static int my_sort( int* v1, int* v2 )
    {
        return *v1 - *v2;
    }

    ENTRIES m_entries;
};

class PANEL_LIBRARY_CONFIG : public PANEL_LIBRARY_CONFIG_BASE
{
public:
    PANEL_LIBRARY_CONFIG( wxWindow* parent,
            wxWindowID id = wxID_ANY,
            const wxPoint& pos  = wxDefaultPosition,
            const wxSize& size  = wxSize(500, 300),
            long style = wxTAB_TRAVERSAL );
    ~PANEL_LIBRARY_CONFIG();

    void SetModel( LIB_LIST_MODEL* aModel );
    void SetWizardEnabled ( bool aEnable );

private:

    virtual void    OnAddLibrary( wxCommandEvent& event ) override;
    virtual void    OnRemoveLibrary( wxCommandEvent& event ) override;
    virtual void    OnLaunchAdvancedSettings( wxCommandEvent& event ) override;
    virtual void    OnRunWizard( wxCommandEvent& event ) override;
    virtual void    OnMoveUp( wxCommandEvent& event ) override;
    virtual void    OnMoveDown( wxCommandEvent& event ) override;


    LIB_LIST_MODEL* m_model;
};


#endif
