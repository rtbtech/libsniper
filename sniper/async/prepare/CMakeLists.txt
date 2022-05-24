set(LIB prepare)

include_directories(.)

set(LIB_SOURCES
        Prepare.h
        Prepare.cpp
        )

find_package(Libev REQUIRED)
include_directories(SYSTEM ${LIBEV_INCLUDE_DIR})

add_library(rtbtech_${LIB} STATIC ${LIB_SOURCES})
target_link_libraries(rtbtech_${LIB} ${LIBEV_LIBRARY})

set(DEPENDENCIES "${DEPENDENCIES}" "std" PARENT_SCOPE)
set(SNIPER_LIBRARIES ${SNIPER_LIBRARIES} "rtbtech_${LIB}" CACHE INTERNAL "sniper_libraries")
