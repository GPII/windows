g++ -I.\include -O0 -g3 -Wall -c -fmessage-length=0  -o src\UsbUserListener.o src\UsbUserListener.cpp
g++ -L.\lib -static-libgcc -o UsbUserListener.exe src\UsbUserListener.o -lcurldll