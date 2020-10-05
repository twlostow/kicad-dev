/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015-2016 Mario Luzeiro <mrluzeiro@ua.pt>
 * Copyright (C) 1992-2020 KiCad Developers, see AUTHORS.txt for contributors.
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

#ifndef EDA_3D_CANVAS_H
#define EDA_3D_CANVAS_H


#include "board_adapter.h"
#include "3d_rendering/3d_render_raytracing/accelerators/caccelerator.h"
#include "3d_rendering/c3d_render_base.h"
#include "3d_cache/3d_cache.h"
#include <gal/hidpi_gl_canvas.h>
#include <wx/image.h>
#include <wx/timer.h>


class WX_INFOBAR;
class wxStatusBar;
class BOARD;
class C3D_RENDER_RAYTRACING;
class C3D_RENDER_OGL_LEGACY;


/**
 *  Implement a canvas based on a wxGLCanvas
 */
class EDA_3D_CANVAS : public HIDPI_GL_CANVAS
{
 public:
    /**
     *  @brief EDA_3D_CANVAS - Creates a new 3D Canvas with a attribute list
     *  @param aParent: the parent creator of this canvas
     *  @param aAttribList: a list of openGL options created by GetOpenGL_AttributesList
     *  @param aBoard: The board
     *  @param aSettings: the settings options to be used by this canvas
     */
    EDA_3D_CANVAS( wxWindow* aParent, const int* aAttribList, BOARD* aBoard,
                   BOARD_ADAPTER& aSettings, CCAMERA& aCamera, S3D_CACHE* a3DCachePointer );

    ~EDA_3D_CANVAS();

    /**
     * Function SetEventDispatcher()
     * Sets a dispatcher that processes events and forwards them to tools.
     * @param aEventDispatcher is the object that will be used for dispatching events.
     * DRAW_PANEL_GAL does not take over the ownership. Passing NULL disconnects all event
     * handlers from the DRAW_PANEL_GAL and parent frame.
     */
    void SetEventDispatcher( TOOL_DISPATCHER* aEventDispatcher );

    void SetStatusBar( wxStatusBar* aStatusBar )
    {
        m_parentStatusBar = aStatusBar;
    }

    void SetInfoBar( WX_INFOBAR* aInfoBar )
    {
        m_parentInfoBar = aInfoBar;
    }

    void ReloadRequest( BOARD *aBoard = NULL, S3D_CACHE *aCachePointer = NULL );

    /**
     * @brief IsReloadRequestPending - Query if there is a pending reload request
     * @return true if it wants to reload, false if there is no reload pending
     */
    bool IsReloadRequestPending() const
    {
        if( m_3d_render )
            return m_3d_render->IsReloadRequestPending();
        else
            return false;
    }

    /**
     * @brief RenderRaytracingRequest - Request to render the current view in Raytracing mode
     */
    void RenderRaytracingRequest();

    /**
     *  Request a screenshot and output it to the aDstImage
     *  @param aDstImage - Screenshot destination image
     */
    void GetScreenshot( wxImage &aDstImage );

    /**
     * @brief SetView3D - Helper function to call view commands
     * @param aKeycode: ascii key commands
     * @return true if the key code was handled,
     *  false if no command found for this code.
     */
    bool SetView3D( int aKeycode );

    /**
     * @brief AnimationEnabledSet - Enable or disable camera animation when switching to a pre-defined view
     * @param aAnimationEnabled: Animation enabled state to set
     */
    void AnimationEnabledSet( bool aAnimationEnabled ) { m_animation_enabled = aAnimationEnabled; }

    /**
     * @brief AnimationEnabledGet - Returns whether camera animation is enabled when switching to a pre-defined view
     * @return true if animation is enabled
     */
    bool AnimationEnabledGet() const { return m_animation_enabled; }

    /**
     * @brief MovingSpeedMultiplierSet - Set the camera animation moving speed multiplier option
     * @param aMovingSpeedMultiplier: One of the possible integer options: [1,2,3,4,5]
     */
    void MovingSpeedMultiplierSet( int aMovingSpeedMultiplier ) { m_moving_speed_multiplier = aMovingSpeedMultiplier; }

    /**
     * @brief MovingSpeedMultiplierGet - Return the current camera animation moving speed multiplier option
     * @return current moving speed multiplier option, one of [1,2,3,4,5]
     */
    int MovingSpeedMultiplierGet() const { return m_moving_speed_multiplier; }

    /**
     * @brief RenderEngineChanged - Notify that the render engine was changed
     */
    void RenderEngineChanged();

    /**
     * @brief DisplayStatus - Update the status bar with the position information
     */
    void DisplayStatus();

    /**
     * @brief Request_refresh - Schedule a refresh update of the canvas
     * @param aRedrawImmediately - true will request a redraw, false will
     * schedule a redraw, after a short timeout.
     */
    void Request_refresh( bool aRedrawImmediately = true );

