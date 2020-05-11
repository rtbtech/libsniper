set(LIB net)

include_directories(.)

set(LIB_SOURCES
        ip.h
        socket.h
        Url.h
        Peer.h
        Domain.h
        hostname.h
        ip.cpp
        socket.cpp
        Url.cpp
        Peer.cpp
        Domain.cpp
        hostname.cpp
        )

find_package(fmt REQUIRED)
find_package(Libre2 REQUIRED)
find_package(Libhttp_parser REQUIRED)
include_directories(SYSTEM ${LIBRE2_INCLUDE_DIR})
include_directories(SYSTEM ${LIBHTTP_PARSER_INCLUDE_DIR})
#Library
add_library(sniper_${LIB} STATIC ${LIB_SOURCES})
target_link_libraries(sniper_${LIB} dl ${LIBRE2_LIBRARY})
target_link_libraries(sniper_${LIB} ${LIBHTTP_PARSER_LIBRARY})
target_link_libraries(sniper_${LIB} fmt::fmt)

set(DEPENDENCIES "${DEPENDENCIES}" "std" "xxhash" PARENT_SCOPE)
set(SNIPER_LIBRARIES ${SNIPER_LIBRARIES} "sniper_${LIB}" CACHE INTERNAL "sniper_libraries")
