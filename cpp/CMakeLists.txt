cmake_minimum_required(VERSION 3.5)

project(ttcp LANGUAGES CXX)

set(APP_EXE ttcp)

set(TTCP_HEADERS common.h inet_address.h socket.h accept.h tcp_stream.h message.h command.h)

set(TTCP_SRCS socket.cc inet_address.cc accept.cc tcp_stream.cc command.cc)

add_library(ttpclib STATIC  ${TTCP_SRCS} ${TTCP_HEADERS})

add_executable(${APP_EXE} ttcp.cc)

target_link_libraries(${APP_EXE} ttpclib)

# add_executable(inet_address_test inet_address_test.cc)

# target_link_libraries(inet_address_test ttpclib)

# tcp_stream_test
# add_executable(tcp_stream_test tcp_stream_test.cc)

# target_link_libraries(tcp_stream_test ttpclib)