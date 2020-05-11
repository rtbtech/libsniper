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
    Buffer.h
    Buffer.cpp
    server/ServerInt.h
    server/ServerInt.cpp
    server/Connection.h
    server/Connection.cpp
    server/Pool.h
    server/Pool.cpp
    server/Request.h
    server/Request.cpp
    server/Response.h
    server/Response.cpp
    server/Config.h
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
    )

#Library
add_library(sniper_${LIB} STATIC ${LIB_SRC})
# fmt
#


set(DEPENDENCIES "${DEPENDENCIES}" "std" "cache" "log" "event" "net" "pico" PARENT_SCOPE)
set(SNIPER_LIBRARIES ${SNIPER_LIBRARIES} "sniper_${LIB}" CACHE INTERNAL "sniper_libraries")

