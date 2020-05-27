set(LIB event)

include_directories(.)

set(LIB_SRC
        Loop.h
        Timer.h
        TimerDetail.h
        Prepare.h
        Touch.h
        Sig.h
        Resolve.h
        Touch.cpp
        Prepare.cpp
        Timer.cpp
        TimerDetail.cpp
        Sig.cpp
        Resolve.cpp
        Wait.h
        Wait.cpp
        wait/Group.h
        wait/Group.cpp
        wait/Pool.h
        wait/Pool.cpp
        )

find_package(Libev REQUIRED)
include_directories(SYSTEM ${LIBEV_INCLUDE_DIR})

#Library
add_library(sniper_${LIB} STATIC ${LIB_SRC})
target_link_libraries(sniper_${LIB} ${LIBEV_LIBRARY})

set(DEPENDENCIES "${DEPENDENCIES}" "std" "net" PARENT_SCOPE)
set(SNIPER_LIBRARIES ${SNIPER_LIBRARIES} "sniper_${LIB}" CACHE INTERNAL "sniper_libraries")
