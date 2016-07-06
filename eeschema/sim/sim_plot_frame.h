/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 CERN
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
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

#ifndef __sim_plot_frame__
#define __sim_plot_frame__

/**
@file Subclass of SIM_PLOT_FRAME_BASE, which is generated by wxFormBuilder. */

#include "sim_plot_frame_base.h"
#include "kiway_player.h"
#include <netlist_exporters/netlist_exporter_pspice.h>

#include <wx/event.h>

class SPICE_SIMULATOR;
class NETLIST_EXPORTER_PSPICE;
class SIM_PLOT_PANEL;

/** Implementing SIM_PLOT_FRAME_BASE */
class SIM_PLOT_FRAME : public SIM_PLOT_FRAME_BASE
{
    public:
        /** Constructor */
        SIM_PLOT_FRAME( KIWAY* aKiway, wxWindow* aParent );
        ~SIM_PLOT_FRAME();

        void SetSchFrame( SCH_EDIT_FRAME* aSchFrame )
        {
            m_schematicFrame = aSchFrame;
        }

        void StartSimulation();
        void StopSimulation();

        void NewPlotPanel();
        void AddVoltagePlot( const wxString& aNetName );

        SIM_PLOT_PANEL* CurrentPlot() const;

    private:
        bool isSimulationRunning();

        /**
         * @brief Updates plot in a particular SIM_PLOT_PANEL. If the panel does not contain
         * the plot, it will be added.
         * @param aSpiceName is the plot name in the format accepted by the current simulator instance
         * (for NGSPICE it is e.g. "V(1)").
         * @param aName is the name used in the legend.
         * @param aPanel is the panel that should receive the update.
         */
        void updatePlot( const wxString& aSpiceName, const wxString& aName, SIM_PLOT_PANEL* aPanel );

        /**
         * @brief Returns node number for a given net.
         * @param aNetName is the net number.
         * @return Corresponding net number or -1 if there is no such net.
         */
        int getNodeNumber( const wxString& aNetName );

        // Menu handlers
        void menuNewPlot( wxCommandEvent& aEvent ) override
        {
            NewPlotPanel();
        }

        void menuExit( wxCommandEvent& event ) override
        {
            Close();
        }

        void menuZoomIn( wxCommandEvent& event ) override;
        void menuZoomOut( wxCommandEvent& event ) override;
        void menuZoomFit( wxCommandEvent& event ) override;
        void menuShowGrid( wxCommandEvent& event ) override;
        void menuShowGridState( wxUpdateUIEvent& event ) override;

        // Event handlers
        void onSignalDblClick( wxCommandEvent& event ) override;
        void onSignalRClick( wxMouseEvent& event ) override;
        void onCursorsUpdate( wxUpdateUIEvent& event ) override;

        void onSimulate( wxCommandEvent& event ) override;
        void onPlaceProbe( wxCommandEvent& event ) override;

        void onClose( wxCloseEvent& aEvent );

        void onSimStarted( wxCommandEvent& aEvent );
        void onSimFinished( wxCommandEvent& aEvent );
        void onSimReport( wxCommandEvent& aEvent );

        SCH_EDIT_FRAME* m_schematicFrame;
        NETLIST_EXPORTER_PSPICE* m_exporter;
        SPICE_SIMULATOR* m_simulator;

        // Right click context menu for signals in the listbox
        class SIGNAL_CONTEXT_MENU : public wxMenu
        {
            public:
                SIGNAL_CONTEXT_MENU( const wxString& aSignal, SIM_PLOT_FRAME* aPlotFrame );

            private:
                void onMenuEvent( wxMenuEvent& aEvent );

                const wxString& m_signal;
                SIM_PLOT_FRAME* m_plotFrame;

                enum SIGNAL_CONTEXT_MENU_EVENTS
                {
                    SHOW_SIGNAL,
                    HIDE_SIGNAL,
                    SHOW_CURSOR,
                    HIDE_CURSOR
                };
        };
};

wxDEFINE_EVENT( wxEVT_SIM_REPORT, wxCommandEvent );
wxDEFINE_EVENT( wxEVT_SIM_STARTED, wxCommandEvent );
wxDEFINE_EVENT( wxEVT_SIM_FINISHED, wxCommandEvent );

#endif // __sim_plot_frame__
