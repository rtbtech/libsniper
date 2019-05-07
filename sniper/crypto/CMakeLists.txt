set(LIB crypto)

include_directories(.)

set(LIB_HEADERS
    digest.h
    secp.h
    hash.h
    openssl.h
    asn1.h
    )

set(LIB_SOURCES
    digest.cpp
    hash.cpp
    asn1.cpp
    openssl.cpp
    secp.cpp
    )

#Library
add_library(sniper_${LIB} STATIC ${LIB_HEADERS} ${LIB_SOURCES})
target_link_libraries(sniper_${LIB} ${SECP256K1_LIBRARY} ${OPENSSL_LIBRARIES} dl)

set(SNIPER_LIBRARIES ${SNIPER_LIBRARIES} "sniper_${LIB}" CACHE INTERNAL "sniper_libraries")
