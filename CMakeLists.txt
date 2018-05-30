cmake_minimum_required(VERSION 3.9)
project(net-snmp)

set(CMAKE_VERBOSE_MAKEFILE on)

file(GLOB_RECURSE SRCS "*.c" "*.h")

add_executable(zdummy ${SRCS})

#add_custom_target(snmpd COMMAND cd ${PROJECT_SOURCE_DIR}/build && make)
add_custom_target(snmpd COMMAND echo)

include_directories(
        include/
        build/include/  # include "<net-snmp/net-snmp-config.h>"
)

add_definitions(-DHAVE_CONFIG_H)

#[[
cd build/
../configure --prefix=$HOME/usr/opt/net-snmp CFLAGS='-ggdb3 -O0'
make install # because can't debug build/agent/[.libs/]snmpd

sudo mkdir /var/net-snmp/
sudo vim /var/net-snmp/snmpd.conf

sysdescr sys descr sys descr
sysname ""
syslocation ""
syscontact ""
sysservices 78
rocommunity "public"
rocommunity6 "public"

agent/snmpd -DALL -f -Le -r udp:10161 udp6:10161
agent/.libs/snmpd -DALL -f -Le -r udp:10161 udp6:10161

snmpwalk -v 2c -c public localhost:10161 .1.3.6.1.2.1.5.1.0
snmpwalk -v 2c -c public UDP6:localhost:10161 .1.3.6.1.2.1.5.1.0
]]