    /**
     * Used to forward events to the canvas from popups, etc.
     */
    void OnEvent( wxEvent& aEvent );

private:
    /** Called by a wxPaintEvent event
     */
    void OnPaint( wxPaintEvent& aEvent );

    /**
     * The actual function to repaint the canvas.
     * It is usually called by OnPaint() but because it does not use a wxPaintDC
     * it can be called outside a wxPaintEvent
     */
    void DoRePaint();

    void OnEraseBackground( wxEraseEvent &event );

    void OnRefreshRequest( wxEvent& aEvent );

    void OnMouseWheel( wxMouseEvent &event );

#if wxCHECK_VERSION( 3, 1, 0 ) || defined( USE_OSX_MAGNIFY_EVENT )
    void   OnMagnify( wxMouseEvent& event );
#endif

    void OnMouseMove( wxMouseEvent &event );
    void OnLeftDown( wxMouseEvent &event );
    void OnLeftUp( wxMouseEvent &event );
    void OnMiddleUp( wxMouseEvent &event );
    void OnMiddleDown( wxMouseEvent &event );
    void OnTimerTimeout_Editing( wxTimerEvent& event );
    void OnCloseWindow( wxCloseEvent &event );
    void OnResize( wxSizeEvent &event );
    void OnTimerTimeout_Redraw( wxTimerEvent& event );

    DECLARE_EVENT_TABLE()

 private:
    /**
     * @brief stop_editingTimeOut_Timer - stop the editing time, so it will not timeout
     */
    void stop_editingTimeOut_Timer();

    /**
     * @brief restart_editingTimeOut_Timer - reset the editing timer
     */
    void restart_editingTimeOut_Timer();

    /**
     * @brief request_start_moving_camera - start a camera movement
     * @param aMovingSpeed: the time speed
     * @param aRenderPivot: if it should display pivot cursor while move
     */
    void request_start_moving_camera( float aMovingSpeed = 2.0f, bool aRenderPivot = true );

    /**
     * @brief move_pivot_based_on_cur_mouse_position -
     * This function hits a ray to the board and start a moviment
     */
    void move_pivot_based_on_cur_mouse_position();

    /**
     * @brief render_pivot - render the pivot cursor
     * @param t: time between 0.0 and 1.0
     * @param aScale: scale to apply on the cursor
     */
    void render_pivot( float t, float aScale );

    /**
     * @brief initializeOpenGL
     * @return if OpenGL initialization succeed
     */
    bool initializeOpenGL();

    /**
     * @brief releaseOpenGL - free created targets and openGL context
     */
    void releaseOpenGL();

    RAY getRayAtCurrrentMousePosition();

 private:

    TOOL_DISPATCHER*       m_eventDispatcher;
    wxStatusBar*           m_parentStatusBar;         // Parent statusbar to report progress
    WX_INFOBAR*            m_parentInfoBar;

    wxGLContext*           m_glRC;                    // Current OpenGL context
    bool                   m_is_opengl_initialized;
    bool                   m_is_opengl_version_supported;

    wxTimer                m_editing_timeout_timer;   // Expires after some time signalling that
                                                      // the mouse / keyboard movements are over
    wxTimer                m_redraw_trigger_timer;    // Used to schedule a redraw event

    bool                   m_mouse_is_moving;         // Mouse activity is in progress
    bool                   m_mouse_was_moved;
    bool                   m_camera_is_moving;        // Camera animation is ongoing
    bool                   m_render_pivot;            // Render the pivot while camera moving
    float                  m_camera_moving_speed;     // 1.0f will be 1:1
    unsigned               m_strtime_camera_movement; // Ticktime of camera movement start
    bool                   m_animation_enabled;       // Camera animation enabled
    int                    m_moving_speed_multiplier; // Camera animation speed multiplier option

    BOARD_ADAPTER&         m_boardAdapter;            // Pre-computed 3D info and settings
    CCAMERA&               m_camera;
    C3D_RENDER_BASE*       m_3d_render;
    C3D_RENDER_RAYTRACING* m_3d_render_raytracing;
    C3D_RENDER_OGL_LEGACY* m_3d_render_ogl_legacy;

    static const float     m_delta_move_step_factor;  // Step factor to used with cursor on
                                                      // relation to the current zoom

    bool                   m_opengl_supports_raytracing;
    bool                   m_render_raytracing_was_requested;

    CCONTAINER             m_3DShapes_container;      // Holds 3D shapes from modules
    CGENERICACCELERATOR    *m_accelerator3DShapes;    // used for mouse over searching

    BOARD_ITEM*            m_currentIntersectedBoardItem;

    /**
     *  Trace mask used to enable or disable the trace output of this class.
     *  The debug output can be turned on by setting the WXTRACE environment variable to
     *  "KI_TRACE_EDA_3D_CANVAS".  See the wxWidgets documentation on wxLogTrace for
     *  more information.
     */
    static const wxChar *m_logTrace;
};


#endif // EDA_3D_CANVAS_H
