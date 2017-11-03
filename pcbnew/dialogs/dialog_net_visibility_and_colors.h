/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 CERN
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

#include <dialogs/dialog_net_visibility_and_colors_base.h>
#include <layers_id_colors_and_visibility.h>

class CONNECTIVITY_DATA;
class PCB_EDIT_FRAME;
class NET_COLOR_MODEL;

class DIALOG_NET_VISIBILITY_AND_COLORS : public DIALOG_NET_VISIBILITY_AND_COLORS_BASE
{
public:
    DIALOG_NET_VISIBILITY_AND_COLORS( PCB_EDIT_FRAME* aParent );

    bool TransferDataFromWindow() override;
    bool TransferDataToWindow() override;

private:
    void onClose( wxCloseEvent& aEvent ) override;
//    void onTrackNetclassCheck( wxCommandEvent& aEvent ) override;
//    void onViaNetclassCheck( wxCommandEvent& aEvent ) override;
    void onCancelClick( wxCommandEvent& aEvent ) override;
    void onOkClick( wxCommandEvent& aEvent ) override;
    void onCtrlItemContextMenu( wxDataViewEvent& event ) override;
    void onCtrlItemActivated( wxDataViewEvent& event ) override;

    void OnInitDlg( wxInitDialogEvent& event ) override
    {
        // Call the default wxDialog handler of a wxInitDialogEvent
        TransferDataToWindow();

        // Now all widgets have the size fixed, call FinishDialogSettings
        FinishDialogSettings();
        Layout();
    }

    PCB_EDIT_FRAME *m_frame;
    NET_COLOR_MODEL* m_model;
    wxDataViewColumn *m_colName;
    wxDataViewColumn *m_colVisible;
    wxDataViewColumn *m_colColor;

};
