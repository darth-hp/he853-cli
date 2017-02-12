all: he853

he853: main.o he853.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -lhidapi-libusb -lusb-1.0 -lpthread

clean:
	$(RM) *.o he853
