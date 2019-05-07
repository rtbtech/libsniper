set(LIB threads)

include_directories(.)


set(LIB_HEADERS
    Spinlock.h
    Stop.h
    )


add_custom_target(sniper_${LIB} SOURCES ${LIB_HEADERS})
