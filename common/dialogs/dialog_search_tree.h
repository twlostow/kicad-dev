/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 CERN
 * @author Michele Castellana <michele.castellana@cern.ch>
 * @author Maciej Suminski <maciej.suminski@cern.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __DIALOG_SEARCH_TREE_H__
#define __DIALOG_SEARCH_TREE_H__

#include <dialog_search_tree_base.h>
#include <functional>

class wxConfigBase;

using ITEM_ID = wxTreeItemId;
using ITEM_IDS = wxArrayTreeItemIds;

/**
 * Class SEARCH_TREE
 * Custom wxWidgets component to display a tree structure of components (symbols or footprints).
 *
 * By default this class provides an hidden root node in order to
 * have all the object in the first layer of the tree at the same level
 */
class SEARCH_TREE : public SEARCH_TREE_BASE
{
public:
    ///> Hierarchy levels
    enum LEVELS
    {
        LIBRARY,
        COMPONENT,
        UNIT
    };

    ///> Base class used to keep custom data associated with tree items.
    struct USER_DATA
    {
        virtual ~USER_DATA()
        {}
    };

    SEARCH_TREE( wxWindow* aParent, wxWindowID aId = wxID_ANY,
            const wxPoint& aPos = wxDefaultPosition,
            const wxSize& aSize = wxSize( 500, 299 ), long aStyle = 0 );

    ~SEARCH_TREE();

    ///> Filtering flags
    enum FILTER
    {
        UNFILTERED              = 0x00,
        FILTER_BY_NAME          = 0x01,
        FILTER_BY_PIN_COUNT     = 0x02,
        FILTER_BY_LIBRARY       = 0x04
    };

    /**
     * Removes all items from the tree.
     */
    void ResetTree();

    /**
     * Refreshes the tree contents.
     */
    virtual void Update() override = 0;

    /**
     * Returns the total number of items in the tree.
     */
    unsigned int GetCount() const
    {
        return m_tree->GetCount();
    }

    /**
     * Expands all items.
     */
    void ExpandAll()
    {
        m_tree->ExpandAll();
    }

    /**
     * Expands the currently selected items.
     */
    void ExpandSelected();

    /**
     * Expands an item.
     */
    void Expand( const ITEM_ID& aItem )
    {
        m_tree->Expand( aItem );
    }

    /**
     * Selects an item.
     */
    void SelectItem( const ITEM_ID& aItem );

    /**
     * Checks whether an item is selected.
     * @return True if it is selected.
     */
    bool IsSelected( const ITEM_ID& aItem ) const
    {
        return aItem.IsOk() && m_tree->IsSelected( aItem );
    }

    virtual void SaveSettings( wxConfigBase* aCfg );

    /**
     * Returns current filtering settings.
     */
    int GetFilteringOptions() const
    {
        return m_filteringOptions;
    }

    /**
     * Sets the list of keywords used to filter the tree items.
     */
    void SetFilteringKeywords( const wxArrayString aFilter );

    /**
     * Sets the number of pins used for filtering the tree items.
     */
    void SetPinCountFilter( unsigned int aNum );

    /**
     * Inserts an item as a child of the root node.
     * @param aLibrary is the string describing the item.
     * @param aData is an optional data related to the item. It is destroyed upon the item removal.
     */
    ITEM_ID InsertLibrary( const wxString& aLibrary, USER_DATA* aData = nullptr );

    /**
     * Removes a library, including all the components.
     * @return True if the operation succeeds.
     */
    bool RemoveLibrary( const wxString& aLibrary );

    /**
     * Inserts a component in a selected library.
     * The library has to exist for the operation to succeed.
     *
     * @param aItem is the component name.
     * @param aParent is the ID of the parent library.
     * @param aData is optional user data assigned to the item.
     * @return ID of the inserted item.
     */
    ITEM_ID InsertItem( const wxString& aItem, const ITEM_ID& aParent, USER_DATA* aData = nullptr );

    /**
     * Inserts a component in a selected library.
     * The library has to exist for the operation to succeed.
     *
     * @param aItem is the component name.
     * @param aParent is the name of the parent library.
     * @param aData is optional user data assigned to the item.
     * @return ID of the inserted item.
     */
    ITEM_ID InsertItem( const wxString& aItem, const wxString& aParent, USER_DATA* aData = nullptr )
    {
        return InsertItem( aItem, FindItem( aParent ) );
    }

