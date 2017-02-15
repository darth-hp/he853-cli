# About

This project provides a library and CLI programm for the [HomeEasy now Smartwares HE853 USB RF remote control sender](http://service.smartwares.eu/en-us/product/10.036.05/he853-he-comp-usb-netwerk-dongle.aspx).
The sender is used for toggling 433MHz RF controlled devices.

USB stick VID:PID identity and name: 04d9:1357 Holtek Semiconductor, Inc.

The library was extracted from [roseasy](http://ros.org/wiki/roseasy)
which is a part of the [ROS (Robot Operating System)](http://www.ros.org/wiki/) project.

Another project that provides a C# Windows SDK for the dongle is
the [HE853 Control Project](http://he853control.sourceforge.net/).

## Usage

You can build the CLI programm by simply running

  `make`

  To install the binary and the udev rule file to provide access for the users group run

  `sudo make install`

The default protocol is `EU` - you can adjust this in the `main.cpp`
or pass it in over the command line - see parameters below.

```ShellSession
Usage: ./he853 [<DeviceID> <Command> [<Protocol>]]
  DeviceID - ID of the device to act on
  Command  - 0=OFF, 1=ON
    NOTE: AnBan has also values > 1
  Protocol - A=AnBan, U=UK, E=EU, K=KAKU, N=KAKUNEW, L=ALL
    Default protocol is 'E'
    NOTE: Protocol ALL is meant for tests and sends out with all protocols!
  Without parameters the device status will be shown
```

NOTE: You have to have proper access rigths (root) to the USB stick to use it
but the program will tell you if it can't properly access the stick.
Either you can workaround this using `sudo` or by installing the udev rules file.
Good documentation about udev usage and debug is [here](https://wiki.archlinux.org/index.php/udev)

The command is either a *0* for *OFF* or anything > 0 for *ON*.
To program the power socket you have to place the power socket into learning
mode and send the *ON* command to it:

  `./he853 2001 1`

After that you can use the deviceId *2001* for toggling the power socket.

### Requirements

* libusb-1.0 development headers
* udev development headers sometimes required for static builds, sometimes not :)

### Compile Notes

  `he853.h` allows setting of RUN_DRY and DEBUG

### PHP Example

  `<?php exec("/opt/bin/he853 2001 1"); ?>`

### Known Issues

* doesn't compile on OSX
* Compiling under Optware-ng works out of the box after installing
all required dependencies, for Entware-ng you need to install
[include files](https://github.com/Entware-ng/Entware-ng/wiki/Using-gcc-(native-compilation))
manually but they recommend to crosscompile.
Native build required some adjustments for the Makefile:

```Makefile
all: he853

hid-libusb.o: hid-libusb.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -Wl,-rpath,"/opt/lib" -I/opt/include/libusb-1.0 -c $< -o $@

he853: main.o he853.o hid-libusb.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $+ -Wl,-dynamic-linker,/opt/lib/ld-linux.so.3 -Wl,-rpath,"/opt/lib" -o $@ -lusb-1.0 -lpthread

clean:
	$(RM) *.o he853
```
