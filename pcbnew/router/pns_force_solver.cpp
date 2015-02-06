/*
 * KiRouter - a push-and-(sometimes-)shove PCB router
 *
 * Copyright (C) 2013-2015 CERN
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

#include "pns_node.h"
#include "pns_via.h"
#include "pns_router.h"
#include "pns_force_solver.h"

PNS_FORCE_SOLVER::PNS_FORCE_SOLVER ( PNS_ROUTER *aRouter, PNS_NODE *aNode ) :
	PNS_ALGO_BASE ( aRouter ),
	m_collisionMask ( PNS_ITEM::ANY ),
	m_iterationLimit ( 10 ),
	m_compoundMode ( true ),
	m_compoundClearance ( -1 ),
	m_world ( aNode )
{

}

PNS_FORCE_SOLVER::~PNS_FORCE_SOLVER()
{

}

void PNS_FORCE_SOLVER::SetCollisionMask ( int aCollisionMask )
{
	m_collisionMask = aCollisionMask;
}

void PNS_FORCE_SOLVER::SetIterationLimit ( int aIterationLimit )
{
	m_iterationLimit = aIterationLimit;
}

void PNS_FORCE_SOLVER::SetCompoundMode ( bool aCompoundMode )
{
	m_compoundMode = aCompoundMode;
}
void PNS_FORCE_SOLVER::SetCompoundClearance ( int aCompoundClearance )
{
	m_compoundClearance = aCompoundClearance;
}

void PNS_FORCE_SOLVER::Add( PNS_ITEM *aItem )
{
	assert (aItem->OfKind( PNS_ITEM::VIA) );

//	m_items.aCompoundMode ( aItem );
}


bool PNS_FORCE_SOLVER::Solve ()
{
	int iter = 0;
	
    VECTOR2I totalForce;

    while( iter < m_iterationLimit )
    {
    	PNS_NODE::OBSTACLES obstacles;

       	BOOST_FOREACH ( PNS_ITEM *item, m_items.Items() )
       	{
       		PNS_NODE::OPT_OBSTACLE obs = m_world->CheckColliding( item, m_collisionMask );

       		if(obs)
       			obstacles.push_back ( *obs );
       	}

        if( obstacles.empty() )
            break;

		VECTOR2I worstForce;
		int worstMagnitude = std::numeric_limits<int>::min();

        BOOST_FOREACH(PNS_OBSTACLE &obs, obstacles )
        {
        	VECTOR2I f;
            int clearance = m_world->GetClearance( obs.m_item, obs.m_head );

	        bool col = CollideShapes( obs.m_item->Shape(), obs.m_head->Shape(), clearance, true, f );

	        if(col)
	        {
	        	int l = f.EuclideanNorm();
	        	if( l > worstMagnitude )
	        	{
	        		worstForce = f;
	        		worstMagnitude = l;
	        	}
	        }
        }

		totalForce += worstForce;

		BOOST_FOREACH ( PNS_ITEM *item, m_items.Items() )
		{
			PNS_VIA *v = static_cast<PNS_VIA *> (item); // fixme: support other things than vias
			v->SetPos ( v->Pos() + worstForce );
		}


#if 0
        if( iter > aMaxIterations / 2 )
        {
            VECTOR2I l = aDirection.Resize( m_diameter / 2 );
            totalForce += l;
            mv.SetPos( mv.Pos() + l );
        }
#endif




/*        if(m_compoundMode)
        {
        	PNS_ITEM *worst = NULL;

        }*/

        iter++;
    }

	printf("iter: %d\n", iter);
    if(iter != m_iterationLimit)
    	return true;

    return false;


    #if 0

        if( iter > aMaxIterations / 2 )
        {
            VECTOR2I l = aDirection.Resize( m_diameter / 2 );
            totalForce += l;
            mv.SetPos( mv.Pos() + l );
        }

        bool col = CollideShapes( obs->m_item->Shape(), mv.Shape(), clearance, true, force2 );

        if( col ) {
            totalForce += force2;
            mv.SetPos( mv.Pos() + force2 );
        }

        iter++;
    }

    if( iter == aMaxIterations )
        return false;

    aForce = totalForce;

    return true;
#endif

}
