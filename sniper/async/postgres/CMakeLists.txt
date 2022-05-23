set(LIB postgres)

include_directories(.)

set(LIB_SOURCES
        DB.h
        DB.cpp
        Query.h
        Query.cpp
        Request.h
        Request.cpp
        Response.h
        Response.cpp
        )

find_package(fmt REQUIRED)

add_library(rtbtech_${LIB} STATIC ${LIB_SOURCES})
target_link_libraries(rtbtech_${LIB} fmt::fmt)

set(DEPENDENCIES "${DEPENDENCIES}" "std" "cache" "log" "event" PARENT_SCOPE)
set(SNIPER_LIBRARIES ${SNIPER_LIBRARIES} "rtbtech_${LIB}" CACHE INTERNAL "sniper_libraries")
