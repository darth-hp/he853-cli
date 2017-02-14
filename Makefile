PREFIX = /usr/local
PROGRAM_NAME = he853

BIN_DIR = $(PREFIX)/bin

INSTALL = install
RM      = rm -f
MKDIR   = mkdir -p

CC = gcc
COMPILER_OPTIONS = -Wall -Os -s
CFLAGS           = $(COMPILER_OPTIONS)
CXXFLAGS         = $(COMPILER_OPTIONS)

INSTALL_PROGRAM = $(INSTALL) -c -m 0755
INSTALL_DATA    = $(INSTALL) -c -m 0644

_IU=/include/libusb-1.0
_IC=$(shell pkg-config --silence-errors --cflags libusb-1.0 || [ -d /usr${_IU} ] && echo -I/usr${_IU} || echo -I/opt${_IU})

all: $(PROGRAM_NAME)

hidapi-libusb.o: hidapi-libusb.c
	$(CC) $(CPPFLAGS) $(CFLAGS) ${_IC} -c $< -o $@

he853: main.o $(PROGRAM_NAME).o hidapi-libusb.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lusb-1.0 -lpthread

# I couldn't find udev includes for Synology to build it static
# Let's keep it as reference for working platforms
he853-static: main.o $(PROGRAM_NAME).o hidapi-libusb.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -l:$libusb-1.0.a -ludev -lpthread

installdirs:
	test -d $(BIN_DIR) || $(MKDIR) $(BIN_DIR)

install: $(PROGRAM_NAME) installdirs
	$(INSTALL_PROGRAM)	fdupes   $(BIN_DIR)/$(PROGRAM_NAME)

clean:
	$(RM) *.o $(PROGRAM_NAME) $(PROGRAM_NAME)-static
