set(LIB xxhash)

include_directories(.)

set(LIB_HEADERS
    utils.h
    xxhash.h
    )

set(LIB_SOURCES
    xxhash.c
    )

#Library
add_library(sniper_${LIB} STATIC ${LIB_HEADERS} ${LIB_SOURCES})

set(DEPENDENCIES "${DEPENDENCIES}" "std" PARENT_SCOPE)
set(SNIPER_LIBRARIES ${SNIPER_LIBRARIES} "sniper_${LIB}" CACHE INTERNAL "sniper_libraries")
