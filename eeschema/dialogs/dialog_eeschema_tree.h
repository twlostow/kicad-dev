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

#ifndef __DIALOG_FOOTPRINTS_TREE_H__
#define __DIALOG_FOOTPRINTS_TREE_H__

#include <dialog_search_tree.h>
#include <memory>

class wxListBox;
class LIB_EDIT_FRAME;
class LIB_MANAGER;

///////////////////////////////////////////////////////////////////////////////
/// Class EESCHEMA_TREE
/// > library_name_1
///   > comp_1
///     > part A
///     > part B
///   > comp_2
///   > comp_3
///   > comp_4
/// > library_name_2
///   > comp_1
///   > comp_2
///     > part A
///     > part B
///   > comp_3
///   > comp_4
/// > library_name_3
///   > comp_1
///   > comp_2
///   > comp_3
///   > comp_4
///     > part A
///     > part B
///////////////////////////////////////////////////////////////////////////////
class EESCHEMA_TREE : public SEARCH_TREE
{
public:
    EESCHEMA_TREE( LIB_EDIT_FRAME* aParent );
    ~EESCHEMA_TREE();

    /**
     * Recreates the library list.
     */
    void Update() override;

    /**
     * Updates the tree contents using the state stored in LIB_MANAGER.
     */
    void Sync();

    /**
     * Reloads the entries for a library.
     */
    void UpdateLibrary( const wxString& aLibName );

    /**
     * Assigns the LIB_MANAGER object and refreshes the contents.
     */
    void LoadFootprints( LIB_MANAGER* aLibManager );

    /**
     * Returns true if the right-click context menu is on the screen.
     */
    bool IsContextMenuActive() const
    {
        return m_menuActive;
    }

protected:
    void OnLeftDClick( wxMouseEvent& aEvent ) override;
    void OnRightClick( wxMouseEvent& aEvent ) override;

private:
    // Right-click context menus
    std::unique_ptr<wxMenu> m_menuLibrary, m_menuComponent;

    ///> Flag determining whether a right-click context menu is active
    bool m_menuActive;

    wxStaticText* m_staticFootCandText;
    wxListBox* m_footprintCandidates;
    LIB_MANAGER* m_libMgr;
};

#endif //__DIALOG_FOOTPRINTS_TREE_H__
