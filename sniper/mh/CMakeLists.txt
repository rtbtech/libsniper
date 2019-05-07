set(LIB mh)

include_directories(.)

set(LIB_HEADERS
    Key.h
    TX.h
    varint.h
    )

set(LIB_SOURCES
    Key.cpp
    TX.cpp
    varint.cpp
    )

#Library
add_library(sniper_${LIB} STATIC ${LIB_HEADERS} ${LIB_SOURCES})
target_link_libraries(sniper_${LIB} sniper_crypto)

set(SNIPER_LIBRARIES ${SNIPER_LIBRARIES} "sniper_${LIB}" CACHE INTERNAL "sniper_libraries")
