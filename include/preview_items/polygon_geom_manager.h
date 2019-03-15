/*
 * This program source code file is part of KICAD, a free EDA CAD application.
 *
 * Copyright (C) 2017 Kicad Developers, see AUTHORS.txt for contributors.
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

#ifndef PREVIEW_POLYGON_GEOM_MANAGER__H_
#define PREVIEW_POLYGON_GEOM_MANAGER__H_

#include <memory>
#include <geometry/shape_line_chain.h>

class OUTLINE_SHAPE_BUILDER;


/**
 * Class that handles the drawing of a polygon, including
 * management of last corner deletion and drawing of leader lines
 * with various constraints (eg 45 deg only).
 *
 * This class handles only the geometry of the process.
 */
class POLYGON_GEOM_MANAGER
{
public:

    /**
     * "Listener" interface for a class that wants to be updated about
     * polygon geometry changes
     */
    class CLIENT
    {
    public:
        /**
         * Called before the first point is added - clients can do
         * initialisation here, and can veto the start of the process
         * (e.g. if user cancels a dialog)
         *
         * @return false to veto start of new polygon
         */
        virtual bool OnFirstPoint( POLYGON_GEOM_MANAGER& aMgr ) = 0;

        ///> Sent when the polygon geometry changes
        virtual void OnGeometryChange( const POLYGON_GEOM_MANAGER& aMgr ) = 0;

        ///> Called when the polygon is complete
        virtual void OnComplete( const POLYGON_GEOM_MANAGER& aMgr ) = 0;
    };

    
    /**
     * @param aClient is the client to pass the results onto
     */
    POLYGON_GEOM_MANAGER( CLIENT& aClient );

    /**
     * Lock in a polygon point.
     */
    bool AddPoint( const VECTOR2I& aPt );

    /**
     * Mark the polygon finished and update the client
     */
    void SetFinished();

    /**
     * Clear the manager state and start again
     */
    void Reset();

    /**
     * Enables/disables self-intersecting polygons.
     * @param aEnabled true if self-intersecting polygons are enabled.
     */
    void AllowIntersections( bool aEnabled )
    {
        m_intersectionsAllowed = true;
    }

    /**
     * Checks whether self-intersecting polygons are enabled.
     * @return true if self-intersecting polygons are enabled.
     */
    bool IntersectionsAllowed() const
    {
        return m_intersectionsAllowed;
    }

    /**
     * Checks whether the locked points constitute a self-intersecting outline.
     * @param aIncludeLeaderPts when true, also the leading points (not placed ones) will be tested.
     * @return True when the outline is self-intersecting.
     */
    bool IsSelfIntersecting( bool aIncludeLeaderPts ) const;

    /**
     * Set the current cursor position
     */
    void SetCursorPosition( const VECTOR2I& aPos );

    /**
     * @return true if the polygon in "in progress", i.e. it has at least
     * one locked-in point
     */
    bool IsPolygonInProgress() const;

    /**
     * @return true if locking in the given point would close the
     * current polygon
     */
    bool NewPointClosesOutline( const VECTOR2I& aPt ) const;

    /**
     * Remove the last-added point from the polygon
     */
    void DeleteLastCorner();

    /* =================================================================
     * Interfaces for users of the geometry
     */

    /**
     * Get the "locked-in" points that describe the polygon itself
     */
    const SHAPE_LINE_CHAIN& GetLockedInPoints() const
    {
        return m_lockedPoints;
    }

    /**
     * Get the points comprising the leader line (the line from the
     * last locked-in point to the current cursor position
     */
    const SHAPE_LINE_CHAIN& GetLeaderLinePoints() const
    {
        return m_leaderPts;
    }

    OUTLINE_SHAPE_BUILDER* GetOutlineBuilder() const;

private:

    /**
     * Update the leader line points based on a new endpoint (probably
     * a cursor position)
     */
    void updateLeaderPoints( const VECTOR2I& aEndPoint );

    ///> The "user" of the polygon data that is informed when the geometry changes
    CLIENT& m_client;

    ///> Shape builder object
    std::unique_ptr<OUTLINE_SHAPE_BUILDER> m_shapeBuilder;

    ///> Flag enabling self-intersecting polygons
    bool m_intersectionsAllowed;

    ///> Point that have been "locked in"
    SHAPE_LINE_CHAIN m_lockedPoints;

    ///> Points in the temporary "leader" line(s)
    SHAPE_LINE_CHAIN m_leaderPts;
};

#endif // PREVIEW_POLYGON_GEOM_MANAGER__H_
