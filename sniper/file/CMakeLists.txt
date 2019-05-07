set(LIB file)

include_directories(.)


set(LIB_HEADERS
    load.h
    )

add_custom_target(sniper_${LIB} SOURCES ${LIB_HEADERS})
