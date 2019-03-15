/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013-2017 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef __OUTLINE_EDITOR_H
#define __OUTLINE_EDITOR_H

#include <tools/pcb_tool.h>
#include <memory>

namespace KIGFX
{
    class VIEW_GROUP;
};

class SELECTION_TOOL;
class ANCHOR;

class GS_ANCHOR;
class GS_SOLVER;
class GEOM_PREVIEW;

/**
 * Class POINT_EDITOR
 *
 * Tool that displays edit points allowing to modify items by dragging the points.
 */
class OUTLINE_EDITOR : public PCB_TOOL
{
public:
    OUTLINE_EDITOR();
    ~OUTLINE_EDITOR();

    /// @copydoc TOOL_INTERACTIVE::Reset()
    void Reset( RESET_REASON aReason ) override;

    /// @copydoc TOOL_INTERACTIVE::Init()
    bool Init() override;

    /**
     * Function OnSelected()
     *
     * Change selection event handler.
     */
    int OnSelectionChange( const TOOL_EVENT& aEvent );
    int ChamferCorner( const TOOL_EVENT& aEvent );
    int FilletCorner( const TOOL_EVENT& aEvent );
    int BreakOutline( const TOOL_EVENT& aEvent );

    ///> Sets up handlers for various events.
    void setTransitions() override;

private:
    ///> Updates which point is being edited.
    void updateEditedAnchor( const TOOL_EVENT& aEvent );
    void updateOutline();
    void addToSolver( BOARD_ITEM* aItem, bool aPrimary );

    ///> Sets the current point being edited. NULL means none.
    void setEditedAnchor( GS_ANCHOR* aAnchor );
    int modifiedSelection( const TOOL_EVENT& aEvent );

    std::shared_ptr<GS_SOLVER> m_solver;
    std::shared_ptr<GEOM_PREVIEW> m_geomPreview;

    GS_ANCHOR* m_editedAnchor = nullptr;

    ///> Selection tool used for obtaining selected items
    SELECTION_TOOL* m_selectionTool;

};

#endif
