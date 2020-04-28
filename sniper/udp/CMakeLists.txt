set(LIB udp)

include_directories(.)

set(LIB_HEADERS
    UDP.h
    )

set(LIB_SOURCES
    UDP.cpp
    )

#Library
add_library(sniper_${LIB} STATIC ${LIB_HEADERS} ${LIB_SOURCES})

set(SNIPER_LIBRARIES ${SNIPER_LIBRARIES} "sniper_${LIB}" CACHE INTERNAL "sniper_libraries")
