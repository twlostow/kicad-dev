/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2014 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2008-2017 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 2004-2017 KiCad Developers, see change_log.txt for contributors.
 * Copyright (C) 2016-2017 CERN
 * @author Michele Castellana <michele.castellana@cern.ch>
 * @author Maciej Suminski <maciej.suminski@cern.ch>
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

/**
 * @file libeditframe.h
 * @brief Definition of class LIB_EDIT_FRAME
 */

#ifndef LIBEDITFRM_H_
#define LIBEDITFRM_H_

#include <sch_base_frame.h>

#include <lib_draw_item.h>
#include <lib_collectors.h>
#include <memory>

class SCH_EDIT_FRAME;
class SCH_SCREEN;
class PART_LIB;
class LIB_PART;
class LIB_ALIAS;
class LIB_FIELD;
class LIB_MANAGER;
class DIALOG_LIB_EDIT_TEXT;
class EESCHEMA_TREE;

/**
 * The component library editor main window.
 */
class LIB_EDIT_FRAME : public SCH_BASE_FRAME
{
    LIB_PART*       m_curPart;              ///< currently modified part
    PART_LIB*       m_curLib;               ///< currently modified library
    LIB_PART*       m_tempCopyComponent;    ///< temp copy of a part during edit, I own it here.
    LIB_COLLECTOR   m_collectedItems;       ///< Used for hit testing.
    wxComboBox*     m_partSelectBox;        ///< a Box to select a part to edit (if any)
    wxComboBox*     m_aliasSelectBox;       ///< a box to select the alias to edit (if any)
    EESCHEMA_TREE*  m_panelTree;            ///< search tree widget

    /** Convert of the item currently being drawn. */
    bool m_drawSpecificConvert;

    /**
     * Specify which component parts the current draw item applies to.
     *
     * If true, the item being drawn or edited applies only to the selected
     * part.  Otherwise it applies to all parts in the component.
     */
    bool m_drawSpecificUnit;

    /**
     * Set to true to not synchronize pins at the same position when editing
     * components with multiple parts or multiple body styles.  Setting this
     * to false allows editing each pin per part or body style individually.
     * This requires the user to open each part or body style to make changes
     * to the pin at the same location.
     */
    bool m_editPinsPerPartOrConvert;

    /**
     * the option to show the pin electrical name in the component editor
     */
    bool m_showPinElectricalTypeName;

    /** The current draw or edit graphic item fill style. */
    static FILL_T m_drawFillStyle;

    /** Default line width for drawing or editing graphic items. */
    static int m_drawLineWidth;

    static LIB_ITEM*    m_lastDrawItem;
    static LIB_ITEM*    m_drawItem;
    static wxString     m_aliasName;

    // The unit number to edit and show
    static int m_unit;

    // Show the normal shape ( m_convert <= 1 ) or the converted shape
    // ( m_convert > 1 )
    static int m_convert;

    // true to force DeMorgan/normal tools selection enabled.
    // They are enabled when the loaded component has
    // Graphic items for converted shape
    // But under some circumstances (New component created)
    // these tools must left enabled
    static bool m_showDeMorgan;

    /// The current text size setting.
    static int m_textSize;

    /// Current text angle setting.
    static double m_current_text_angle;

    /// The default pin num text size setting.
    static int m_textPinNumDefaultSize;

    /// The default  pin name text size setting.
    static int m_textPinNameDefaultSize;

    ///  Default pin length
    static int m_defaultPinLength;

    /// Default repeat offset for pins in repeat place pin
    int m_repeatPinStep;

    static wxSize m_clientSize;

    friend class DIALOG_LIB_EDIT_TEXT;

    LIB_ITEM* locateItem( const wxPoint& aPosition, const KICAD_T aFilterList[] );

public:

    LIB_EDIT_FRAME( KIWAY* aKiway, wxWindow* aParent );

    ~LIB_EDIT_FRAME();

    /** The current library being edited, or NULL if none. */
    PART_LIB* GetCurLib() const;

    /**
     * Returns the name of currently edited library.
     */
    wxString GetCurLibName() const;

    /** Sets the current library and return the old. */
    void SetCurLib( PART_LIB* aLib );

    void SetCurLib( const wxString& aLibName );

    /**
     * Function GetCurPart
     * returns the current part being edited, or NULL if none selected.
     * This is a LIB_PART that I own, it is at best a copy of one in a library.
     */
    LIB_PART* GetCurPart() const
    {
        return m_curPart;
    }

