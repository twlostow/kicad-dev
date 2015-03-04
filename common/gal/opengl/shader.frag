/*
 * This program source code file is part of KICAD, a free EDA CAD application.
 *
 * Copyright (C) 2013 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
 *
 * Fragment shader
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

#version 120

// Shader types
const float SHADER_LINE                 = 1.0f;
const float SHADER_FILLED_CIRCLE        = 2.0f;
const float SHADER_STROKED_CIRCLE       = 3.0f;
const float SHADER_PIXEL_LINE_R0        = 4.0f;
const float SHADER_PIXEL_LINE_R1        = 5.0f;

varying vec4 shaderParams;
varying vec2 circleCoords;


void filledCircle( vec2 aCoord )
{
    if( dot( aCoord, aCoord ) < 1.0f )
        gl_FragColor = gl_Color;
else
        discard;
}


void strokedCircle( vec2 aCoord, float aRadius, float aWidth )
{
    float outerRadius = aRadius + ( aWidth / 2 );
    float innerRadius = aRadius - ( aWidth / 2 );
    float relWidth = innerRadius / outerRadius;

    if( ( dot( aCoord, aCoord ) < 1.0f ) &&
        ( dot( aCoord, aCoord ) > relWidth * relWidth ) )
        gl_FragColor = gl_Color;
    else
        discard;
}

float DistToLine(vec2 pt1, vec2 pt2, vec2 testPt)
{
  vec2 lineDir = pt2 - pt1;
  vec2 perpDir = vec2(lineDir.y, -lineDir.x);
  vec2 dirToPt1 = pt1 - testPt;
  return abs(dot(normalize(perpDir), dirToPt1));
}

void main()
{
    if( shaderParams[0] == SHADER_FILLED_CIRCLE )
        filledCircle( circleCoords );
    else if ( shaderParams [0] == SHADER_STROKED_CIRCLE )
        strokedCircle( circleCoords, shaderParams[2], shaderParams[3] );
    else if ( shaderParams [0] == SHADER_PIXEL_LINE_R0)
        gl_FragColor = vec4(gl_Color.rgb, gl_Color.a * gl_TexCoord[0].y);
    else if ( shaderParams [0] == SHADER_PIXEL_LINE_R1)
    {
        float ratio = abs(shaderParams[1]), d = abs(gl_TexCoord[0].y);
        float a ;
        if ( d < ratio )
            a = 1.0;
        else
            a = 1.0 - ( d - ratio ) / ( 1.0 - ratio );

        
        gl_FragColor = vec4(gl_Color.rgb, a);
    } else
    {
        // Simple pass-through
        gl_FragColor = gl_Color;
    }
}
    