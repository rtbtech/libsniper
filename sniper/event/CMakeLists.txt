set(LIB event)

include_directories(.)

set(LIB_HEADERS
    Loop.h
    Timer.h
    TimerDetail.h
    Prepare.h
    Touch.h
    Sig.h
    )

set(LIB_SOURCES
    Touch.cpp
    Prepare.cpp
    Timer.cpp
    TimerDetail.cpp
    Sig.cpp
    )

#Library
add_library(sniper_${LIB} STATIC ${LIB_HEADERS} ${LIB_SOURCES})
target_link_libraries(sniper_${LIB} ev.a)

set(SNIPER_LIBRARIES ${SNIPER_LIBRARIES} "sniper_${LIB}" CACHE INTERNAL "sniper_libraries")
