set(LIB http)

include_directories(.)

set(LIB_SRC
    Server.h
    Server.cpp
    Client.h
    Client.cpp
    SyncClient.h
    SyncClient.cpp
    utils.h
    utils.cpp
    Wait.h
    Wait.cpp
    server/Config.h
    server/Connection.h
    server/Connection.cpp
    server/Request.h
    server/Request.cpp
    server/Response.h
    server/Response.cpp
    server/Status.h
    server/Status.cpp
    client/Connection.h
    client/Connection.cpp
    client/Request.h
    client/Request.cpp
    client/Response.h
    client/Response.cpp
    client/Config.h
    client/Pool.h
    client/Pool.cpp
    wait/Group.h
    wait/Group.cpp

    Server2.h
    Server2.cpp
    server2/ServerInt.h
    server2/ServerInt.cpp
    server2/Connection.h
    server2/Connection.cpp
    server2/Pool.h
    server2/Pool.cpp
    server2/Request.h
    server2/Request.cpp
    server2/Response.h
    server2/Response.cpp
    server2/Buffer.h
    server2/Buffer.cpp
    server2/Config.h
    )

#Library
add_library(sniper_${LIB} STATIC ${LIB_SRC})
target_link_libraries(sniper_${LIB} sniper_event)
target_link_libraries(sniper_${LIB} sniper_net)
target_link_libraries(sniper_${LIB} sniper_pico)

set(SNIPER_LIBRARIES ${SNIPER_LIBRARIES} "sniper_${LIB}" CACHE INTERNAL "sniper_libraries")