    /**
     * Returns the name of currrently modified part.
     */
    wxString GetCurPartName() const;

    /**
     * Function SetCurPart
     * takes ownership over aPart and notes that it is the one currently
     * being edited.
     */
    void SetCurPart( LIB_PART* aPart );

    /**
     * Returns the library for which the right click context menu is opened
     * or the currently modified library.
     */
    PART_LIB* GetSelectedLib() const;

    /**
     * Returns the part for which the right click context menu is opened
     * or the currently modified library.
     */
    LIB_PART* GetSelectedPart() const;

    /** @return the default pin num text size.
     */
    static int GetPinNumDefaultSize() { return m_textPinNumDefaultSize; }

    /** @return The default  pin name text size setting.
     */
    static int GetPinNameDefaultSize() { return m_textPinNameDefaultSize; }

    /** @return The default pin len setting.
     */
    static int GetDefaultPinLength() { return m_defaultPinLength; }

    /** Set the default pin len.
     */
    static void SetDefaultPinLength( int aLength ) { m_defaultPinLength = aLength; }

    /**
     * @return the increment value of the position of a pin
     * for the pin repeat command
     */
    int GetRepeatPinStep() const { return m_repeatPinStep; }

    /**
     * Sets the repeat step value for pins repeat command
     * @param aStep the increment value of the position of an item
     * for the repeat command
     */
    void SetRepeatPinStep( int aStep) { m_repeatPinStep = aStep; }


    void ReCreateMenuBar() override;

    /**
     * Function EnsureActiveLibExists
     * must be called after the libraries are reloaded
     * (for instance after loading a schematic project)
     */
    static void EnsureActiveLibExists();

    void InstallConfigFrame( wxCommandEvent& event );
    void OnPreferencesOptions( wxCommandEvent& event );
    void Process_Config( wxCommandEvent& event );

    /**
     * Function SycnronizePins
     * @return True if the edit pins per part or convert is false and the current
     *         component has multiple parts or body styles.  Otherwise false is
     *         returned.
     */
    bool SynchronizePins();

    /**
     * Function OnPlotCurrentComponent
     * plot the current component in SVG or PNG format.
     */
    void OnPlotCurrentComponent( wxCommandEvent& aEvent );
    void Process_Special_Functions( wxCommandEvent& aEvent );
    void OnSelectTool( wxCommandEvent& aEvent );
    void OnSelectAlias( wxCommandEvent& aEvent );
    void OnSelectPart( wxCommandEvent& aEvent );

    /**
     * From Option toolbar: option to show the electrical pin type name
     */
    void OnShowElectricalType( wxCommandEvent& event );

    /**
     * Creates a new library. The library is added to the project libraries table.
     */
    void CreateNewLibrary( wxCommandEvent& aEvent );

    /**
     * Save changes to the selected library.
     */
    void SaveLibrary( wxCommandEvent& aEvent );

    /**
     * Save the selected library, including unsaved changes to a new file.
     */
    void SaveLibraryAs( wxCommandEvent& aEvent );

    /**
     * Adds an existing library. The library is added to the project libraries table.
     */
    void AddLibrary( wxCommandEvent& aEvent );

    /**
     * Removes a library. The library is removed from the project libraries table.
     */
    void RemoveLibrary( wxCommandEvent& aEvent );

    /**
     * Saves a single part in the selected library. The library file is updated without including
     * the remaining unsaved changes.
     */
    void SavePart( wxCommandEvent& aEvent );

    /**
     * Creates a new part in the selected library.
     */
    void CreateNewPart( wxCommandEvent& aEvent );

    /**
     * Routine to read one part.
     * The format is that of libraries, but it loads only 1 component.
     * Or 1 component if there are several.
     * If the first component is an alias, it will load the corresponding root.
     */
    void ImportPart( wxCommandEvent& aEvent );

    /**
     * Function OnExportPart
     * creates a new library and backup the current component in this library or export
     * the component of the current library.
     */
    void ExportPart( wxCommandEvent& aEvent );

    /**
     * Opens the selected part for editing.
     */
    void EditSelectedPart( wxCommandEvent& aEvent );

    /**
     * Copies/cuts (depending on the aEvent ID) the selected part to the Library Editor clipboard.
     * It can be pasted to another library.
     */
    void CopyCutSelectedPart( wxCommandEvent& aEvent );

