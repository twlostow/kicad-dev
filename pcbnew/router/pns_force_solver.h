/*
 * KiRouter - a push-and-(sometimes-)shove PCB router
 *
 * Copyright (C) 2013-2014 CERN
 * Author: Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef __PNS_FORCE_SOLVER_H
#define __PNS_FORCE_SOLVER_H

#include <boost/optional.hpp>

#include "pns_algo_base.h"
#include "pns_itemset.h"

class PNS_ROUTER;
class PNS_ITEM;
class PNS_NODE;

class PNS_FORCE_SOLVER : public PNS_ALGO_BASE
{
public:
	PNS_FORCE_SOLVER ( PNS_ROUTER *aRouter, PNS_NODE *aNode );
	~PNS_FORCE_SOLVER();

	void SetCollisionMask ( int aCollisionMask );
	void SetIterationLimit ( int aIterationLimit );
	void SetCompoundMode ( bool aCompoundMode );
	void SetRestrict45Degree ( bool aDiagonalAlign );
	void SetCompoundClearance ( int aCompoundClearance );
	void SetPreferredDirection ( const VECTOR2I& aPrefDir );

	void Add ( PNS_ITEM *aItem );
	bool Solve ();
	
	PNS_ITEMSET& Items() 
	{
		return m_items;
	}

private:
	int m_collisionMask;
	int m_iterationLimit;
	int m_compoundClearance;
	bool m_compoundMode;
	bool m_restrict45deg;
	boost::optional<VECTOR2I> m_preferredDir;

	PNS_NODE *m_world;
	PNS_ITEMSET m_items;

};

#endif
