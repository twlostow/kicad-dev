/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2017 CERN
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

#include <geometry/shape_segment.h>
#include <geometry/shape_line_chain.h>

const SHAPE_LINE_CHAIN SHAPE_SEGMENT::ConvertToPolyline( bool aWire, int aOffset, double aApproximationAccuracy )
{
    assert( aWire );

    SHAPE_LINE_CHAIN rv;

    rv.Append( m_seg.A );
    rv.Append( m_seg.B );

    return rv;
}

const std::string SHAPE_SEGMENT::Format() const
{
    std::stringstream ss;

    ss << "2 0 ";

    ss << m_seg.A.x << " " << m_seg.A.y << " ";
    ss << m_seg.B.x << " " << m_seg.B.y << " ";

    return ss.str();
}
