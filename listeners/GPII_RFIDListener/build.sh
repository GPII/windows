#!/bin/bash 
#
# Build script for GPII_RFIDListener
#
# Can be run in MinGW Bash shell 
# Requires mingw32-base and mingw32-gcc-g++ to be installed
#
COPTS_DEBUG='-Og -g3 -Wall -Werror -fmessage-length=0 -DWIN32 -DDEBUG -D_DEBUG -DWIN32_LEAN_AND_MEAN' 
COPTS_RELEASE='-g0 -O3 -Wall -Werror -fmessage-length=0 -DWIN32 -DNDEBUG -DWIN32_LEAN_AND_MEAN' 
COPTS=$COPTS_DEBUG

mkdir -p build
g++ -c $COPTS -o build/GPII_RFIDListener.o GPII_RFIDListener.cpp
g++ -I./winscard -c $COPTS -Wno-sign-compare -o build/WinSmartCard.o WinSmartCard.cpp
g++ -I./libcurl -c $COPTS -DCURL_STATICLIB -o build/FlowManager.o FlowManager.cpp
dlltool -k -d winscard/WinSCard.def -l build/WinSCard.lib
g++ -static-libgcc -static-libstdc++ -o build/GPII_RFIDListener.exe build/FlowManager.o build/WinSmartCard.o build/GPII_RFIDListener.o -L./build -L./libcurl -lWinSCard -lcurl -lwsock32 -lWs2_32
