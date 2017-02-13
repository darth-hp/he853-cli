all: he853

hid-raw.o: hid-raw.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

he853: main.o he853.o hid-raw.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -o $@ -ludev

clean:
	$(RM) *.o he853
