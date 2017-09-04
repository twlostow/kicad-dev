/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2017 CERN
 * Author: Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
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


#ifndef __CONSTRAINT_SOLVER_H
#define __CONSTRAINT_SOLVER_H

#include <vector>
#include <set>
#include <deque>

#include <math/vector2d.h>
#include <geometry/seg.h>
#include <geometry/point_set.h>

class BOARD_ITEM;
class GS_ITEM;
class DRAWSEGMENT;
class CONSTRAINT_LINEAR;
class GEOM_SOLVER;
class GEOM_PREVIEW;

enum GS_CONSTRAINT_TYPE
{
    CS_START_ANGLE = 1,
    CS_RADIUS = 2,
    CS_CENTRAL_ANGLE= 4,
    CS_LENGTH = 8,
    CS_DIRECTION = 16
};

enum GS_ITEM_TYPE
{
    GST_SEGMENT = 0,
    GST_ARC,
    GST_LINEAR_CONSTRAINT
};

class GS_SEGMENT;

class GS_ANCHOR
{
public:
    GS_ANCHOR() {};

    GS_ANCHOR( GS_ITEM* parent, int id, VECTOR2I pos )
        : m_parent( parent ),
        m_pos( pos ),
        m_id( id ),
        m_changedPos( false )
    {

    }

    GS_SEGMENT* NeighbourSegment( GS_SEGMENT *aCurrent );

    void SetPos( const VECTOR2I& aPos )
    {
        m_pos = aPos;
    }

    const VECTOR2I& GetPos() const
    {
        return m_pos;
    }

    void SetNextPos( const VECTOR2I& aPos )
    {
        m_nextPos = aPos;

        if( aPos != m_pos )
        {
            m_changedPos = true;
        }
    }

    const VECTOR2I& GetNextPos() const
    {
        return m_nextPos;
    }

    void UpdatePos()
    {
        m_pos = m_nextPos;
        m_changedPos = false;
    }

    bool PositionChanged() const
    {
        return m_changedPos;
    }

    GS_ITEM* GetParent() const
    {
        return m_parent;
    }

    void ClearLinks()
    {
        m_linkedAnchors.clear();
    }

    void Link( const GS_ANCHOR* aAnchor )
    {
        m_linkedAnchors.insert( const_cast<GS_ANCHOR*> ( aAnchor ) );
    }

    const std::set<GS_ANCHOR*>& GetLinks() const
    {
        return m_linkedAnchors;
    }

    int GetId() const
    {
        return m_id;
    }

    void SetConstrainable( bool aConstrainable )
    {
        m_constrainable = aConstrainable;
    }

    bool IsConstrainable() const { return m_constrainable; };

private:

    bool m_constrainable = true;
    int m_flags;
    GS_ITEM* m_parent;
    VECTOR2I m_pos;
    VECTOR2I m_nextPos;
    VECTOR2I m_offset;
    std::set<GS_ANCHOR*> m_linkedAnchors;
    int m_id;
    bool m_changedPos;
};

class GS_ITEM
{
public:
    GS_ITEM( GS_ITEM_TYPE aType, BOARD_ITEM* aParent = nullptr ) : m_type( aType ), m_parent(
                aParent ) {};
    virtual ~GS_ITEM() {};

    virtual void MoveAnchor( int aId,
            const VECTOR2I& aP,
            std::vector<GS_ANCHOR*>& aChangedAnchors ) = 0;

    virtual void    SaveState() = 0;
    virtual void    RestoreState()  = 0;
    virtual void    UpdateAnchors() = 0;
    virtual void Commit( BOARD_ITEM *aTarget = nullptr ) = 0;

    virtual bool IsSatisfied() const
    {
        return true;
    }

    const std::vector<GS_ANCHOR*>& GetAnchors() const { return m_anchors; }

    void SetPrimary( bool aPrimary )
    {
        m_primary = aPrimary;
    }

