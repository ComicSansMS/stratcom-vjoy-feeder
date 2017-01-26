# Stratcom vJoy-Feeder #

Use your [Microsoft SideWinder Strategic Commander](https://en.wikipedia.org/wiki/Microsoft_SideWinder#Strategic_Commander) as a DirectInput joystick.

This application acts as a feeder for the [vJoy](http://vjoystick.sourceforge.net/site/) virtual Joystick device driver. This allows you to use your Strategic Commander with any game that supports gamepads or joysticks via DirectInput.

## Requirements ##

This software is currently 64 bit Windows only. It works on all Windows versions from Windows XP and up, including Windows 7, Windows 8 and Windows 10. 

## Installation ##

1. First download and install [vJoy](http://vjoystick.sourceforge.net/site/index.php/download-a-install/72-download).
2. Launch 'Configure vJoy' from the Start Menu. If you cannot find the entry in the Start Menu, the program should be located in `C:\Program Files\vJoy\x64\vJoyConf.exe`.
3. Add at least one Joystick with the 3 axes X, Y, and Z, and at least 32 buttons. To make use of all features, also add two additional devices with 32 buttons each. Check the `USB Game Controllers` dialog in the Windows Control Panel to ensure the devices have been added.
4. Connect your Strategic commander via USB and launch the program. Any input on the Strategic Commander will now be fed to the vJoy joystick.

## Build instructions ##

To build Stratcom vJoy feeder from source you will need:

- [Visual Studio 2015](https://www.visualstudio.com/en-us/products/vs-2015-product-editions.aspx)
- [CMake](https://cmake.org/download/)
- [Boost](http://www.boost.org/)
- [Qt](http://www.qt.io/developers/) 5.7.0 with the Windows 64 binaries for Visual Studio 2015 ([Direct link](http://download.qt.io/official_releases/qt/5.7/5.7.0/qt-opensource-windows-x86-msvc2015_64-5.7.0.exe))
- [libstratcom](https://github.com/ComicSansMS/libstratcom) 1.1.0 ([Direct Link](https://github.com/ComicSansMS/libstratcom/releases/download/v1.1.0/libstratcom-v1.1.0-win64.zip))
- [vJoy](http://vjoystick.sourceforge.net/site/) Feeder SDK ([Direct Link](http://vjoystick.sourceforge.net/site/index.php/component/weblinks/weblink/13-uncategorised/11-redirect-vjoy2sdk?task=weblink.go))

To build the application:

- Check out the source code
- Extract the vJoy SDK to a folder `external/vJoy` under the source root folder.
- Point the `QT5_ROOT` environment variable to the root of your Qt5 installation.
- Run CMake to generate the VS2015 solution files.
- Point the `LIBSTRATCOM_PREFIX_PATH` CMake variable to the location where you extracted the libstratcom binaries.
- Build the Visual Studio solution.  
