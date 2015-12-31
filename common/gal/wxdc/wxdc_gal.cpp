/*
 * This program source code file is part of KICAD, a free EDA CAD application.
 *
 * Copyright (C) 2012 Torsten Hueter, torstenhtr <at> gmx.de
 * Copyright (C) 2012 Kicad Developers, see change_log.txt for contributors.
 *
 * WXDC_GAL - wxDC implementation of the Graphics Abstraction Layer
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

#include <gal/wxdc/wxdc_gal.h>
#include <SutherlandHodgmanClipPoly.h>
#include <cmath>

using namespace std;
namespace KIGFX {

WXDC_GAL::WXDC_GAL( wxWindow *aParent, wxEvtHandler* aMouseListener, wxEvtHandler* aPaintListener,
                    const wxString& aName ) :
                    wxWindow( aParent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxEXPAND, aName )
{
    // Default values
    fillColor   = COLOR4D( 0, 0, 0, 1 );
    strokeColor = COLOR4D( 1, 1, 1, 1 );

    screenSize = VECTOR2D( 20, 20 );   // window will be soon resized

    parentWindows = aParent;
    mouseListener = aMouseListener;
    paintListener = aPaintListener;

    isGrouping = false;

    zoomFactor = 1.0;

    // Connecting the event handlers
    // Mouse events are skipped to the parent
    Connect( wxEVT_MOTION,          wxMouseEventHandler( WXDC_GAL::skipMouseEvent ) );
    Connect( wxEVT_LEFT_DOWN,       wxMouseEventHandler( WXDC_GAL::skipMouseEvent ) );
    Connect( wxEVT_LEFT_UP,         wxMouseEventHandler( WXDC_GAL::skipMouseEvent ) );
    Connect( wxEVT_LEFT_DCLICK,     wxMouseEventHandler( WXDC_GAL::skipMouseEvent ) );
    Connect( wxEVT_MIDDLE_DOWN,     wxMouseEventHandler( WXDC_GAL::skipMouseEvent ) );
    Connect( wxEVT_MIDDLE_UP,       wxMouseEventHandler( WXDC_GAL::skipMouseEvent ) );
    Connect( wxEVT_MIDDLE_DCLICK,   wxMouseEventHandler( WXDC_GAL::skipMouseEvent ) );
    Connect( wxEVT_RIGHT_DOWN,      wxMouseEventHandler( WXDC_GAL::skipMouseEvent ) );
    Connect( wxEVT_RIGHT_UP,        wxMouseEventHandler( WXDC_GAL::skipMouseEvent ) );
    Connect( wxEVT_RIGHT_DCLICK,    wxMouseEventHandler( WXDC_GAL::skipMouseEvent ) );
    Connect( wxEVT_MOUSEWHEEL,      wxMouseEventHandler( WXDC_GAL::skipMouseEvent ) );
#if defined _WIN32 || defined _WIN64
    Connect( wxEVT_ENTER_WINDOW,    wxMouseEventHandler( WXDC_GAL::skipMouseEvent ) );
#endif

    // Initialize line attributes map

    isDeleteSavedPixels = false;

    isGrouping = false;

    initCursor( 20 );

    allocateBitmaps();

    transformation = worldScreenMatrix;

    // Set grid defaults
    SetGridColor( COLOR4D( 0.3, 0.3, 0.3, 0.3 ) );
    SetCoarseGrid( 10 );
    SetGridLineWidth( 1.0 );
}


WXDC_GAL::~WXDC_GAL()
{
    // TODO Deleting of list contents like groups and paths
    delete wxbitmap;
}

void WXDC_GAL::ResizeScreen( int aWidth, int aHeight )
{
    screenSize = VECTOR2I( aWidth, aHeight );

    SetSize( wxSize( aWidth, aHeight ) );

    transformation = worldScreenMatrix;
}

bool WXDC_GAL::Show( bool aShow )
{
    bool s = wxWindow::Show( aShow );

    if( aShow )
        wxWindow::Raise();

    return s;
}





void WXDC_GAL::onPaint( wxPaintEvent& aEvent )
{
    PostPaint();
}


void WXDC_GAL::onSize( wxSizeEvent& aEvent )
{
    wxSize newSize = aEvent.GetSize();

    // Assign the new size
    screenSize = VECTOR2D( newSize.x, newSize.y );

    // Delete old bitmaps and create new ones
    delete wxbitmap;
    delete wxbitmapBackup;
    allocateBitmaps();

    PostPaint();
}


void WXDC_GAL::skipMouseEvent( wxMouseEvent& aEvent )
{
    // Post the mouse event to the event listener registered in constructor, if any
    if( mouseListener )
    {
        wxPostEvent( mouseListener, aEvent );
    }
}


void WXDC_GAL::BeginDrawing() throw( int )
{
    // The size of the client area needs to be greater than zero
    clientRectangle = parentWindows->GetClientRect();

    if( clientRectangle.width == 0 || clientRectangle.height == 0 )
    {
        throw EXCEPTION_ZERO_CLIENT_RECTANGLE;
    }

    printf("beginDraw: %dx%x\n", clientRectangle.width, clientRectangle.height );

    //clientDC = new wxClientDC( this );
    dc = new wxBufferedPaintDC( this );

    // Clear the screen
    ClearScreen();

    // Compute the world <-> screen transformations
    ComputeWorldScreenMatrix();


    // Start drawing with a new path
    isElementAdded = true;

    lineWidth = 0;

    isDeleteSavedPixels = true;

//    dc->SetLogicalFunction( wxOR );
}

void WXDC_GAL::ComputeWorldScreenMatrix()
{
    GAL::ComputeWorldScreenMatrix();
    transformation = worldScreenMatrix;
}


void WXDC_GAL::EndDrawing()
{
    // Force remaining objects to be drawn
    printf("EndDrawign!\n");
    Flush();
    //delete ( dc );
    //wxClientDC client_dc( this );
    //wxBufferedDC dc( &client_dc );
    //dc.DrawBitmap( *wxbitmap, 0, 0 );
    delete ( dc ); //clientDC );
}


void WXDC_GAL::SaveScreen()
{
    wxMemoryDC dc( *wxbitmapBackup );
    dc.DrawBitmap( *wxbitmap, 0, 0, false );
}

void WXDC_GAL::RestoreScreen()
{
    wxMemoryDC dc( *wxbitmap );
    dc.DrawBitmap( *wxbitmapBackup, 0, 0, false );
}


int WXDC_GAL::computeOutCode( VECTOR2D aPoint, double offset )
{
    int code;
    // We first assume that the point is inside the clip window
    code = INSIDE;

    if ( aPoint.x + offset < 0 )
    { // Left of the clip window
        code |= LEFT;
    }
    else if ( aPoint.x - offset > screenSize.x )
    { // Right of clip window
        code |= RIGHT;
    }
    if ( aPoint.y + offset < 0 )
    { // Below the clip window
        code |= BOTTOM;
    }
    else if ( aPoint.y - offset > screenSize.y )
    { // Above the clip window
        code |= TOP;
    }

    return code;
}



void WXDC_GAL::DrawLine( const VECTOR2D& aStartPoint, const VECTOR2D& aEndPoint )
{
    VECTOR2D startPoint = transformation * aStartPoint;
    VECTOR2D endPoint = transformation * aEndPoint;

    //printf("line sp %.1f %.1f ep %.1f %.1f grp %d\n", startPoint.x, startPoint.y, endPoint.x, endPoint.y, !!isGrouping );

    // Check, if the line is outside the window
    int codeS = computeOutCode( startPoint, lineWidth * worldScale );
    int codeE = computeOutCode( endPoint, lineWidth * worldScale );
    //if ( ( codeS & codeE ) == 0 )
    { // Draw the line
        dc->DrawLine( startPoint.x, startPoint.y, endPoint.x, endPoint.y );
    }

    if ( isGrouping )
    {
        GroupElement element;
        element.command = CMD_DRAW_LINE;
        element.points.push_back( aStartPoint );
        element.points.push_back( aEndPoint );
        groups.back().push_back( element );
    }
}


void WXDC_GAL::DrawCircle( const VECTOR2D& aCenterPoint, double aRadius )
{
    VECTOR2D centerPoint = transformation * aCenterPoint;
    double radius = aRadius * worldScale;

    // Drop circle if it's outside the screen
    //if( computeOutCode( centerPoint, radius ) == INSIDE )
    {
        dc->DrawCircle( centerPoint.x, centerPoint.y, radius );
    }

    if( isGrouping )
    {
        GroupElement element;
        element.command = CMD_DRAW_CIRCLE;
        element.points.push_back( aCenterPoint );
        element.scalars.push_back( aRadius );
        groups.back().push_back( element );
    }
}

// TODO wxDC draws arcs differently compared to cairo, so this should be improved here
void WXDC_GAL::DrawArc( const VECTOR2D& aCenterPoint, double aRadius,
                      double aStartAngle, double aEndAngle )
{
    // Compute the start and the end point, transform the points
    VECTOR2D centerPoint = transformation * aCenterPoint;
    double radius = aRadius * worldScale;
    VECTOR2D pointA( aRadius * cos( aStartAngle ), aRadius * sin( aStartAngle ) );
    pointA += aCenterPoint;
    VECTOR2D pointB( aRadius * cos( aEndAngle ), aRadius * sin( aEndAngle ) );
    pointB += aCenterPoint;
    pointA = transformation * pointA;
    pointB = transformation * pointB;

    // Check, if the arc is outside the window
    //if( computeOutCode( centerPoint, radius ) == INSIDE )
    {   // Draw the arc
        dc->DrawArc( pointB.x, pointB.y, pointA.x, pointA.y, centerPoint.x, centerPoint.y );
    }

    if( isGrouping )
    {
        GroupElement element;
        element.command = CMD_DRAW_ARC;
        element.points.push_back( aCenterPoint );
        element.scalars.push_back( aRadius );
        element.scalars.push_back( aStartAngle );
        element.scalars.push_back( aEndAngle );
        groups.back().push_back( element );
    }
}


void WXDC_GAL::DrawPolyline( std::deque<VECTOR2D>& aPointList )
{
    bool isFirstPoint = true;

    wxPointList pointList;

    // Iterate over the point list and draw the segments
    for( std::deque<VECTOR2D>::iterator it = aPointList.begin(); it != aPointList.end(); ++it )
    {
        VECTOR2D point = transformation * ( *it );
        wxPoint *wxpoint = new wxPoint( point.x, point.y );
        pointList.push_back( wxpoint );
    }

    dc->DrawLines( &pointList );

    for( wxPointList::iterator it = pointList.begin(); it != pointList.end(); ++it )
    {
        delete ( *it );
    }

    if( isGrouping )
    {
        GroupElement element;
        element.command = CMD_DRAW_POLYLINE;
        for( std::deque<VECTOR2D>::iterator it = aPointList.begin(); it != aPointList.end(); ++it )
        {
            element.points.push_back( *it );
        }
        groups.back().push_back( element );
    }
}


void WXDC_GAL::DrawPolygon( const std::deque<VECTOR2D>& aPointList )
{
    int numberPoints = aPointList.size();

    static std::vector<wxPoint> clippedPolygon;
    static pointVector inputPolygon, outputPolygon;

    
    inputPolygon.clear();
    outputPolygon.clear();
    clippedPolygon.clear();

    wxPoint points[numberPoints];

    // Iterate over the point list and draw the polygon
    int i = 0;
    for( std::deque<VECTOR2D>::const_iterator it = aPointList.begin(); it != aPointList.end(); ++it )
    {
        // Transform the points
        VECTOR2D point = transformation * ( *it );
        inputPolygon.push_back( PointF( (REAL) point.x, (REAL) point.y ) );
    }

    RectF window( 0.0, 0.0, screenSize.x, screenSize.y );

    SutherlandHodgman sh( window );
    sh.Clip( inputPolygon, outputPolygon );

    for( cpointIterator cit = outputPolygon.begin(); cit != outputPolygon.end(); ++cit )
    {
        clippedPolygon.push_back( wxPoint( KiROUND( cit->X ), KiROUND( cit->Y ) ) );
    }

    // Draw Polygon
    if( clippedPolygon.size() )
    {
      //  dc->DrawPolygon( clippedPolygon.size(), &clippedPolygon[0] );
    }

    if( isGrouping )
    {
        GroupElement element;
        element.command = CMD_DRAW_POLYGON;
        for( std::deque<VECTOR2D>::const_iterator it = aPointList.begin(); it != aPointList.end(); ++it )
        {
            element.points.push_back( *it );
        }
        groups.back().push_back( element );
    }
}


void WXDC_GAL::DrawRectangle( const VECTOR2D& aStartPoint, const VECTOR2D& aEndPoint )

{
    // Transform the points
    VECTOR2D diagonalPointA( aEndPoint.x, aStartPoint.y );
    VECTOR2D diagonalPointB( aStartPoint.x, aEndPoint.y );
    diagonalPointA = transformation * diagonalPointA;
    diagonalPointB = transformation * diagonalPointB;
    VECTOR2D startPoint = transformation * aStartPoint;
    VECTOR2D endPoint = transformation * aEndPoint;

    // Don't draw the rectangle if it's outside the clip area
    double lineWidth = lineWidth * worldScale;
    int codeSP = computeOutCode( startPoint, lineWidth );
    int codeEP = computeOutCode( endPoint, lineWidth );
    int codeDA = computeOutCode( diagonalPointA, lineWidth );
    int codeDB = computeOutCode( diagonalPointB, lineWidth );


    //if( ( codeSP & codeEP & codeDA & codeDB ) == 0 )
    {
        wxPoint wxpoints[5];
        wxpoints[0].x = startPoint.x;
        wxpoints[0].y = startPoint.y;
        wxpoints[1].x = diagonalPointA.x;
        wxpoints[1].y = diagonalPointA.y;
        wxpoints[2].x = endPoint.x;
        wxpoints[2].y = endPoint.y;
        wxpoints[3].x = diagonalPointB.x;
        wxpoints[3].y = diagonalPointB.y;
        wxpoints[4].x = startPoint.x;
        wxpoints[4].y = startPoint.y;

        //printf("drawRect %d %d\n", wxpoints[0].x, wxpoints[0].y);

        if( isFillEnabled )
        {
            dc->DrawPolygon( 5, wxpoints );
        }
        if( isStrokeEnabled )
        {
            dc->DrawLines( 5, wxpoints );
        }
    }

    if( isGrouping )
    {
        GroupElement element;
        element.command = CMD_DRAW_RECTANGLE;
        element.points.push_back( aStartPoint );
        element.points.push_back( aEndPoint );
        groups.back().push_back( element );
    }

}

void WXDC_GAL::DrawSegment( const VECTOR2D& aStartPoint, const VECTOR2D& aEndPoint, double aWidth )
{
  SetLineWidth ( aWidth );
  DrawLine(aStartPoint, aEndPoint);
}


/// @copydoc GAL::DrawCurve()

void WXDC_GAL::DrawCurve( const VECTOR2D& aStartPoint, const VECTOR2D& aControlPointA,
                          const VECTOR2D& aControlPointB, const VECTOR2D& aEndPoint )
{
    // FIXME Brute force method, use a better (recursive?) algorithm
    // FIXME the same method is also used in opengl_gal.cpp

    std::deque<VECTOR2D> pointList;

    double t = 0.0;
    double dt = 1.0 / (double)CURVE_POINTS;

    for( int i = 0; i <= CURVE_POINTS; i++ )
    {
        double omt = 1.0 - t;
        double omt2 = omt * omt;
        double omt3 = omt * omt2;
        double t2 = t * t;
        double t3 = t * t2;

        VECTOR2D vertex = omt3 * aStartPoint + 3.0 * t * omt2 * aControlPointA
                          + 3.0 * t2 * omt * aControlPointB + t3 * aEndPoint;

        pointList.push_back( vertex );

        t += dt;
    }

    DrawPolyline( pointList );

    if( isGrouping )
        {
            GroupElement element;
            element.command = CMD_DRAW_CURVE;
            element.points.push_back( aStartPoint );
            element.points.push_back( aControlPointA );
            element.points.push_back( aControlPointB );
            element.points.push_back( aEndPoint );
            groups.back().push_back( element );
        }
}

void WXDC_GAL::SetBackgroundColor( COLOR4D aColor )
{
    wxColour color( aColor.r * 255, aColor.g * 255, aColor.b * 255, aColor.a * 255 );

    dc->SetBackground( wxBrush( color ) );
//    backgroundColor = aColor;
}

void WXDC_GAL::SetIsFill( bool aIsFillEnabled )
{
    isFillEnabled = aIsFillEnabled;

    if( isFillEnabled )
    {
        setBrush();
    }
    else
    {
        dc->SetBrush( *wxTRANSPARENT_BRUSH );
    }

    if( isGrouping )
    {
        GroupElement groupElement;
        groupElement.command = CMD_SET_FILL;
        groupElement.boolArgument = aIsFillEnabled;
        groups.back().push_back( groupElement );
    }
}


void WXDC_GAL::SetIsStroke( bool aIsStrokeEnabled )
{
    isStrokeEnabled = aIsStrokeEnabled;

    if( isStrokeEnabled )
    {
        setPen();
    }
    else
    {
        dc->SetPen( *wxTRANSPARENT_PEN );
    }

    if( isGrouping )
    {
        GroupElement groupElement;
        groupElement.command = CMD_SET_STROKE;
        groupElement.boolArgument = aIsStrokeEnabled;
        groups.back().push_back( groupElement );
    }
}


void WXDC_GAL::SetStrokeColor( COLOR4D aColor )
{
    if( strokeColor != aColor )
    {
        strokeColor = aColor;

        setPen();

        if( isGrouping )
        {
            GroupElement groupElement;
            groupElement.command = CMD_SET_STROKECOLOR;
            groupElement.scalars.push_back( strokeColor.r );
            groupElement.scalars.push_back( strokeColor.g );
            groupElement.scalars.push_back( strokeColor.b );
            groupElement.scalars.push_back( strokeColor.a );
            groups.back().push_back( groupElement );
        }
    }
}

void WXDC_GAL::SetFillColor( COLOR4D aColor )
{
    if( fillColor != aColor )
    {
        fillColor = aColor;

        setBrush();

        if( isGrouping )
        {
            GroupElement groupElement;
            groupElement.command = CMD_SET_FILLCOLOR;
            groupElement.scalars.push_back( fillColor.r );
            groupElement.scalars.push_back( fillColor.g );
            groupElement.scalars.push_back( fillColor.b );
            groupElement.scalars.push_back( fillColor.a );
            groups.back().push_back( groupElement );
        }
    }
}


void WXDC_GAL::SetLineWidth( double aLineWidth )
{
    lineWidth = aLineWidth;

    setPen();

    if ( isGrouping )
    {
        GroupElement groupElement;
        groupElement.command = CMD_SET_LINE_WIDTH;
        groupElement.scalars.push_back( aLineWidth );
        groups.back().push_back( groupElement );
    }
}

void WXDC_GAL::ClearScreen()
{
  SetBackgroundColor( COLOR4D (0.0, 0.0, 0.0, 1.0) );

  dc->Clear();
}

void WXDC_GAL::Transform( MATRIX3x3D aTransformation )
{
    transformation =  transformation * aTransformation;
    if ( isGrouping )
    {
        GroupElement groupElement;
        groupElement.command = CMD_TRANSFORM;
        groupElement.matrixArgument = aTransformation;
        groups.back().push_back( groupElement );
    }
}

void WXDC_GAL::Rotate( double aAngle )
{
    MATRIX3x3D rotation;
    rotation.SetIdentity();
    rotation.SetRotation( aAngle );
    transformation = transformation * rotation;

    // Rotate context
    if ( isGrouping )
    {
        GroupElement groupElement;
        groupElement.command = CMD_ROTATE;
        groupElement.scalars.push_back( aAngle );
        groups.back().push_back( groupElement );
    }
}

void WXDC_GAL::Translate( const VECTOR2D& aTranslation )
{
    MATRIX3x3D translation;
    translation.SetIdentity();
    translation.SetTranslation( aTranslation );
    transformation = transformation * translation;

    //printf("translate !\n");

    if ( isGrouping )
    {
        GroupElement groupElement;
        groupElement.command = CMD_TRANSLATE;
        groupElement.points.push_back( aTranslation );
        groups.back().push_back( groupElement );
    }
}

void WXDC_GAL::Scale( const VECTOR2D &aScale )
{
    MATRIX3x3D scale;
    scale.SetIdentity();
    scale.SetScale( aScale );
    transformation = transformation * scale;

    //printf("scale !\n");
    if ( isGrouping )
    {
        GroupElement groupElement;
        groupElement.command = CMD_SCALE;
        groupElement.points.push_back( aScale );
        groups.back().push_back( groupElement );
    }
}

void WXDC_GAL::Save()
{
    transformations.push_back( transformation );

    if( isGrouping )
    {
        GroupElement groupElement;
        groupElement.command = CMD_SAVE;
        groups.back().push_back( groupElement );
    }

}

void WXDC_GAL::Restore()
{
    if( transformations.size() != 0 )
    {
        transformation = transformations.back();
        transformations.pop_back();
    }

    if( isGrouping )
    {
        GroupElement groupElement;
        groupElement.command = CMD_RESTORE;
        groups.back().push_back( groupElement );
    }
}

int WXDC_GAL::BeginGroup()
{
    std::deque<GroupElement> group;
    groups.push_back( group );

    isGrouping = true;
    return groups.size() - 1;
}

void WXDC_GAL::EndGroup()
{
    isGrouping = false;
}

void WXDC_GAL::DeleteGroup( int aGroupNumber )
{
    groups.erase(groups.begin() + aGroupNumber);
}

// This method implements a small Virtual Machine - all stored commands
// are executed; nested calling is also possible
void WXDC_GAL::DrawGroup( int aGroupNumber )
{
    for (std::deque<GroupElement>::iterator it  = groups[aGroupNumber].begin();
                                            it != groups[aGroupNumber].end(); ++it)
    {
        switch ( it->command )
        {
        case CMD_DRAW_LINE:
            DrawLine( it->points[0], it->points[1] );
            break;

        case CMD_DRAW_POLYLINE:
            DrawPolyline( it->points );
            break;

        case CMD_DRAW_CIRCLE:
            DrawCircle( it->points[0], it->scalars[0] );
            break;

        case CMD_DRAW_ARC:
            DrawArc( it->points[0], it->scalars[0], it->scalars[1], it->scalars[2] );
            break;

        case CMD_DRAW_POLYGON:
            DrawPolygon( it->points );
            break;

        case CMD_DRAW_RECTANGLE:
            DrawRectangle( it->points[0], it->points[1] );
            break;

        case CMD_SET_FILL:
            // cout << "CMD_SET_FILL" << endl;
            SetIsFill( it->boolArgument );
            break;

        case CMD_SET_STROKE:
            // cout << "CMD_SET_STROKE" << endl;
            SetIsStroke( it->boolArgument );
            break;

        case CMD_SET_FILLCOLOR:
            // cout << "CMD_SET_FILLCOLOR" << endl;
            SetFillColor(
                    COLOR4D( it->scalars[0], it->scalars[1], it->scalars[2], it->scalars[3] ) );
            break;

        case CMD_SET_STROKECOLOR:
            // // cout << "CMD_SET_STROKECOLOR" << endl;
            SetStrokeColor(
                    COLOR4D( it->scalars[0], it->scalars[1], it->scalars[2], it->scalars[3] ) );
            break;

        case CMD_SET_LINE_WIDTH:
            //// cout << "CMD_SET_LINE_WIDTH" << endl;
            SetLineWidth( it->scalars[0] );
            break;

        case CMD_SET_LINE_JOIN:
            // cout << "CMD_SET_LINE_JOIN" << endl;
            //SetLineJoin( (LineJoin) it->intArgument );
            break;

        case CMD_SET_LINE_CAP:
            // cout << "CMD_SET_LINE_CAP" << endl;
            //SetLineCap( (LineCap) it->intArgument );
            break;

        case CMD_TRANSFORM:
            Transform( it->matrixArgument );
            // cout << "CMD_TRANSFORM" << endl;
            break;

        case CMD_ROTATE:
            Rotate( it->scalars[0] );
            // cout << "CMD_ROTATE" << endl;
            break;

        case CMD_TRANSLATE:
            Translate( it->points[0] );
            // cout << "CMD_TRANSLATE" << endl;
            break;

        case CMD_SCALE:
            Scale( it->points[0] );
            // cout << "CMD_SCALE" << endl;
            break;

        case CMD_SAVE:
            Save();
            // cout << "CMD_SAVE" << endl;
            break;

        case CMD_RESTORE:
            Restore();
            // cout << "CMD_RESTORE" << endl;
            break;

        case CMD_CALL_GROUP:
            // cout << "CMD_CALL_GROUP" << endl;
            DrawGroup( it->intArgument );
            break;
        }
    }
}

void WXDC_GAL::Flush()
{
}


// ---------------
// Cursor handling
// ---------------

void WXDC_GAL::initCursor( int aCursorSize )
{
    cursorPixels = new wxBitmap( aCursorSize, aCursorSize );
    cursorPixelsSaved = new wxBitmap( aCursorSize, aCursorSize );
    cursorSize = aCursorSize;

    wxMemoryDC cursorShape( *cursorPixels );

    cursorShape.SetBackground( *wxTRANSPARENT_BRUSH );
    cursorShape.SetPen( *wxWHITE_PEN );
    cursorShape.Clear();

    cursorShape.DrawLine( 0, aCursorSize / 2, aCursorSize - 1, aCursorSize / 2 );
    cursorShape.DrawLine( aCursorSize / 2, 0, aCursorSize / 2, aCursorSize - 1 );
}


VECTOR2D WXDC_GAL::ComputeCursorToWorld( VECTOR2D aCursorPosition )
{

    MATRIX3x3D inverseMatrix = worldScreenMatrix.Inverse();
    VECTOR2D cursorPositionWorld = inverseMatrix * aCursorPosition;

    return cursorPositionWorld;
}


void WXDC_GAL::DrawCursor( VECTOR2D aCursorPosition )
{
    if( !IsShownOnScreen() )
        return;

    wxClientDC dc( this );
    wxMemoryDC cursorSave( *cursorPixelsSaved );
    wxMemoryDC cursorShape( *cursorPixels );

    // Snap to grid
    VECTOR2D cursorPositionWorld = ComputeCursorToWorld( aCursorPosition );

    cursorPositionWorld.x = round( cursorPositionWorld.x / gridSize.x ) * gridSize.x;
    cursorPositionWorld.y = round( cursorPositionWorld.y / gridSize.y ) * gridSize.y;
    aCursorPosition = worldScreenMatrix * cursorPositionWorld;

    aCursorPosition = aCursorPosition - VECTOR2D( cursorSize / 2, cursorSize / 2 );

    if( !isDeleteSavedPixels )
    {
        dc.Blit( savedCursorPosition.x, savedCursorPosition.y, cursorSize, cursorSize, &cursorSave,
                 0, 0 );
    }
    else
    {
        isDeleteSavedPixels = false;
    }

    cursorSave.Blit( 0, 0, cursorSize, cursorSize, &dc, aCursorPosition.x, aCursorPosition.y );

    dc.Blit( aCursorPosition.x, aCursorPosition.y, cursorSize, cursorSize, &cursorShape, 0, 0,
             wxOR );

    savedCursorPosition.x = (wxCoord)aCursorPosition.x;
    savedCursorPosition.y = (wxCoord)aCursorPosition.y;
}


void WXDC_GAL::DrawGridLine( VECTOR2D aStartPoint, VECTOR2D aEndPoint )
{
    DrawLine( aStartPoint, aEndPoint );
}

} // namespace KIGFX
