/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * https://www.gnu.org/licenses/gpl-3.0.html
 * or you may search the http://www.gnu.org website for the version 3 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef LIB_MANAGER_H
#define LIB_MANAGER_H

#include <map>
#include <set>
#include <memory>
#include <wx/arrstr.h>

class LIB_ALIAS;
class LIB_PART;
class PART_LIB;
class PART_LIB_BUFFER;
class SCH_SCREEN;
class LIB_EDIT_FRAME;

/**
 * Class to handle modifications to the symbol libraries.
 */
class LIB_MANAGER
{
public:
    LIB_MANAGER( LIB_EDIT_FRAME& aFrame );

    /**
     * Updates the LIB_MANAGER data to account for the changes introduced to the project libraries.
     * @see PROJECT::SchLibs()
     */
    void Sync();

    /**
     * Returns the array of library names.
     */
    wxArrayString GetLibraryNames() const;

    /**
     * Returns a set containing all part names for a specific library.
     */
    wxArrayString GetPartNames( const wxString& aLibrary ) const;

    /**
     * Creates an empty library and adds it to the project. The library file is created.
     */
    bool CreateLibrary( const wxString& aName, const wxString& aFileName );

    /**
     * Adds an existing library. The library is added to the project as well.
     */
    bool AddLibrary( PART_LIB* aLibrary );

    //bool RenameLibrary( const wxString& aOld, const wxString& aNew );

    /**
     * Removes a library. The library is removed from the project libraries as well.
     */
    bool RemoveLibrary( const wxString& aName );

    /**
     * Updates the part buffer with a new version of a part.
     * It is required to save the library to use the updated part in the schematic editor.
     */
    bool UpdatePart( LIB_PART* aPart, const wxString& aLibrary );

    /**
     * Renames the part in the part buffer.
     * It is required to save the library to use the new name in the schematic editor.
     */
    bool RenamePart( const wxString& aOld, const wxString& aNew, const wxString& aLibrary );

    /**
     * Removes the part from the part buffer.
     * It is required to save the library to have the part removed in the schematic editor.
     */
    bool RemovePart( const wxString& aName, const wxString& aLibrary );

    /**
     * Returns a library copy used for editing.
     */
    PART_LIB* GetLibraryCopy( const wxString& aLibrary );

    /**
     * Returns the most recent version of a part (either from the part buffer or from the original
     * library) or nullptr if it does not exist.
     */
    LIB_PART* GetPart( const wxString& aPart, const wxString& aLibrary ) const;

    /**
     * Returns the part copy from the buffer. In case it does not exist yet, the copy is created.
     * LIB_MANAGER retains the ownership.
     */
    LIB_PART* GetPartCopy( const wxString& aPart, const wxString& aLibrary );

    /**
     * Returns the screen used to edit a specific part. If it does not exist, it is created.
     * LIB_MANAGER retains the ownership.
     */
    SCH_SCREEN* GetScreen( const wxString& aPart, const wxString& aLibrary );

    /**
     * Returns true if aPart exists in aLibrary. Part is considered existent if it is available
     * in the part buffer.
     */
    bool PartExists( const wxString& aPart, const wxString& aLibrary ) const;

    /**
     * Returns true if aLibrary exists.
     */
    bool LibraryExists( const wxString& aLibrary ) const
    {
        return m_libs.count( aLibrary ) > 0;
    }

    /**
     * Returns true if aLibrary has unsaved modifications.
     */
    bool IsLibraryModified( const wxString& aLibrary ) const;

    /**
     * Returns true if aPart has unsaved modifications.
     */
    bool IsPartModified( const wxString& aPart, const wxString& aLibrary ) const;

    /**
     * Returns true if the library is stored in a read-only file.
     */
    bool IsLibraryReadOnly( const wxString& aLibrary ) const;

    /**
     * Saves part modifications to its library. Library has to be saved afterwards to
     * offer the updated part in the schematic editor.
     */
    bool SavePart( const wxString& aPart, const wxString& aLibrary );

    /**
     * Saves library modifications to a file.
     * @param aLibName is the library name.
     * @param aFileName is the target file. If empty, the original library path is used.
     * @return True on success, false otherwise.
     */
    bool SaveLibrary( const wxString& aLibName, const wxString& aFileName = wxEmptyString );

    /**
     * Saves all libraries to disk.
     */
    bool SaveAll();

    /**
     * Reverts unsaved changes for a particular part.
     */
    bool RevertPart( const wxString& aPart, const wxString& aLibrary );

    /**
     * Reverts unsaved changes for a library.
     */
    bool RevertLibrary( const wxString& aLibrary );

    /**
     * Computes the hash of all stored libraries.
     */
    int GetHash() const;

    /**
     * Replaces all characters considered illegal in library/part names with underscores.
     */
    static wxString ValidateName( const wxString& aName );

    /**
     * Retrieves the original name of a part if it was renamed. If it was not, the current name
     * is returned.
     */
    wxString GetOriginalName( const wxString& aPart, const wxString& aLibrary ) const;

    /**
     * Returns a library name that is not currently in use.
     * Used for generating names for new libraries.
     */
    wxString GetUniqueLibraryName() const;

    /**
     * Returns a component name that is not stored in a library.
     * Used for generating names for new components.
     */
    wxString GetUniqueComponentName( const wxString& aLibrary ) const;

private:
    ///> Parent frame
    LIB_EDIT_FRAME& m_frame;

    ///> Structure to handle changes to a library
    struct LIB_BUFFER
    {
        LIB_BUFFER( const PART_LIB* aLibrary = nullptr );

        PART_LIB* getLib() const
        {
            return m_lib.get();
        }

        ///> Sets the library to be buffered. LIB_BUFFER creates a copy of the library,
        ///> all the temporary changes are saved there.
        void setLib( const PART_LIB* aLibrary );

        ///> SCH_SCREENs storing currently modified parts and undo buffers
        std::map<wxString, std::unique_ptr<SCH_SCREEN>> m_screens;

        ///> Modification flag (used e.g. for marking changes when a part is removed)
        bool m_modified;

        ///> Map old names to new ones
        std::map<wxString, wxString> m_renamed;

    private:
        ///> Copy of the modified library
        std::unique_ptr<PART_LIB> m_lib;
    };

    ///> Returns a library copy for editing, if a library does not exist - it is not created.
    PART_LIB* getLib( const wxString& aLibrary ) const
    {
        auto it = m_libs.find( aLibrary );
        return it != m_libs.end() ? it->second.getLib() : nullptr;
    }

    ///> Stores a part in a library (either by adding or replacing the part).
    bool updatePart( PART_LIB* aLibrary, LIB_PART* aPart );

    ///> Returns the original library used in the project
    PART_LIB* getOriginalLib( const wxString& aLibrary ) const;

    ///> Create an empty library file
    bool touchLibraryFile( const wxString& aFileName ) const;

    ///> Adds a library to the project configuration
    void cfgLibsAdd( const PART_LIB* aLibrary );

    ///> Removes a library from the project configuration
    void cfgLibsRemove( const PART_LIB* aLibrary );

    ///> The library buffer
    std::map<wxString, LIB_BUFFER> m_libs;
};

#endif /* LIB_MANAGER_H */
