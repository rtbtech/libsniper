set(LIB cache)

include_directories(.)


set(LIB_HEADERS
    ArrayCache.h
    FreeList.h
    Cache.h
    Cache_int.h
    Singleton.h
    )


set(DEPENDENCIES "${DEPENDENCIES}" "std" PARENT_SCOPE)
add_custom_target(sniper_${LIB} SOURCES ${LIB_HEADERS})
