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

#ifndef WXDC_GAL_H_
#define WXDC_GAL_H_

// STL includes
#include <map>
#include <deque>
#include <iterator>

#include <gal/graphics_abstraction_layer.h>

// wxWidgets includes
#include <wx/dcbuffer.h>
#include <wx/dcmemory.h>
#include <wx/colour.h>
#include <wx/rawbmp.h>
#include <wx/log.h>


#define EXCEPTION_ZERO_CLIENT_RECTANGLE 0
#define EXCEPTION_ZERO_CONTEXT          1

/**
 * @brief Class WXDC_GAL is the wxDC implementation of the graphics abstraction layer.
 *
 * Quote from the wxWidgets documentation:
 * " [..] A wxDC is a device context onto which graphics and text can be drawn. [..] "
 *
 * wxDC provides a simple graphics interface, it doesn't provide as much features compared to
 * cairo or OpenGL; but it's on certain platforms like Linux fast and a fallback solution,
 * if the OpenGL hardware acceleration isn't supported well.
 *
 */

 namespace KIGFX {

class WXDC_GAL: public GAL, public wxWindow
{
public:
    /**
     * Constructor WXDC_GAL
     *
     * @param aParent is the wxWidgets immediate wxWindow parent of this object.
     *
     * @param aMouseListener is the wxEvtHandler that should receive the mouse events,
     *  this can be can be any wxWindow, but is often a wxFrame container.
     *
     * @param aPaintListener is the wxEvtHandler that should receive the paint
     *  event.  This can be any wxWindow, but is often a derived instance
     *  of this class or a containing wxFrame.  The "paint event" here is
     *  a wxCommandEvent holding EVT_GAL_REDRAW, as sent by PostPaint().
     *
     * @param aName is the name of this window for use by wxWindow::FindWindowByName()
     */
    WXDC_GAL( wxWindow *aParent, wxEvtHandler* aMouseListener = NULL,
              wxEvtHandler* aPaintListener = NULL, const wxString& aName = wxT("wxdcCanvas") );

    virtual ~WXDC_GAL();

    // ---------------
    // Drawing methods
    // ---------------

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::BeginDrawing()
    virtual void BeginDrawing() throw (int);

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::EndDrawing()
    virtual void EndDrawing();

    /// @copydoc GAL::DrawLine()
    virtual void DrawLine( const VECTOR2D& aStartPoint, const VECTOR2D& aEndPoint );

    /// @copydoc GAL::DrawSegment()
    virtual void DrawSegment( const VECTOR2D& aStartPoint, const VECTOR2D& aEndPoint, double aWidth );

    /// @copydoc GAL::DrawCircle()
    virtual void DrawCircle( const VECTOR2D& aCenterPoint, double aRadius );

    /// @copydoc GAL::DrawArc()
    virtual void DrawArc( const VECTOR2D& aCenterPoint, double aRadius,
                          double aStartAngle, double aEndAngle );

    /// @copydoc GAL::DrawRectangle()
    virtual void DrawRectangle( const VECTOR2D& aStartPoint, const VECTOR2D& aEndPoint );

    /// @copydoc GAL::DrawPolyline()
    virtual void DrawPolyline( std::deque<VECTOR2D>& aPointList );

    /// @copydoc GAL::DrawPolygon()
    virtual void DrawPolygon( const std::deque<VECTOR2D>& aPointList );

    /// @copydoc GAL::DrawCurve()
    virtual void DrawCurve( const VECTOR2D& startPoint, const VECTOR2D& controlPointA,
                            const VECTOR2D& controlPointB, const VECTOR2D& endPoint );

    // --------------
    // Screen methods
    // --------------

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::ClearScreen()
    virtual void ClearScreen();

    // -----------------
    // Attribute setting
    // -----------------

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::SetIsFill()
    virtual void SetIsFill( bool aIsFillEnabled );

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::SetIsStroke()
    virtual void SetIsStroke( bool aIsStrokeEnabled );

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::SetFillColor()
    virtual void SetFillColor( COLOR4D aColor );

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::SetStrokeColor()
    virtual void SetStrokeColor( COLOR4D aColor );

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::GetStrokeColor()
    COLOR4D      GetStrokeColor();

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::SetBackgroundColor()
    virtual void SetBackgroundColor( COLOR4D aColor );


    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::SetLineWidth()
    virtual void SetLineWidth( double aLineWidth );

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::GetLineWidth()
    double       GetLineWidth();

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::SetLayerDepth()
    virtual void SetLayerDepth( double aLayerDepth ){
        //GAL::SetLayerDepth( aLayerDepth );
    }

    // --------------
    // Transformation
    // --------------

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::Transform()
    virtual void Transform( MATRIX3x3D aTransformation );

    virtual void Rotate( double aAngle );

    /**
     * @brief Translate the context.
     *
     * @param aTranslation is the translation vector.
     */
    virtual void Translate( const VECTOR2D& aTranslation );

    /**
     * @brief Scale the context.
     *
     * @param aScale is the scale factor for the x- and y-axis.
     */
    virtual void Scale( const VECTOR2D& aScale );

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::Save()
    virtual void Save();

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::Restore()
    virtual void Restore();

    // --------------------------------------------
    // Group methods
    // ---------------------------------------------

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::BeginGroup()
    virtual int BeginGroup();

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::EndGroup()
    virtual void EndGroup();

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::DrawGroup()
    virtual void DrawGroup( int aGroupNumber );

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::DeleteGroup()
    virtual void DeleteGroup( int aGroupNumber );

    // --------------------------------------------------------
    // Handling the world <-> screen transformation
    // --------------------------------------------------------
    virtual void ComputeWorldScreenMatrix();



    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::SetWorldUnitLength()
    void SetWorldUnitLength( double aWorldUnitLength );

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::SetScreenDPI()
    void SetScreenDPI( double aScreenDPI );

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::SetLookAtPoint()
    void SetLookAtPoint( VECTOR2D aPoint );

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::GetLookAtPoint()
    VECTOR2D GetLookAtPoint();

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::SetZoomFactor()
    void SetZoomFactor( double aZoomFactor );

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::GetZoomFactor()
    double GetZoomFactor();

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::SaveScreen()
    virtual void SaveScreen();

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::RestoreScreen()
    virtual void RestoreScreen();
    /// @brief Resizes the canvas.
    virtual void ResizeScreen( int aWidth, int aHeight );

    /// @brief Shows/hides the GAL canvas
    virtual bool Show( bool aShow );

    /// @copydoc GAL::Flush()
    virtual void Flush();

    // -------
    // Cursor
    // -------

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::ComputeCursorToWorld()
    virtual VECTOR2D ComputeCursorToWorld( VECTOR2D aCursorPosition );

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::SetIsCursorEnabled()
    void SetIsCursorEnabled( bool aIsCursorEnabled );

    /// @copydoc GRAPHICS_ABSTRACTION_LAYER::DrawCursor()
    virtual void DrawCursor( VECTOR2D aCursorPosition );

    /**
     * Function PostPaint
     * posts an event to m_paint_listener.  A post is used so that the actual drawing
     * function can use a device context type that is not specific to the wxEVT_PAINT event.
     */
    void PostPaint()
    {
        if( paintListener )
        {
            wxPaintEvent redrawEvent;
            wxPostEvent( paintListener, redrawEvent );
        }
    }

    void SetMouseListener( wxEvtHandler* aMouseListener )
    {
        mouseListener = aMouseListener;
    }

    void SetPaintListener( wxEvtHandler* aPaintListener )
    {
        paintListener = aPaintListener;
    }

protected:
    virtual void DrawGridLine( VECTOR2D aStartPoint, VECTOR2D aEndPoint );

private:
    /// Maximum number of arguments for one command
    static const int MAX_WXDC_ARGUMENTS = 6;

    /// The number of points for curve approximation
    static const int CURVE_POINTS  = 32;

    /// Definitions for the command recorder
    enum GraphicsCommand
    {
        CMD_DRAW_LINE,          ///< Draw a line
        CMD_DRAW_POLYLINE,      ///< Draw a polyline
        CMD_DRAW_CIRCLE,        ///< Draw a circle
        CMD_DRAW_ARC,           ///< Draw an arc
        CMD_DRAW_POLYGON,       ///< Draw a polygon
        CMD_DRAW_RECTANGLE,     ///< Draw a rectangle
        CMD_DRAW_CURVE,         ///< Draw a curve
        CMD_SET_FILL,           ///< Enable/disable filling
        CMD_SET_STROKE,         ///< Enable/disable stroking
        CMD_SET_FILLCOLOR,      ///< Set the fill color
        CMD_SET_STROKECOLOR,    ///< Set the stroke color
        CMD_SET_LINE_WIDTH,     ///< Set the line width
        CMD_SET_LINE_CAP,       ///< Set the line cap style
        CMD_SET_LINE_JOIN,      ///< Set the line join style
        CMD_TRANSFORM,          ///< Transform the actual context
        CMD_ROTATE,             ///< Rotate the context
        CMD_TRANSLATE,          ///< Translate the context
        CMD_SCALE,              ///< Scale the context
        CMD_SAVE,               ///< Save the transformation matrix
        CMD_RESTORE,            ///< Restore the transformation matrix
        CMD_CALL_GROUP          ///< Call a group
    };

    /// Type definition for an graphics group element
    typedef struct
    {
        GraphicsCommand command;                ///< Command to execute
        std::deque<VECTOR2D> points;             ///< Point arguments
        std::deque<double> scalars;             ///< Arguments for commands
        bool boolArgument;                      ///< A bool argument
        int intArgument;                        ///< An int argument
        MATRIX3x3D matrixArgument;              ///< A matrix argument
    } GroupElement;

    // Variables related to wxWidgets
    wxWindow* parentWindows;                    ///< Parent window
    wxEvtHandler* mouseListener;                ///< Mouse listener
    wxEvtHandler* paintListener;                ///< Paint listener
    wxRect clientRectangle;                     ///< Area definition of the surface

    // Cursor variables
    std::deque<wxColour> savedCursorPixels;     ///< Saved pixels of the cursor
    bool isDeleteSavedPixels;                   ///< True, if the saved pixels can be discarded
    wxPoint savedCursorPosition;                ///< The last cursor position
    wxBitmap* cursorPixels;                     ///< Cursor pixels
    wxBitmap* cursorPixelsSaved;                ///< Saved cursor pixels
    int cursorSize;                             ///< Cursor size

    // Variables for the grouping function
    int actualGroupIndex;                       ///< The index of the actual group
    bool isGrouping;                            ///< Is grouping enabled ?
    bool isElementAdded;                        ///< Was an graphic element added ?
    std::deque<std::deque<GroupElement> > groups; ///< List of graphic groups

    // Variables related to wxWidgets
    unsigned char* bitmapBuffer;                ///< Storage of the cairo image
    wxBitmap* wxbitmap;                         ///< Pointer to the wxWidgets bitmap
    wxBitmap* wxbitmapBackup;                   ///< Pointer to the saved wxWidgets bitmap
    wxClientDC* clientDC;                       ///< Pointer to the clientDC
    wxDC* dc;                               ///< Pointer to a buffered DC

    // Mapping between wxDC and GAL line attributes
    MATRIX3x3D transformation;     ///< Actual transformation matrix
    std::deque<MATRIX3x3D> transformations;     ///< Transformation stack

    /// Allocate bitmaps for drawing
    void allocateBitmaps()
    {
        wxbitmap = new wxBitmap( screenSize.x, screenSize.y, wxBITMAP_SCREEN_DEPTH );
        wxbitmapBackup = new wxBitmap( screenSize.x, screenSize.y, wxBITMAP_SCREEN_DEPTH );
    }

    // Event handlers
    /**
     * @brief Paint event handler.
     *
     * @param aEvent is the paint event.
     */
    void onPaint( wxPaintEvent& aEvent );

    /**
     * @brief Window resizing event handler.
     *
     * @param aEvent is the window resizing event.
     */
    void onSize( wxSizeEvent& aEvent );

    /**
     * @brief Mouse event handler, forwards the event to the child.
     *
     * @param aEvent is the mouse event.
     */
    void skipMouseEvent( wxMouseEvent& aEvent );

    /**
     * @brief Initialize the cursor.
     *
     * @param aCursorSize is the size of the cursor.
     */
    void initCursor( int aCursorSize );

    /// Update the wxDC pen
    inline void setPen( void )
    {
        if( isStrokeEnabled )
        {
            wxColour color( strokeColor.r * strokeColor.a * 255,
                            strokeColor.g * strokeColor.a * 255,
                            strokeColor.b * strokeColor.a * 255, 255 );
            wxPen pen = wxPen( color );
//            pen.SetCap( lineCapMap[lineCap] );
  //          pen.SetJoin( lineJoinMap[lineJoin] );
            pen.SetWidth( floor( lineWidth * worldScale + 0.5 ) );

            dc->SetPen( pen );
        }
    }

    /// Update the wxDC brush
    inline void setBrush( void )
    {
        if( isFillEnabled )
        {
            wxColour color( fillColor.r * fillColor.a * 255, fillColor.g * fillColor.a * 255,
                            fillColor.b * fillColor.a * 255, 255 );

            dc->SetBrush( wxBrush( color ) );
        }
    }

    // Some ideas and structures are taken from the Cohen-Sutherland algorithm on Wikipedia
    // http://en.wikipedia.org/wiki/Cohen-Sutherland_algorithm

    // "Outcode" definition
    static const int INSIDE = 0;    /// Inside the clip window
    static const int LEFT = 1;      /// Left of the clip window
    static const int RIGHT = 2;     /// Right of the clip window
    static const int BOTTOM = 4;    /// Below the clip window
    static const int TOP = 8;       /// Above the clip window

    /**
     * @brief Compute the "outcode".
     *
     * The outcode determines if the given point is inside (=0) or outside (!=0) the screen area.
     *
     * @param point is the point.
     * @param offset is an offset value, for including e.g. the line width.
     * @return is the computed outcode.
     */
    int computeOutCode( VECTOR2D aPoint, double offset );

};
};

#endif /* WXDC_GAL_H_ */
