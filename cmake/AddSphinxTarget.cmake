# Handy function for creating the different Sphinx targets.
#
# ``builder`` should be one of the supported builders used by
# the sphinx-build command.
#
# ``project`` should be the project name
function(add_sphinx_target builder project)
    set(SPHINX_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/${builder}/sphinx")
    set(SPHINX_DOC_TREE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")
    set(SPHINX_TARGET_NAME docs-${project}-${builder})
    add_custom_target(${SPHINX_TARGET_NAME}
            COMMAND ${SPHINX_EXECUTABLE}
                    -b ${builder}
                    -d "${SPHINX_DOC_TREE_DIR}"
                    -q # Quiet: no output other than errors and warnings.
                    -W # Warnings are errors.
                    "${CMAKE_SOURCE_DIR}/Doc" # Source
                    "${SPHINX_BUILD_DIR}" # Output
            COMMENT
            "Generating ${builder} Sphinx documentation for ${project}")

    # When "clean" target is run, remove the Sphinx build directory
    set_property(DIRECTORY APPEND PROPERTY
            ADDITIONAL_MAKE_CLEAN_FILES
            "${SPHINX_BUILD_DIR}")

    # We need to remove ${SPHINX_DOC_TREE_DIR} when make clean is run
    # but we should only add this path once
    get_property(_CURRENT_MAKE_CLEAN_FILES
            DIRECTORY PROPERTY ADDITIONAL_MAKE_CLEAN_FILES)
    list(FIND _CURRENT_MAKE_CLEAN_FILES "${SPHINX_DOC_TREE_DIR}" _INDEX)
    if(_INDEX EQUAL -1)
        set_property(DIRECTORY APPEND PROPERTY
                ADDITIONAL_MAKE_CLEAN_FILES
                "${SPHINX_DOC_TREE_DIR}")
    endif()

    add_dependencies(sphinx ${SPHINX_TARGET_NAME})
endfunction()