    /**
     * Pastes the Library Editor clipboard to the selected library. The clipboard contents is not
     * emptied.
     */
    void PasteParts( wxCommandEvent& aEvent );

    void RenameSelectedPart( wxCommandEvent& aEvent );
    void RemoveSelectedPart( wxCommandEvent& aEvent );
    void RevertSelectedPart( wxCommandEvent& aEvent );

    void OnEditComponentProperties( wxCommandEvent& aEvent );
    void InstallFieldsEditorDialog( wxCommandEvent& aEvent );

    /**
     * Function LoadOneLibraryPart
     * loads a library component from the currently selected library.
     *
     * If a library is already selected, the user is prompted for the component name
     * to load.
     */
    void LoadOneLibraryPart( wxTreeEvent& aEvent );

    void OnViewEntryDoc( wxCommandEvent& aEvent );
    void OnCheckComponent( wxCommandEvent& aEvent );
    void OnSelectBodyStyle( wxCommandEvent& aEvent );
    void OnEditPin( wxCommandEvent& aEvent );
    void OnSelectItem( wxCommandEvent& aEvent );

    void OnOpenPinTable( wxCommandEvent& aEvent );

    void OnUpdateSelectTool( wxUpdateUIEvent& aEvent );
    void OnUpdateSelectedLib( wxUpdateUIEvent& aEvent );
    void OnUpdateEditingPart( wxUpdateUIEvent& aEvent );
    void OnUpdatePartModified( wxUpdateUIEvent& aEvent );
    void OnUpdateLibModified( wxUpdateUIEvent& aEvent );
    void OnUpdateClipboardNotEmpty( wxUpdateUIEvent& aEvent );
    void OnUpdateUndo( wxUpdateUIEvent& aEvent );
    void OnUpdateRedo( wxUpdateUIEvent& aEvent );
    void OnUpdateViewDoc( wxUpdateUIEvent& aEvent );
    void OnUpdatePinByPin( wxUpdateUIEvent& aEvent );
    void OnUpdatePinTable( wxUpdateUIEvent& aEvent );
    void OnUpdatePartNumber( wxUpdateUIEvent& aEvent );
    void OnUpdateDeMorganNormal( wxUpdateUIEvent& aEvent );
    void OnUpdateDeMorganConvert( wxUpdateUIEvent& aEvent );
    void OnUpdateSelectAlias( wxUpdateUIEvent& aEvent );
    void OnUpdateElectricalType( wxUpdateUIEvent& aEvent );
    void OnUpdateSearchTree( wxUpdateUIEvent& aEvent );

    void UpdateAliasSelectList();
    void UpdatePartSelectList();

    /**
     * Function DisplayLibInfos
     * updates the main window title bar with the current library name and read only status
     * of the library.
     */
    void DisplayLibInfos();

    /**
     * Function RedrawComponent
     * Redraw the current component loaded in library editor
     * Display reference like in schematic (a reference U is shown U? or U?A)
     * accordint to the current selected unit and De Morgan selection
     * although it is stored without ? and part id.
     * @param aDC = the current device context
     * @param aOffset = a draw offset. usually 0,0 to draw on the screen, but
     * can be set to page size / 2 to draw or print in SVG format.
     */
    void RedrawComponent( wxDC* aDC, wxPoint aOffset );

    /**
     * Function RedrawActiveWindow
     * Redraw the current component loaded in library editor, an axes
     * Display reference like in schematic (a reference U is shown U? or U?A)
     * update status bar and info shown in the bottom of the window
     */
    void RedrawActiveWindow( wxDC* DC, bool EraseBg ) override;

    void OnCloseWindow( wxCloseEvent& Event );
    void ReCreateHToolbar() override;
    void ReCreateVToolbar() override;
    void CreateOptionToolbar();
    void OnLeftClick( wxDC* DC, const wxPoint& MousePos ) override;
    bool OnRightClick( const wxPoint& MousePos, wxMenu* PopMenu ) override;
    double BestZoom() override;         // Returns the best zoom
    void OnLeftDClick( wxDC* DC, const wxPoint& MousePos ) override;

    ///> @copydoc EDA_DRAW_FRAME::GetHotKeyDescription()
    EDA_HOTKEY* GetHotKeyDescription( int aCommand ) const override;

    bool OnHotKey( wxDC* aDC, int aHotKey, const wxPoint& aPosition, EDA_ITEM* aItem = NULL ) override;

