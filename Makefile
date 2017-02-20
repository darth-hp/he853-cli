PREFIX = /usr/local
PROGRAM_NAME = he853

BIN_DIR := $(PREFIX)/bin
INSTALL = install
RM      = rm -f
MKDIR   = mkdir -p

COMPILER_OPTIONS = -Wall -Os -s -lrt
CFLAGS           := $(COMPILER_OPTIONS)
CXXFLAGS         := $(COMPILER_OPTIONS)

INSTALL_PROGRAM := $(INSTALL) -c -m 0755
INSTALL_DATA    := $(INSTALL) -c -m 0644

# Some systems require udev, some not for building a static version
# udev is not provided on Synology - building a static one with libusb avoids tinkering with not provided libs
MY_LIBUDEV := $(shell pkg-config --libs --silence-errors libudev)
MY_UDEV_RULES = /lib/udev/rules.d

MY_LIBUSB        = libusb-1.0
MY_LIBUSB_LIBS   := $(shell pkg-config --libs --silence-errors $(MY_LIBUSB))
MY_LIBUSB_CFLAGS := $(shell pkg-config --cflags --silence-errors $(MY_LIBUSB))

MY_INC_LIBS      = -lpthread

all: $(PROGRAM_NAME)

# Doesn't work without this
testusblib:
	$(info MY_LIBUSB_LIBS $(flavor MY_LIBUSB_LIBS))
ifndef MY_LIBUSB_LIBS
	$(error ERROR: Failed to find $(MY_LIBUSB)-dev files - please install first or provide PKG_CONFIG_PATH)
endif

hidapi-libusb.o: hidapi-libusb.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(MY_LIBUSB_CFLAGS) -c $< -o $@

# Shared builds work with libusb and udev dev files, else try static
he853: main.o $(PROGRAM_NAME).o hidapi-libusb.o
	$(testusblib)
ifdef MY_LIBUDEV
	$(info NOTE: Building a shared version)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ $(MY_LIBUSB_LIBS) $(MY_INC_LIBS)
else
	$(info NOTE: Building a static version due to missing udev-dev files)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -Wl,-Bstatic $(MY_LIBUSB_LIBS) -Wl,-Bdynamic $(MY_INC_LIBS)
endif

# you can still try to force building a shared version
he853-shared: main.o $(PROGRAM_NAME).o hidapi-libusb.o
	$(testusblib)
ifndef MY_LIBUDEV
	$(info NOTE: No udev-dev files found - trying to build without ...)
endif
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ $(MY_LIBUSB_LIBS) $(MY_LIBUDEV) $(MY_INC_LIBS)

# you can still try to force building a static version
he853-static: main.o $(PROGRAM_NAME).o hidapi-libusb.o
	$(testusblib)
ifndef MY_LIBUDEV
	$(info NOTE: No udev-dev files found - but still trying to build without ...)
endif
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -Wl,-Bstatic $(MY_LIBUSB_LIBS) -Wl,-Bdynamic $(MY_LIBUDEV) $(MY_INC_LIBS)

installdirs:
	test -d $(BIN_DIR) || $(MKDIR) $(BIN_DIR)

install: $(PROGRAM_NAME) installdirs
	$(INSTALL_PROGRAM) $(PROGRAM_NAME) $(BIN_DIR)/$(PROGRAM_NAME)
	$(INSTALL_DATA) 80-$(PROGRAM_NAME).rules $(MY_UDEV_RULES)/

uninstall: $(PROGRAM_NAME)
	$(RM) $(BIN_DIR)/$(PROGRAM_NAME)
	$(RM) $(MY_UDEV_RULES)/80-$(PROGRAM_NAME).rules

clean:
	$(RM) *.o $(PROGRAM_NAME) $(PROGRAM_NAME)-static $(PROGRAM_NAME)-shared

.PHONY: all he853-static he853-shared install uninstall clean
