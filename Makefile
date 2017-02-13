#HIDAPI-HIDRAW = $(shell pkg-config --libs hidapi-hidraw)

all: he853

hidapi-libusb.o: hidapi-libusb.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(shell pkg-config --cflags libusb-1.0) -c $< -o $@

he853: main.o he853.o hidapi-libusb.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ $(shell pkg-config --libs libusb-1.0) -lpthread

# ifeq ($(HIDAPI-BACKEND),-lhidapi-hidraw)
# 	@echo Compiling with hidraw
# 	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ $(HIDAPI-BACKEND)
# else
# 	@echo Compiling with libusb
# 	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ $(HIDAPI-BACKEND)
# endif


clean:
	$(RM) *.o he853
