Lightpack project with Prismatik flavour
---------
*Modified version which includes various improvements for Windows, esp. a Desktop Duplication API Grabber*

**Table of content:** <br />
&nbsp;&nbsp;[Short description] (#lightpack-project-with-prismatik-flavour) <br />
&nbsp;&nbsp;[Useful URLs] (#useful-urls) <br />
&nbsp;&nbsp;[Build Prismatik with Windows] (#prismatik-build-instructions-for-windows) <br />
&nbsp;&nbsp;[Build with Linux] (#build-instructions-for-linux) <br />
&nbsp;&nbsp;[Build with OS X] (#build-instructions-for-os-x) <br />
&nbsp;&nbsp;[Build firmware] (#firmware-build-instructions) <br />


**Lightpack** is an fully open-source and simple hardware implementation of the backlight for any computer. It's USB content-driving ambient lighting system.

**Prismatik** is an open-source software we buid to control Lightpack device. It grabs screen, analize picture,
calculate resulting colors and provide soft and gentle lighting with Lightpack device. Moreother, you can
handle another devices with Prismatik such as Adalight, Ardulight or even Alienware LightFX system.

#####Main features:
* Fully open-source under GPLv3 (hardware, software, firmware)
* Cross-platform GUI (Qt)
* USB HID (no need to install any drivers)
* The device is simple to build (just Do-It-Yourself)

#####Useful URLs:
* [Project mothership] (https://github.com/psieg/Lightpack/)
* [Original project mothership] (https://github.com/woodenshark/Lightpack/)
* [Binary downloads] (https://github.com/psieg/Lightpack/releases)
* Wiki with DIY and documentation [ENG] (http://code.google.com/p/light-pack/w/list) / [RUS] (http://code.google.com/p/lightpack/w/list)
* [Post new issue] (https://github.com/psieg/Lightpack/issues)
* [Team] (https://github.com/psieg/Lightpack/graphs/contributors)

---

###Prismatik build instructions for Windows
####Prerequisites:
* [Qt SDK](http://qt-project.org/downloads), you may need to set `%QTDIR%`
* [Windows SDK](https://msdn.microsoft.com/en-us/windows/desktop/ff851942.aspx) or [Microsoft DirectX SDK](http://www.microsoft.com/en-us/download/details.aspx?id=6812)
* POSIX shell utilities [MSYS for example](http://www.mingw.org/wiki/MSYS). It may help to have the `PATH` environment variable set for the utilities (Run &rarr; sysdm.cpl &rarr; Advanced &rarr; Environment Variable &rarr; Edit `PATH` system variable (`C:\MinGW\msys\1.0\bin;` for example)

####Build process:
1. go to `<repo>/Software`
2. copy and edit `build-vars.prf` accoring to your machine
2. optional, if locales changed: run `update_locales.bat` or `./update_locales.sh` (slow on Windows)
3. run `scripts/win32/generate_sln.sh`
4. build `Lightpack.sln` with MSBuild / VisualStudio

####Building an installer:
1. run `scripts/win32/prepare_installer.sh`
2. build `dist_windows/script.iss` (64bit) or `script32.iss` (32bit) with ISCC (the InnoSetup compiler)

---

###Build instructions for Linux
####Prerequisites:
You will need the following packages, usually all of them are in distro's repository:
* qt5-default
* libqt5serialport5-dev
* build-essential
* libgtk2.0-dev
* libusb-1.0-0-dev
* libnotify-dev
* libudev-dev
* qttools5-dev-tools
* if you are using Ubuntu: libappindicator-dev

####Build process:
1. go to `<repo>/Software`
2. optional, if locales changed: run `./update_locales.sh`
3. run `qmake -r`
4. run `make`

####Building a deb package:
1. run `scripts/linux/prepare_installer.sh`
2. `cd dist_linux` and run `build-deb.sh`

####Manual deployment:
Instead of building a deb package, you can:

1. Add a rule for **UDEV**. See comments from `<repo>/Software/dist_linux/deb/etc/udev/rules.d/93-lightpack.rules` for how to do it.
2. Make sure `<repo>/Software/qtserialport/libQt5SerialPort.so.5` is available for loading by *Prismatik* (place it in appropriate dir or use *LD_LIBRARY_PATH* variable)

---

###Build instructions for OS X
####Prerequisites:
* Qt SDK (5.0+)
* MacOSX 10.9.sdk

###### Whole dependencies list for Prismatik 5.10.1:
* QtCore.framework
* QtGui.framework
* QtNetwork.framework
* QtOpenGL.framework

####Build process:
1. Download and unpack 5.0+ **Qt SDK** from www.qt-project.org
4. Build **Prismatik** project

to run Prismatik please make sure PythonQt libs are available for load at runtime

---

###Firmware build instructions

*Please note that these instructions are for Debian based systems.*

**Compiling firmware only:**
1. Install [AVR GCC Toolchain](http://avr-eclipse.sourceforge.net/wiki/index.php/The_AVR_GCC_Toolchain): `sudo apt-get install gcc-avr binutils-avr gdb-avr avr-libc avrdude`
2. Compile the firmware:
  * `cd Firmware`
  * `make LIGHTPACK_HW=7` (or any other hardware version 4-7)
  * Alternatively, you can do `./build_batch.sh` to build the firmware for all hardware versions
3. The firmware can be found in the same directory (individual build) or *Firmware/hex* (batch build).

**Compiling and uploading firmware to device:**
1. Install [AVR GCC Toolchain](http://avr-eclipse.sourceforge.net/wiki/index.php/The_AVR_GCC_Toolchain) and **dfu-programmer**: `sudo apt-get install gcc-avr binutils-avr gdb-avr avr-libc avrdude dfu-programmer`
2. Reboot device to bootloader (via the secret button on the device)
3. Compile and upload the firmware:
  * `cd Firmware`
  * `make LIGHTPACK_HW=7 && make dfu LIGHTPACK_HW=7` (or any other hardware version 4-7)

---

Please let us know if you find mistakes, bugs or errors. Contributions are welcome.<br />
Post new issue : https://github.com/psieg/Lightpack/issues
