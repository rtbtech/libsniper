set(LIB event)

include_directories(.)

set(LIB_HEADERS
    Loop.h
    Timer.h
    TimerDetail.h
    Prepare.h
    Touch.h
    Sig.h
    Resolve.h
    )

set(LIB_SOURCES
    Touch.cpp
    Prepare.cpp
    Timer.cpp
    TimerDetail.cpp
    Sig.cpp
    Resolve.cpp
    )

#Library
add_library(sniper_${LIB} STATIC ${LIB_HEADERS} ${LIB_SOURCES})

set(SNIPER_LIBRARIES ${SNIPER_LIBRARIES} "sniper_${LIB}" CACHE INTERNAL "sniper_libraries")
