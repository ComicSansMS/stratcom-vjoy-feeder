
if(WIN32)
    find_path(QT5_BASE_DIR
        NAMES "lib/cmake/Qt5"
        HINTS ENV QT5_ROOT
        PATH_SUFFIXES "5.7/msvc2015_64" "5.11.1/msvc2017_64"
        DOC "Qt5 Root Directory"
    )
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
