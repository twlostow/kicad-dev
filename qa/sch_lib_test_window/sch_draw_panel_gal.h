/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2014 CERN
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

#ifndef SCH_DRAW_PANEL_GAL_H_
#define SCH_DRAW_PANEL_GAL_H_

#include <class_draw_panel_gal.h>
#include <layers_id_colors_and_visibility.h>

namespace KIGFX
{
    class WORKSHEET_VIEWITEM;
    class SCH_VIEW;
};

class COLORS_DESIGN_SETTINGS;
class LIB_PART;
class SCH_SHEET;

class SCH_DRAW_PANEL_GAL : public EDA_DRAW_PANEL_GAL
{
public:
    SCH_DRAW_PANEL_GAL( wxWindow* aParentWindow, wxWindowID aWindowId, const wxPoint& aPosition,
                        const wxSize& aSize, KIGFX::GAL_DISPLAY_OPTIONS& aOptions,
                        GAL_TYPE aGalType = GAL_TYPE_OPENGL );

    virtual ~SCH_DRAW_PANEL_GAL();

    /**
     * Function DisplayBoard FIXME
     * adds all items from the current board to the VIEW, so they can be displayed by GAL.
     * @param aBoard is the PCB to be loaded.
     */
     void DisplayComponent( const LIB_PART *aComponent );
     void DisplaySheet( const SCH_SHEET *aSheet );

    /**
     * Function UseColorScheme
     * Applies layer color settings.
     * @param aSettings are the new settings.
     */
    void UseColorScheme( const COLORS_DESIGN_SETTINGS* aSettings );

    ///> @copydoc EDA_DRAW_PANEL_GAL::OnShow()
    void OnShow() override;

    bool SwitchBackend( GAL_TYPE aGalType ) override;


protected:

    KIGFX::SCH_VIEW* view() const;

    ///> Reassigns layer order to the initial settings.
    void setDefaultLayerOrder();

    ///> Sets rendering targets & dependencies for layers.
    void setDefaultLayerDeps();
};

#endif /* PCB_DRAW_PANEL_GAL_H_ */
