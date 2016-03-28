
if(WIN32)
    find_path(QT5_BASE_DIR
        NAMES "lib/cmake/Qt5"
        HINTS ENV QT5_ROOT
        PATH_SUFFIXES "5.6/msvc2015_64"
        DOC "Qt5 Root Directory"
    )

    set(QT5_ADDITIONAL_DLLS
        "${QT5_BASE_DIR}/bin/icuin54.dll"
        "${QT5_BASE_DIR}/bin/icuuc54.dll"
        "${QT5_BASE_DIR}/bin/icudt54.dll"
    )

    if(NOT QT5_NO_OPENGL)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(SEARCH_SUFFIX "Lib/x64" "Lib/win8/um/x64")
        else()
            set(SEARCH_SUFFIX "Lib" "Lib/win8/um/x86")
        endif()
        find_path(WINSDK_LIB_DIR
            NAMES Glu32.lib
            HINTS ENV WindowsSdkDir ENV WINSDK_ROOT
            PATH_SUFFIXES ${SEARCH_SUFFIX}
        )
    endif()
endif()


function(getPDBForDLL DLL_PATH OUT_VAR)
    get_filename_component(dll_dir ${DLL_PATH} DIRECTORY)
    get_filename_component(dll_we ${DLL_PATH} NAME_WE)
    set(${OUT_VAR} "${dll_dir}/${dll_we}.pdb" PARENT_SCOPE)
endfunction()


function(getQt5Dlls QT_TARGET OUT_VAR)
    unset(DLLS)
    get_property(tmp TARGET ${QT_TARGET} PROPERTY IMPORTED_LOCATION_DEBUG)
    list(APPEND DLLS ${tmp})
    getPDBForDLL(${tmp} tmp_pdb)
    if (EXISTS ${tmp_pdb})
        list(APPEND DLLS ${tmp_pdb})
    endif()
    get_property(tmp TARGET ${QT_TARGET} PROPERTY IMPORTED_LOCATION_RELEASE)
    list(APPEND DLLS ${tmp})
    get_property(deps TARGET ${QT_TARGET} PROPERTY IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG)
    foreach(dep ${deps})
        if(TARGET ${dep})
            getQt5Dlls(${dep} tmp)
            list(APPEND DLLS ${tmp})
        endif()
    endforeach()
    set(result ${${OUT_VAR}})
    list(APPEND result ${DLLS})
    list(REMOVE_DUPLICATES result)
    set(${OUT_VAR} ${result} PARENT_SCOPE)
endfunction()
