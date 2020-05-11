if (DEFINED RTBTECH_DEPS)
    set(DEPENDENCIES "")

    foreach (dir ${RTBTECH_DEPS})
        if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir} AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${dir})
            message("Added: ${dir}")
            add_subdirectory(${dir})
        endif ()
    endforeach ()


    while (TRUE)
        set(STOP true)

        foreach (dir ${DEPENDENCIES})
            if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir} AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${dir})
                if (NOT ("${dir}" IN_LIST RTBTECH_DEPS))
                    set(RTBTECH_DEPS ${RTBTECH_DEPS} ${dir})
                    message("Added: ${dir}")
                    add_subdirectory(${dir})
                    set(STOP false)
                endif ()
            endif ()
        endforeach ()

        if (STOP)
            break()
        endif ()
    endwhile ()

endif ()
