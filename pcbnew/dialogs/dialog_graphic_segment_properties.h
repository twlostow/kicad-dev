/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015-2017 CERN
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

 #ifndef __DIALOG_GRAPHIC_SEGMENT_PROPERTIES_H_
 #define __DIALOG_GRAPHIC_SEGMENT_PROPERTIES_H_

class SELECTION;
class COMMIT;

class PCB_BASE_FRAME;

#include <widgets/unit_binder.h>
#include <dialogs/dialog_graphic_segment_properties_base.h>

class DIALOG_GRAPHIC_SEGMENT_PROPERTIES : public DIALOG_GRAPHIC_SEGMENT_PROPERTIES_BASE
{
public:
    DIALOG_GRAPHIC_SEGMENT_PROPERTIES( PCB_BASE_FRAME* aParent, const SELECTION& aItems );

    ///> Applies values from the dialog to the selected items.
    bool Apply( COMMIT& aCommit );

private:
    void onClose( wxCloseEvent& aEvent ) override;
    void onCancelClick( wxCommandEvent& aEvent ) override;
    void onOkClick( wxCommandEvent& aEvent ) override;

    void OnInitDlg( wxInitDialogEvent& event ) override
    {
        // Call the default wxDialog handler of a wxInitDialogEvent
        TransferDataToWindow();

        // Now all widgets have the size fixed, call FinishDialogSettings
        FinishDialogSettings();
    }

    ///> Checks if the dialog values are correct.
    bool check() const;

    ///> Sets wxTextCtrl to the value stored in OPT<T> or "<...>" if it is not available.
    template<typename T>
        void setCommonVal( const OPT<T>& aVal, wxTextCtrl* aTxtCtrl, UNIT_BINDER& aBinder )
    {

        printf("SetCV %d\n", *aVal );
        if( aVal )
            aBinder.SetValue( *aVal );
        else
            aTxtCtrl->SetValue( "<...>" );
    }

    ///> Selected items to be modified.
    const SELECTION& m_items;

    UNIT_BINDER m_startX, m_startY;
    UNIT_BINDER m_endX, m_endY;
    UNIT_BINDER m_width;

};

#endif
