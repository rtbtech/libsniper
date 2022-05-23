set(LIB wait)

include_directories(.)

set(LIB_HEADERS
        Group.h
        )

#find_package(fmt REQUIRED)

set(DEPENDENCIES "${DEPENDENCIES}" "std" "cache" "event" "log" PARENT_SCOPE)
add_custom_target(rtbtech_${LIB} SOURCES ${LIB_HEADERS})
