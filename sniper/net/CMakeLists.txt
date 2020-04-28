set(LIB net)

include_directories(.)

set(LIB_HEADERS
    ip.h
    socket.h
    Url.h
    http_parser.h
    Peer.h
    Domain.h
    hostname.h
    )

set(LIB_SOURCES
    ip.cpp
    socket.cpp
    Url.cpp
    http_parser.c
    Peer.cpp
    Domain.cpp
    hostname.cpp
    )

#Library
add_library(sniper_${LIB} STATIC ${LIB_HEADERS} ${LIB_SOURCES})
target_link_libraries(sniper_${LIB} dl)
target_link_libraries(sniper_${LIB} sniper_xxhash)

set(SNIPER_LIBRARIES ${SNIPER_LIBRARIES} "sniper_${LIB}" CACHE INTERNAL "sniper_libraries")
