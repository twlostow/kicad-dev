/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2014 CERN
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
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

#ifndef __SCH_COMPONENT_PREVIEW_PANEL
#define __SCH_COMPONENT_PREVIEW_PANEL

#include <class_drawpanel_gal.h>

namespace KIGFX 
{
	class VIEW;
}

class LIB_COMPONENT;
class SCH_COMPONENT;

/**
 * Class SCH_COMPONENT_PREVIEW_PANEL
 * Simple preview panel for schematic components
 */

class SCH_COMPONENT_PREVIEW_PANEL : public EDA_DRAW_PANEL_GAL
{
public:
	SCH_COMPONENT_PREVIEW_PANEL( wxWindow *aParent );
	~SCH_COMPONENT_PREVIEW_PANEL();
	
	void PreviewComponent( LIB_COMPONENT *aComponent, int aUnit );

private:
	SCH_COMPONENT *m_currentComponent;
	
};

#endif
