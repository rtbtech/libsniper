set(LIB std)

include_directories(.)


set(LIB_HEADERS
    vector.h
    chrono.h
    filesystem.h
    memory.h
    mutex.h
    boost_vector.h
    map.h
    set.h
    pair.h
    string.h
    error.h
    functional.h
    check.h
    list.h
    intrusive_cache.h
    deque.h
    optional.h
    tuple.h
    any.h
    boost_list.h
    variant.h
    rapidjson.h
    array.h
    )

add_custom_target(sniper_${LIB} SOURCES ${LIB_HEADERS})
