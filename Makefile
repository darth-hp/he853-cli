PROGRAM_NAME = he853
PREFIX = /usr/local
BIN_DIR = $(PREFIX)/bin
CC = gcc
INSTALL = install
INSTALL_PROGRAM = $(INSTALL) -m 0755

# optimize size and strip symbol tables
CXXFLAGS = -Os -s

# Try our best to get the include path
_IU=/include/libusb-1.0
_IC=$(shell pkg-config --silence-errors --cflags libusb-1.0 || [ -d /usr${_IU} ] && echo -I/usr${_IU} || echo -I/opt${_IU})

all: $(PROGRAM_NAME)

hidapi-libusb.o: hidapi-libusb.c
	$(CC) $(CPPFLAGS) $(CFLAGS) ${_IC} -c $< -o $@

he853: main.o he853.o hidapi-libusb.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lusb-1.0 -lpthread

# I couldn't find udev includes for Synology - let's keep it as reference
he853-static: main.o he853.o hidapi-libusb.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -l:$libusb-1.0.a -ludev -lpthread

install: $(PROGRAM_NAME)
	$(INSTALL_PROGRAM) $+ $(BIN_DIR)/$(PROGRAM_NAME)

uninstall: $(PROGRAM_NAME)
	$(RM) /usr/local/bin/$+

clean:
	$(RM) *.o he853 he853-static
