set(LIB xxhash)

include_directories(.)

set(LIB_SOURCES
        utils.h
        utils.cpp
        )

find_package(Libxxhash REQUIRED)
include_directories(SYSTEM ${LIBXXHASH_INCLUDE_DIR})

#Library
add_library(sniper_${LIB} STATIC ${LIB_SOURCES})
target_link_libraries(sniper_${LIB} ${LIBXXHASH_LIBRARY})

set(DEPENDENCIES "${DEPENDENCIES}" "std" PARENT_SCOPE)
set(SNIPER_LIBRARIES ${SNIPER_LIBRARIES} "sniper_${LIB}" CACHE INTERNAL "sniper_libraries")