    /**
     * Removes an item. Works both for libraries and components.
     * @return True in case of success.
     */
    bool RemoveItem( const ITEM_ID& aItem );

    /**
     * Assigns user data for an item. The SEARCH_TREE object takes ownership of the data,
     * it will be destroyed once the owner item is removed.
     * @param aItem is the parent item.
     * @param aData is the data to be assigned.
     */
    bool SetData( const ITEM_ID& aItem, USER_DATA* aData );

    /**
     * Returns user data for an item.
     * @param aItem is the owner item to retrieve data from.
     */
    template<typename T>
    T* GetData( const ITEM_ID& aItem )
    {
        if( !aItem.IsOk() )
            return nullptr;

        ITEM_DATA* data = static_cast<ITEM_DATA*>( m_tree->GetItemData( aItem ) );
        return static_cast<T*>( data->m_userData );
    }

    /**
     * Returns a single selected object on the requested level.
     * If not possible, it returns an invalid ITEM_ID.
     */
    ITEM_ID GetSingleSelected( int aLevel ) const;

    /**
     * Returns the list of currently selected libraries.
     */
    wxArrayString GetSelectedLibraries() const;

    /**
     * Returns the list of currently selected components.
     */
    wxArrayString GetSelectedComponents() const;

    /**
     * Returns the currently selected component. If there are multiple components selected,
     * it returns an empty string.
     */
    wxString GetSelectedComponent() const
    {
        return GetText( GetSingleSelected( COMPONENT ) );
    }

    /**
     * Returns the currently selected component ID. If there are multiple components selected,
     * it returns an invalid ITEM_ID.
     */
    ITEM_ID GetSelectedComponentId() const
    {
        return GetSingleSelected( COMPONENT );
    }

    /**
     * Returns the currently selected component. If there are multiple libraries selected,
     * it returns an empty string.
     */
    wxString GetSelectedLibrary() const
    {
        return GetText( GetSingleSelected( LIBRARY ) );
    }

    /**
     * Returns the currently selected component ID. If there are multiple libraries selected,
     * it returns an invalid ITEM_ID.
     */
    ITEM_ID GetSelectedLibraryId() const
    {
        return GetSingleSelected( LIBRARY );
    }

    void OnFiltering( bool aApply, SEARCH_TREE::FILTER aFilter );

    /**
     * Returns text of an ITEM_ID (optionally at requested level).
     */
    wxString GetText( const ITEM_ID& aItem, int aLevel = -1 ) const
    {
        ITEM_ID id = ( aLevel >= 0 ? GetItemAtLevel( aItem, aLevel ) : aItem );

        return ( id.IsOk() ? m_tree->GetItemText( id ) : wxString() );
    }

    /**
     * Checks whether a particular item is a library.
     */
    bool IsLibrary( const ITEM_ID& aItem ) const
    {
        return GetLevel( aItem ) == 0;
    }

    /**
     * Checks whether a particular item is a component.
     */
    bool IsComponent( const ITEM_ID& aItem ) const
    {
        return GetLevel( aItem ) == 1;
    }

    /**
     * Returns the hierarchy level of an item. 0 is the top level (libraries).
     */
    int GetLevel( const ITEM_ID& aItem ) const;

    /**
     * Returns an ITEM_ID for a requested level in the same branch. The requested level
     * has to be lower than the aItem's level, otherwise there is no unambiguous answer.
     * This function is used to e.g. obtain library ITEM_ID for a part/unit.
     */
    ITEM_ID GetItemAtLevel( const ITEM_ID& aItem, int aLevel ) const;

    /**
     * Returns the library ITEM_ID owning a particular node.
     */
    wxString GetLibrary( const ITEM_ID& aItem ) const;

    /**
     * Returns the component ITEM_ID for a particular node. If aItem's level is higher than
     * component then an invalid ITEM_ID is returned.
     */
    wxString GetComponent( const ITEM_ID& aItem ) const;

    /**
     * Sets the font for an item.
     * @param aItem is the item name to be marked.
     * @param aBold makes the item displayed with a bold font
     * @param aItalic makes the item displayed with an italic font
     * @return False in case of an error, true otherwise.
     */
    bool SetItemFont( const ITEM_ID& aItem, bool aBold, bool aItalic );

    // idem
    bool SetItemColor( const ITEM_ID& aItem, wxColour aColour );

	/**
     * Marks an item as (un)modified.
     * @param aItem is the item name to be marked.
     * @param aModified decides whether the item should be marked as modified or not.
     * @return False in case of an error, true otherwise.
     */
    bool SetModified( const ITEM_ID& aItem, bool aModified = true);

