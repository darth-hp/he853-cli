PREFIX = /usr/local
PROGRAM_NAME = he853

BIN_DIR = $(PREFIX)/bin

INSTALL = install
RM      = rm -f
MKDIR   = mkdir -p

COMPILER_OPTIONS = -Wall -Os -s
CFLAGS           = $(COMPILER_OPTIONS)
CXXFLAGS         = $(COMPILER_OPTIONS)

INSTALL_PROGRAM = $(INSTALL) -c -m 0755
INSTALL_DATA    = $(INSTALL) -c -m 0644

# Some systems require udev, some not for building a static version
# udev is not provided on Synology - building a static one with libusb avoids tinkering with not provided libs
LIBUDEV = $(shell pkg-config --libs libudev)

all: $(PROGRAM_NAME)

hidapi-libusb.o: hidapi-libusb.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(shell pkg-config --cflags libusb-1.0) -c $< -o $@

he853: main.o $(PROGRAM_NAME).o hidapi-libusb.o
ifdef LIBUDEV
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lusb-1.0 -lpthread
else
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -Wl,-Bstatic $(shell pkg-config --libs libusb-1.0) -Wl,-Bdynamic -lpthread
endif

# you can still try to force building a static version
he853-static: main.o $(PROGRAM_NAME).o hidapi-libusb.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -Wl,-Bstatic $(shell pkg-config --libs libusb-1.0) -Wl,-Bdynamic $(LIBUDEV) -lpthread

installdirs:
	test -d $(BIN_DIR) || $(MKDIR) $(BIN_DIR)

install: $(PROGRAM_NAME) installdirs
	$(INSTALL_PROGRAM) $(PROGRAM_NAME) $(BIN_DIR)/$(PROGRAM_NAME)

clean:
	$(RM) *.o $(PROGRAM_NAME) $(PROGRAM_NAME)-static
