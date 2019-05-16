set(LIB lz4)

include_directories(.)


set(LIB_HEADERS
    lz4.h
    lz4frame.h
    lz4frame_static.h
    lz4hc.h
    lz4opt.h
    utils.h
    )


set(LIB_SOURCES
    lz4.c
    lz4frame.c
    lz4hc.c
    utils.cpp
    )



#Library
add_library(sniper_${LIB} STATIC ${LIB_HEADERS} ${LIB_SOURCES})
target_compile_options(sniper_${LIB} PRIVATE -Wno-implicit-fallthrough)

set(SNIPER_LIBRARIES ${SNIPER_LIBRARIES} "sniper_${LIB}" CACHE INTERNAL "sniper_libraries")
