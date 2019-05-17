# see en

cmake_minimum_required(VERSION 3.9)
project(net-snmp)

set(CMAKE_VERBOSE_MAKEFILE on)

file(GLOB_RECURSE SRCS "*.c" "*.h")

add_executable(zdummy ${SRCS})

#add_custom_target(snmpd COMMAND cd ${PROJECT_SOURCE_DIR}/build && make)
add_custom_target(snmpd COMMAND echo)

include_directories(
        include/
        apps/           # include "../agent_global_vars.h"
        build/include/  # include "<net-snmp/net-snmp-config.h>"
)

add_definitions(-DHAVE_CONFIG_H)
