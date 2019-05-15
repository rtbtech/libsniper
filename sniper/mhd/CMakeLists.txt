set(LIB mhd)

include_directories(.)

set(LIB_HEADERS
    MHD.h
    )

set(LIB_SOURCES
    MHD.cpp
    )


#Library
add_library(sniper_${LIB} STATIC ${LIB_HEADERS} ${LIB_SOURCES})
target_link_libraries(sniper_${LIB} rt.a)
target_link_libraries(sniper_${LIB} uriparser)
target_link_libraries(sniper_${LIB} microhttpd.a)

set(SNIPER_LIBRARIES ${SNIPER_LIBRARIES} "sniper_${LIB}" CACHE INTERNAL "sniper_libraries")
