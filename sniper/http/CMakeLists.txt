set(LIB http)

include_directories(.)

set(LIB_HEADERS
    Server.h
    Client.h
    SyncClient.h
    utils.h
    Wait.h
    server/Config.h
    server/Connection.h
    server/Request.h
    server/Response.h
    server/Status.h
    client/Connection.h
    client/Request.h
    client/Response.h
    client/Config.h
    client/Pool.h
    wait/Group.h
    )

set(LIB_SOURCES
    Server.cpp
    Client.cpp
    SyncClient.cpp
    utils.cpp
    Wait.cpp
    server/Connection.cpp
    server/Request.cpp
    server/Response.cpp
    server/Status.cpp
    client/Connection.cpp
    client/Request.cpp
    client/Response.cpp
    client/Pool.cpp
    wait/Group.cpp
    )

#Library
add_library(sniper_${LIB} STATIC ${LIB_HEADERS} ${LIB_SOURCES})
target_link_libraries(sniper_${LIB} ev.a)
target_link_libraries(sniper_${LIB} sniper_event)
target_link_libraries(sniper_${LIB} sniper_net)
target_link_libraries(sniper_${LIB} sniper_pico)

set(SNIPER_LIBRARIES ${SNIPER_LIBRARIES} "sniper_${LIB}" CACHE INTERNAL "sniper_libraries")
