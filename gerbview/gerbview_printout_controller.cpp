/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2009 Jean-Pierre Charras, jean-pierre.charras@ujf-grenoble.fr
 * Copyright (C) 1992-2011 KiCad Developers, see AUTHORS.txt for contributors.
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

/**
 * @file gerbview_printout_controller.cpp
 * @brief Gerbview print handler implementation file.
 */

#include <fctsys.h>
#include <appl_wxstruct.h>
#include <class_drawpanel.h>
#include <base_units.h>
#include <layers_id_colors_and_visibility.h>

#include <gerbview_frame.h>
#include <gerbview_printout_controller.h>


GERBVIEW_PRINTOUT_CONTROLLER::GERBVIEW_PRINTOUT_CONTROLLER( const PRINT_PARAMETERS& aParams,
                                                      EDA_DRAW_FRAME*         aParent,
                                                      const wxString&         aTitle ) :
    PRINTOUT_CONTROLLER ( aParams, aParent, aTitle ) 
{
    SetUnits ( &g_GerbviewUnits );

};

LAYER_NUM GERBVIEW_PRINTOUT_CONTROLLER::getAppLayerCount() const
{
    return NB_LAYERS;
}

wxString GERBVIEW_PRINTOUT_CONTROLLER::getTitleBlockText()
{
    return wxT("Gerbview Printout");
}

EDA_RECT GERBVIEW_PRINTOUT_CONTROLLER::getBoundingBox()
{
    return ((GERBVIEW_FRAME*) m_Parent)->GetGerberLayoutBoundingBox();
}

void GERBVIEW_PRINTOUT_CONTROLLER::applyDrawingHacks()
{
    GRForceBlackPen( false );
}

