include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/src)

set(MYVPN_FILES myvpn.c myudp.c mytun.c mynode.c myaddr.c)

find_library(EV_LIB ev)

add_executable(myvpn ${MYVPN_FILES})
target_link_libraries(myvpn myfec ${EV_LIB})
