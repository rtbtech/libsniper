set(LIB log)

include_directories(.)


set(LIB_HEADERS
    log.h
    )

find_package(fmt REQUIRED)

set(DEPENDENCIES "${DEPENDENCIES}" "std" PARENT_SCOPE)
add_custom_target(sniper_${LIB} SOURCES ${LIB_HEADERS})
