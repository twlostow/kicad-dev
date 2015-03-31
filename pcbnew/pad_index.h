/**
 * @brief PAD_INDEX class, indexing pads by nets and nets by pads ;)
 */

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


#ifndef __PAD_INDEX_H
#define __PAD_INDEX_H

#include <vector>
#include <map>

class BOARD;
class D_PAD;
class NETINFO_ITEM;

/**
 * Class PAD_INDEX
 *
 * Holds an index of all pads in the design. The pads are stored in two indices:
 * - global one, sorted by net name
 * - per-net one, storing all pads in a given net.
 * The index is rebuilt upon demand by calling Rebuild() when the parent
 * model is changed.
 */
class PAD_INDEX {
public:
    typedef std::vector<D_PAD*> PADS;

    PAD_INDEX( BOARD* aBoard );
    ~PAD_INDEX( );

    ///> returns a pad at index aIndex in the global table
    D_PAD *GetPad ( int aIndex ) const
    {
        if( aIndex < (int) m_pads.size() )
            return m_pads[aIndex];
        else
            return NULL;
    }

    ///> returns a vector of all pads in all nets
    PADS& AllPads()
    {
        return m_pads;
    }

    ///> returns a vector of all pads belonging to a particular net
    PADS& AllPadsInNet( int aNetCode )
    {
        return m_padsByNet[aNetCode];
    }

    ///> returns a vector of all pads belonging to a particular net
    PADS& AllPadsInNet( const NETINFO_ITEM* aNet );

    ///> returns the count of pads in the design
    unsigned int Size( ) const
    {
        return m_pads.size();
    }

    ///> returns the number of nodes (i.e. connected pads)
    ///> in a given net
    unsigned int CountNodesInNet( const NETINFO_ITEM* aNet );
    unsigned int CountNodesInNet( int aNetcode );

    D_PAD *operator[] ( int aIndex ) const
    {
        return GetPad( aIndex );
    }

    ///> returns the number of nodes (Kicad term for connected pads)
    int GetNodeCount() const
    {
        return m_nodeCount;
    }

    ///> rebuilds pad index from the parent BOARD object
    void Rebuild( );

private:
    BOARD*              m_board;
    std::vector<D_PAD*> m_pads;
    std::map<int, PADS> m_padsByNet;
    int                 m_nodeCount;
};


#endif
