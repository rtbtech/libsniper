set(LIB strings)

include_directories(.)

set(LIB_HEADERS
    ascii_case.h
    split.h
    atoi.h
    hex.h
    trim.h
    join.h
    )

set(LIB_SOURCES
    ascii_case.cpp
    atoi.cpp
    hex.cpp
    trim.cpp
    )

#Library
add_library(sniper_${LIB} STATIC ${LIB_HEADERS} ${LIB_SOURCES})

set(SNIPER_LIBRARIES ${SNIPER_LIBRARIES} "sniper_${LIB}" CACHE INTERNAL "sniper_libraries")
