set(LIB async)
include_directories(.)

set(LIB_HEADERS
        Loop.h
        )

if (DEFINED RTBTECH_DEPS)
    foreach (dir ${RTBTECH_DEPS})
        if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir} AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${dir})
            message("Added: ${dir}")
            add_subdirectory(${dir})
        endif ()
    endforeach ()
endif ()

set(DEPENDENCIES "${DEPENDENCIES}" "std" PARENT_SCOPE)
add_custom_target(rtbtech_${LIB} SOURCES ${LIB_HEADERS})