    bool GeneralControl( wxDC* aDC, const wxPoint& aPosition, EDA_KEY aHotKey = 0 ) override;

    void LoadSettings( wxConfigBase* aCfg ) override;

    void SaveSettings( wxConfigBase* aCfg ) override;

    /**
     * Function CloseWindow
     * triggers the wxCloseEvent, which is handled by the function given
     * to EVT_CLOSE() macro:
     * <p>
     * EVT_CLOSE( LIB_EDIT_FRAME::OnCloseWindow )
     */
    void CloseWindow( wxCommandEvent& event )
    {
        // Generate a wxCloseEvent
        Close( false );
    }

    /**
     * Function OnModify
     * Must be called after a schematic change
     * in order to set the "modify" flag of the current screen
     */
    void OnModify();

    const wxString& GetAliasName()      { return m_aliasName; }

    int GetUnit() { return m_unit; }

    void SetUnit( int unit )
    {
        wxASSERT( unit >= 1 );
        m_unit = unit;
    }

    int GetConvert() { return m_convert; }

    void SetConvert( int convert )
    {
        wxASSERT( convert >= 0 );
        m_convert = convert;
    }

    LIB_ITEM* GetLastDrawItem() { return m_lastDrawItem; }

    void SetLastDrawItem( LIB_ITEM* drawItem )
    {
        m_lastDrawItem = drawItem;
    }

    LIB_ITEM* GetDrawItem() { return m_drawItem; }

    void SetDrawItem( LIB_ITEM* drawItem ) { m_drawItem = drawItem; }

    bool GetShowDeMorgan() { return m_showDeMorgan; }

    void SetShowDeMorgan( bool show ) { m_showDeMorgan = show; }

    bool GetShowElectricalType() { return m_showPinElectricalTypeName; }

    void SetShowElectricalType( bool aShow ) { m_showPinElectricalTypeName = aShow; }

    FILL_T GetFillStyle() { return m_drawFillStyle; }

    /**
     * Function TempCopyComponent
     * create a temporary copy of the current edited component
     * Used to prepare an Undo ant/or abort command before editing the component
     */
    void TempCopyComponent();

    /**
     * Function RestoreComponent
     * Restore the current edited component from its temporary copy.
     * Used to abort a command
     */
    void RestoreComponent();

    /**
     * Function GetTempCopyComponent
     * @return the temporary copy of the current component.
     */
    LIB_PART*      GetTempCopyComponent() { return m_tempCopyComponent; }

    /**
     * Function ClearTempCopyComponent
     * delete temporary copy of the current component and clear pointer
     */
    void ClearTempCopyComponent();

    bool IsEditingDrawItem() { return m_drawItem && m_drawItem->InEditMode(); }

    bool IsClipboardEmpty() const
    {
        return m_clipboard.empty();
    }

    /**
     * Function SaveCopyInUndoList.
     * Create a copy of the current component, and save it in the undo list.
     * Because a component in library editor does not a lot of primitives,
     * the full data is duplicated. It is not worth to try to optimize this save funtion
     */
    void SaveCopyInUndoList( EDA_ITEM* aItemToCopy );

    /**
     * Function SaveLibrary
     * saves the changes to the current library.
     *
     * A backup file of the current library is saved with the .bak extension before the
     * changes made to the library are saved.
     * @param aLibrary The library to save.
     * @param aFileName Optional destination file name. If empty, then the original library file
     * is used.
     * @return True if the library was successfully saved.
     */
    bool SaveLibrary( PART_LIB* aLibrary, const wxString& aFileName = wxEmptyString );

private:
    /**
     * Populates the symbol search tree.
     */
    void PopulateTree();

    /**
     * Function OnActivate
     * is called when the frame is activated. Tests if the current library exists.
     * The library list can be changed by the schematic editor after reloading a new schematic
     * and the current library can point a non existent lib.
     */
    virtual void OnActivate( wxActivateEvent& event ) override;

    // General:

    /**
     * Function SaveOnePart
     * saves a LIB_PART into the current PART_LIB.
     *
     * Any changes are updated in memory only and NOT to a file.  The old component is
     * deleted from the library and/or any aliases before the edited component is updated
     * in the library.
     * @param aLib - the lib part to be saved.
     * @param aPromptUser true to ask for confirmation, when the part_lib is already existing
     *      in memory, false to save silently
     * @return true if the part was saved, false if aborted by user
     */
    bool SaveOnePart( PART_LIB* aToSave, bool aPromptUser = false );