    /**
     * Returns true if aItem has been modified.
     */
    bool IsModified( const ITEM_ID& aItem ) const;

    /**
     * Marks aItem as the currently modified item.
     */
    bool SetCurrent( const ITEM_ID& aItem );

    /**
     * Returns ture if aItem is the currently modified item.
     */
    bool IsCurrent( const ITEM_ID& aItem ) const;

    /**
     * Returns the currently modified library name.
     */
    const wxString& CurrentLibrary() const { return m_currentLib; }

    /**
     * Returns the currently modified component name.
     */
    const wxString& CurrentComponent() const { return m_currentComp; }

    // Depth First Search
    ITEM_ID FindItem( const wxString& aSearchFor, const wxString& aParent = wxEmptyString ) const;

    // First layer of the tree
    ITEM_ID FindLibrary( const wxString& aSearchFor ) const;

    /**
     * Returns the array of currently selected items.
     */
    virtual ITEM_IDS GetSelected() const;

    /**
     * Invokes a function on all ITEM_IDs having specified level. If the levels are not specified,
     * the function is executed on every ITEM_ID.
     */
    void Visit( std::function<void(const ITEM_ID&)> aFunc, int aMinLevel = 0, int aMaxLevel = 1000 );

protected:
    ///> Helper method for Visit()
    void visitRecursive( const ITEM_ID& aNode, std::function<void(const ITEM_ID&)> aFunc, int aMinLevel = 0, int aMaxLevel = 1000 );

    ///> Checks if a phrase mathces the current list of keywords.
    bool matchKeywords( const wxString& aSearchFor ) const;

    /*
     * Interface methods for children classes in order to decouple their implementation
     * with respect to the base class
     */
    wxBoxSizer* getSizerForLabels() const
    {
        return m_labels;
    }

    wxBoxSizer* getSizerForPreviews() const
    {
        return m_previews;
    }

    wxTreeCtrl* getTree() const
    {
        return m_tree;
    }

    unsigned int getPinCount()
    {
        return m_pinCount;
    }

    ///> Enables 'Preview' widget and sets its title.
    void setPreview( bool aVal, const wxString& aText = wxString( "Preview" ) );

    ///> Returns true if the 'Preview' widget is enabled.
    bool hasPreview() const
    {
        return m_hasPreview;
    }

    /*
     * Virtual event handlers, override them in your derived class
     * By default they do nothing.
     */
    virtual void OnTextChanged( wxCommandEvent& aEvent ) override;
    virtual void OnLeftDClick( wxMouseEvent& aEvent ) override;
    

    /*
     * If you override this methods you MUST call this function.
     * The idea is that every class save its own internal.
     */
    virtual void loadSettings( wxConfigBase* aCfg );

    ///> Current filtering options. @see FILTER
    int m_filteringOptions;

    ///> Current text used for filtering (edit box contents).
    wxString m_filterPattern;

private:
    ///> Marks an item using inverted colors
    void invertColors( ITEM_ID aItem, bool aInvert );

    ///> Helper structure assigned to every node in the tree to store additional data.
    struct ITEM_DATA : public wxTreeItemData
    {
        ITEM_DATA( int aLevel, USER_DATA* aUserData, bool aModified = false )
            : m_userData( aUserData ), m_level( aLevel ), m_modified( aModified )
        {
        }

        ~ITEM_DATA()
        {
            delete m_userData;
        }

        ///> Custom user data
        USER_DATA* m_userData;

        ///> Tree depth
        int m_level;

        ///> Modified flag
        bool m_modified;
    };

    ///> Searches for an item in a branch selected by aRoot.
    ITEM_ID findItem( const ITEM_ID& aRoot, const wxString& aSearchFor ) const;

    ///> Selects an item.
    void selectItem( const ITEM_ID& aItem );

    ///> Selects multiple items.
    void selectItems( const ITEM_IDS& aItems );

    ///> Style modifiers used to create the wxTreeCtrl widget
    const long m_style;

    ///> Flag determining whether the preview widget should be visible
    bool m_hasPreview;

    ///> Keywords used for filtering
    wxArrayString m_keywords;

    ///> Pin count filter
    unsigned m_pinCount;

    ///> Name of the currently modified library
    wxString m_currentLib;

    ///> Name of the currently modified component
    wxString m_currentComp;
};

#endif //__DIALOG_SEARCH_TREE_H__
