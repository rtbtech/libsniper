set(LIB pico)

include_directories(.)

set(LIB_HEADERS
    Request.h
    Response.h
    picohttpparser.h
    common.h
    )

set(LIB_SOURCES
    Request.cpp
    Response.cpp
    picohttpparser.c
    )

#Library
add_library(sniper_${LIB} STATIC ${LIB_HEADERS} ${LIB_SOURCES})
target_link_libraries(sniper_${LIB} sniper_strings)

set(SNIPER_LIBRARIES ${SNIPER_LIBRARIES} "sniper_${LIB}" CACHE INTERNAL "sniper_libraries")
