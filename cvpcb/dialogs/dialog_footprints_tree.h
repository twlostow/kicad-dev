/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 CERN
 * @author Michele Castellana <michele.castellana@cern.ch>
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
#include <footprint_info.h>

class COMPONENT;
class KIWAY;

///////////////////////////////////////////////////////////////////////////////
/// Class FOOTPRINTS_TREE
/// > library_name_1
///   > footprint_1
///   > footprint_2
///   > footprint_3
///   > footprint_4
/// > library_name_2
///   > footprint_1
///   > footprint_2
///   > footprint_3
///   > footprint_4
/// > library_name_3
///   > footprint_1
///   > footprint_2
///   > footprint_3
///   > footprint_4
///////////////////////////////////////////////////////////////////////////////
class FOOTPRINTS_TREE : public SEARCH_TREE
{
public:
    FOOTPRINTS_TREE( KIWAY* aKiway, wxWindow* aParent );
    ~FOOTPRINTS_TREE();

    void LoadFootprints( FP_LIB_TABLE* aList );
    void OnEditFootprintLibraryTable( wxCommandEvent& aEvent );

    void Update() override;
    wxArrayTreeItemIds GetSelected() const override;

protected:
    void OnLeftDClick( wxMouseEvent& aEvent ) override;

private:
    std::unique_ptr<FOOTPRINT_LIST> m_footprints;
};

#endif //__DIALOG_FOOTPRINTS_TREE_H__