    bool IsPrimary() const
    {
        return m_primary;
    }

    GS_ITEM_TYPE Type() const
    {
        return m_type;
    }

    BOARD_ITEM* GetParent() const
    {
        return m_parent;
    }

    void Constrain( int aWhat, bool aEnabled )
    {
        m_constrain &= ~aWhat;

        if( aEnabled )
            m_constrain |= aWhat;
    }

    bool IsConstrained( int aWhat )
    {
        return (m_constrain & aWhat) == aWhat;
    }

protected:
    GS_ITEM_TYPE m_type;
    int m_constrain = 0;
    std::vector<GS_ANCHOR*> m_anchors;
    BOARD_ITEM* m_parent = nullptr;
    bool m_primary = false;
};

class GS_SEGMENT : public GS_ITEM
{
public:
    GS_SEGMENT( DRAWSEGMENT* aSeg );
    ~GS_SEGMENT();

    const VECTOR2I& GetStart() const { return m_p0; }
    const VECTOR2I& GetEnd() const { return m_p1; }

    virtual void MoveAnchor( int aId,
            const VECTOR2I& aP,
            std::vector<GS_ANCHOR*>& aChangedAnchors );
    virtual void    SaveState() override;
    virtual void    RestoreState() override;
    virtual void    UpdateAnchors() override;
    virtual void Commit( BOARD_ITEM *aTarget = nullptr ) override;

    const SEG GetSeg() const
    {
        return SEG (m_p0, m_p1);
    }

private:

    VECTOR2I m_p0, m_p1, m_dir, m_midpoint;
    VECTOR2I m_p0_saved, m_p1_saved, m_midpoint_saved;
};

class GS_LINEAR_CONSTRAINT : public GS_ITEM
{
public:
    GS_LINEAR_CONSTRAINT( CONSTRAINT_LINEAR* aConstraint );
    virtual void SaveState();
    virtual void RestoreState();
    virtual bool IsSatisfied() const override;
    virtual void UpdateAnchors();
    virtual void MoveAnchor( int aId,
            const VECTOR2I& aP,
            std::vector<GS_ANCHOR*>& aChangedAnchors ) override;

    virtual void Commit( BOARD_ITEM *aTarget = nullptr ) override;

    const VECTOR2I& GetP0() const { return m_p0; }
    const VECTOR2I& GetP1() const { return m_p1; }

    const VECTOR2I& GetOrigin() const { return m_origin; }

private:
    VECTOR2I m_p0, m_p1, m_dir, m_origin;
    VECTOR2I m_p0_saved, m_p1_saved, m_origin_saved;
};

class GEOM_SOLVER
{
    friend class GEOM_PREVIEW;
    struct DISPLACEMENT
    {
        DISPLACEMENT( GS_ANCHOR *aAnchor, VECTOR2I aP ) :
            anchor(aAnchor),
            p(aP)
            {};

        GS_ANCHOR *anchor;
        VECTOR2I p;
    };

public:
    GEOM_SOLVER();
    ~GEOM_SOLVER();


    void Clear();

    GS_ANCHOR *FindAnchor ( VECTOR2I pos, double snapRadius );
    void StartMove();

    bool ValidateContinuity();


    bool IsResultOK() const;
    bool MoveAnchor( GS_ANCHOR *refAnchor, VECTOR2I pos );

    void Add ( BOARD_ITEM *aItem, bool aPrimary = false );
    const std::vector<GS_ANCHOR*> AllAnchors();
    const std::vector<GS_ITEM*>& AllItems();


    bool findOutlineElements( GS_ITEM* aItem );
    void FindOutlines();

private:
    const int c_epsilon = 1;


    POINT_SET<GS_ANCHOR*> m_allAnchors;
    std::vector<GS_ANCHOR*> m_anchors;
    std::vector<GS_ITEM*> m_items;
    bool m_lastResultOk = false;

};

#endif
