BUILT = he853

# optimize size and strip symbol tables
CXXFLAGS = -Os -s

all: $(BUILT)

hidapi-libusb.o: hidapi-libusb.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(shell pkg-config --cflags libusb-1.0) -c $< -o $@

he853: main.o he853.o hidapi-libusb.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ $(shell pkg-config --libs libusb-1.0) -lpthread

# I couldn't find udev includes for Synology - let's keep it as reference
he853-static: main.o he853.o hidapi-libusb.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -l:libusb-1.0.a -ludev -lpthread

install: $(BUILT)

uninstall: $(BUILT)

clean:
	$(RM) *.o he853 he853-static