    /**
     * Function SaveAllLibraries
     * it the command event handler to save the changes to the current library.
     *
     * A backup file of the current library is saved with the .bak extension before the
     * changes made to the library are saved.
     */
    void SaveAllLibraries( wxCommandEvent& aEvent );

    void OnUpdateAnyLib( wxUpdateUIEvent& aEvent );

    /**
     * Reverts all the unsaved changes to the selected library.
     */
    void RevertLibrary( wxCommandEvent& aEvent );

    /**
     * Loads a part in the editor.
     * @param aAlias is the alias name.
     * @param aLibrary is the library name.
     * @param aUnit is the unit number.
     */
    bool LoadPart( const wxString& aAlias, const wxString& aLibrary, int aUnit = 1 );

    /**
     * Function LoadOneLibraryPartAux
     * loads a copy of \a aLibAlias from \a aLibrary into memory.
     *
     * @param aLibAlias A pointer to the LIB_ALIAS object to load.
     * @param aLibrary A pointer to the PART_LIB object to load \a aLibAlias from.
     * @return True if a copy of \a aLibAlias was successfully loaded from \a aLibrary.
     */
    bool LoadOneLibraryPartAux( LIB_ALIAS* aLibAlias, PART_LIB* aLibrary );

    /**
     * Function DisplayCmpDoc
     * displays the documentation of the selected component.
     */
    void DisplayCmpDoc();

    /**
     * Function OnRotateItem
     * rotates the current item.
     */
    void OnRotateItem( wxCommandEvent& aEvent );

    /**
     * Function OnOrient
     * Handles the ID_LIBEDIT_MIRROR_X and ID_LIBEDIT_MIRROR_Y events.
     */
    void OnOrient( wxCommandEvent& aEvent );

    /**
     * Function deleteItem
     * deletes the currently selected draw item.
     * @param aDC The device context to draw upon when removing item.
     */
    void deleteItem( wxDC* aDC );

private:
    void GetComponentFromUndoList( wxCommandEvent& aEvent );
    void GetComponentFromRedoList( wxCommandEvent& aEvent );

    // Editing pins
    void CreatePin( wxDC* aDC );
    void StartMovePin( wxDC* aDC );

    /**
     * Function CreateImagePins
     * adds copies of \a aPin for \a aUnit in components with multiple parts and
     * \a aConvert for components that have multiple body styles.
     *
     * @param aPin The pin to copy.
     * @param aUnit The unit to add a copy of \a aPin to.
     * @param aConvert The alternate body style to add a copy of \a aPin to.
     * @param aDeMorgan Flag to indicate if \a aPin should be created for the
     *                  alternate body style.
     */
    void CreateImagePins( LIB_PIN* aPin, int aUnit, int aConvert, bool aDeMorgan );

    /**
     * Function PlaceAnchor
     * places an  anchor reference coordinate for the current component.
     * <p>
     * All object coordinates are offset to the current cursor position.
     * </p>
     */
    void PlaceAnchor();

    // Editing graphic items
    LIB_ITEM* CreateGraphicItem( LIB_PART*      aLibAlias, wxDC* aDC );
    void GraphicItemBeginDraw( wxDC* aDC );
    void StartMoveDrawSymbol( wxDC* aDC );
    void StartModifyDrawSymbol( wxDC* aDC ); //<! Modify the item, adjust size etc.
    void EndDrawGraphicItem( wxDC* aDC );

    /**
     * Function LoadOneSymbol
     * read a component symbol file (*.sym ) and add graphic items to the current component.
     * <p>
     * A symbol file *.sym has the same format as a library, and contains only
     * one symbol.
     * </p>
     */
    void LoadOneSymbol();

    /**
     * Function SaveOneSymbol
     * saves the current component to a symbol file.
     * <p>
     * The symbol file format is similar to the standard component library file format, but
     * there is only one symbol.  Invisible pins are not saved.
     */
    void SaveOneSymbol();

    /**
     * Checks if aPart has the same name and library name as the currently modified part.
     */
    bool SameAsCurrentPart( const LIB_PART* aPart ) const;

    void EditGraphicSymbol( wxDC* aDC, LIB_ITEM* aDrawItem );
    void EditSymbolText( wxDC* aDC, LIB_ITEM* aDrawItem );
    LIB_ITEM* LocateItemUsingCursor( const wxPoint& aPosition,
                                     const KICAD_T aFilterList[] = LIB_COLLECTOR::AllItems );
    void EditField( LIB_FIELD* Field );

