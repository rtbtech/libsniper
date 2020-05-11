set(LIB file)

include_directories(.)


set(LIB_HEADERS
    load.h
    )

set(DEPENDENCIES "${DEPENDENCIES}" "std" PARENT_SCOPE)
add_custom_target(sniper_${LIB} SOURCES ${LIB_HEADERS})
