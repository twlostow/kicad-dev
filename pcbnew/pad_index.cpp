/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 Jean-Pierre Charras, jean-pierre.charras@ujf-grenoble.fr
 * Copyright (C) 2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 1992-2012 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <class_board.h>
#include <class_pad.h>
#include <class_module.h>

#include <legacy_ratsnest.h>
#include <pad_index.h>
#include <class_netinfo.h>

#include <algorithm>

PAD_INDEX::PAD_INDEX ( BOARD *aBoard ) : 
	m_board ( aBoard )
{
	Rebuild();
}

PAD_INDEX::~PAD_INDEX ( )
{

}

static bool padlistSortByNetnames( const D_PAD* a, const D_PAD* b )
{
    return ( a->GetNetname().Cmp( b->GetNetname() ) ) < 0;
}

void PAD_INDEX::Rebuild ()
{
    /*
     * initialize:
     *   m_pads (list of pads)
     * set m_Status_Pcb = LISTE_PAD_OK;
     * also clear legacy ratsnest that could have bad data
     *   (legacy Full Ratsnest uses pointer to pads)
     * Be aware NETINFO_ITEM* BOARD::FindNet( const wxString& aNetname )
     * when search a net by its net name does a binary search
     * and expects to have a nets list sorted by an alphabetic case sensitive sort
     * So do not change the sort function used here
     */

    if( m_board->GetStatus() & LISTE_PAD_OK )
        return;

    m_pads.clear();
    m_board->GetLegacyRatsnest()->Clear();

    // Clear variables used in ratsnest computation
    for( MODULE* module = m_board->m_Modules;  module;  module = module->Next() )
    {
        for( D_PAD* pad = module->Pads();  pad;  pad = pad->Next() )
        {
            m_pads.push_back( pad );

            pad->SetSubRatsnest( 0 );
            pad->SetParent( module );
        }
    }

    // Sort pad list per net
    std::sort( m_pads.begin(), m_pads.end(), padlistSortByNetnames );

	m_padsByNet.clear();
	m_nodeCount = 0;

	// Assign pads to appropriate map entries
    for( unsigned ii = 0; ii < m_pads.size(); ii++ )
    {
        D_PAD *pad = m_pads[ii];
        int netCode = pad->GetNetCode();
        if( netCode == NETINFO_LIST::UNCONNECTED ) // pad not connected
            continue;

        // Add pad to the appropriate list of pads
        NETINFO_ITEM* net = pad->GetNet();
        // it should not be possible for BOARD_CONNECTED_ITEM to return NULL as a result of GetNet()
        wxASSERT( net );

        if( net )
        	m_padsByNet[ netCode ].push_back( pad );
            
        m_nodeCount++;
    }

    m_board->SetStatus ( LISTE_PAD_OK );
}

PAD_INDEX::PADS& PAD_INDEX::AllPadsInNet( const NETINFO_ITEM *aNet )
{
	return m_padsByNet[ aNet->GetNet() ];
}

unsigned int PAD_INDEX::CountNodesInNet( const NETINFO_ITEM *aNet )
{
	int netcode = aNet->GetNet();
	if ( m_padsByNet.find ( netcode ) == m_padsByNet.end() )
		return 0;
	else
		return m_padsByNet[ netcode ].size();	
}

unsigned int PAD_INDEX::CountNodesInNet( int aNetcode )
{
	if ( m_padsByNet.find ( aNetcode ) == m_padsByNet.end() )
		return 0;
	else
		return m_padsByNet[ aNetcode ].size();	
}