    void refreshSchematic();

public:
    /**
     * Function LoadComponentAndSelectLib
     * selects the current active library.
     *
     * @param aLibrary The PART_LIB to select
     * @param aLibEntry The component to load from aLibrary (can be an alias).
     * @return true if \a aLibEntry was loaded from \a aLibrary.
     */
    bool LoadComponentAndSelectLib( LIB_ALIAS* aLibEntry, PART_LIB* aLibrary );

    /* Block commands: */

    /**
     * Function BlockCommand
     * returns the block command (BLOCK_MOVE, BLOCK_COPY...) corresponding to
     * the \a aKey (ALT, SHIFT ALT ..)
     */
    virtual int BlockCommand( EDA_KEY aKey ) override;

    /**
     * Function HandleBlockPlace
     * handles the block place command.
     */
    virtual void HandleBlockPlace( wxDC* DC ) override;

    /**
     * Function HandleBlockEnd
     * performs a block end command.
     * @return If command finished (zoom, delete ...) false is returned otherwise true
     *         is returned indicating more processing is required.
     */
    virtual bool HandleBlockEnd( wxDC* DC ) override;

    /**
     * Function PlacePin
     * Place at cursor location the pin currently moved (i.e. pin pointed by m_drawItem)
     * (and the linked pins, if any)
     */
    void PlacePin();

    /**
     * Function GlobalSetPins
     * @param aMasterPin is the "template" pin
     * @param aId is a param to select what should be mofified:
     * - aId = ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINNAMESIZE_ITEM:
     *          Change pins text name size
     * - aId = ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINNUMSIZE_ITEM:
     *          Change pins text num size
     * - aId = ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINSIZE_ITEM:
     *          Change pins length.
     *
     * If aMasterPin is selected ( .m_flag == IS_SELECTED ),
     * only the other selected pins are modified
     */
    void GlobalSetPins( LIB_PIN* aMasterPin, int aId );

    // Automatic placement of pins
    void RepeatPinItem( wxDC* DC, LIB_PIN* Pin );

    /**
     * Function CreatePNGorJPEGFile
     * creates an image (screenshot) of the current component in PNG or JPEG format.
     * @param aFileName = the full filename
     * @param aFmt_jpeg = true to use JPEG file format, false to use PNG file format
     */
    void CreatePNGorJPEGFile( const wxString& aFileName, bool aFmt_jpeg );

    /**
     * Virtual function PrintPage
     * used to print a page
     * @param aDC = wxDC given by the calling print function
     * @param aPrintMask = not used here
     * @param aPrintMirrorMode = not used here (Set when printing in mirror mode)
     * @param aData = a pointer on an auxiliary data (not always used, NULL if not used)
     */
    virtual void PrintPage( wxDC* aDC, LSET aPrintMask,
                            bool aPrintMirrorMode, void* aData = NULL ) override;

    /**
     * Function SVG_PlotComponent
     * Creates the SVG print file for the current edited component.
     * @param aFullFileName = the full filename
     */
    void SVG_PlotComponent( const wxString& aFullFileName );

    void UpdateLibraries( wxIdleEvent& aEvent );

    SCH_SCREEN* m_dummyScreen;

    /**
     * Opens a file open dialog to either import an existing library or create a new one
     * @bool aCreateNew decides whether the file should be imported or created.
     * @return True in case of success.
     */
    bool addLibraryFile( bool aCreateNew );

    /**
     * Opens a file selection dialog to either open an existing file or choose a name for a new one.
     * @param Decides whether the returned file name should be an existing file or a new one.
     */
    wxFileName getLibraryFileName( bool aExisting );

    /**
     * Saves the currently modified part in the buffer.
     */
    void storeCurrentPart();

    /**
     * Clears the editor screen, so there is no edited component/library.
     */
    void emptyScreen();

    /**
     * Clears the clipboard contents.
     */
    void clearClipboard();

    ///> Library Manager, stores temporary copies of the edited parts
    std::unique_ptr<LIB_MANAGER> m_libMgr;

    ///> Clipboard stores parts that are copied between different libraries.
    std::set<LIB_PART*> m_clipboard;

    ///> Hash of all loaded libraries to determine whether the list of parts has changed.
    int m_curHash;

    DECLARE_EVENT_TABLE()
};

#endif  // LIBEDITFRM_H_
