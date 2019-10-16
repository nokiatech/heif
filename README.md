# Multi-Image Application Format (MIAF)
MIAF (Multi-Image Application Format) standard defines a set of additional constraints on HEIF specification, which further enables interoperability. MIAF is an MPEG standard ([ISO/IEC 23000-22](https://www.iso.org/standard/74417.html)).
HEIF is a visual media container format standardized by the Moving Picture Experts Group (MPEG) for storage and sharing of images and image sequences. It is based on the well-known ISO Base Media File Format (ISOBMFF) standard. HEIF Reader/Writer Engine is an implementation of HEIF standard in order to demonstrate its powerful features and capabilities.

This is a MIAF-specific branch. For HEIF implementation, please switch to [this branch](https://github.com/nokiatech/heif).

Please follow this **[link](https://nokiatech.github.io/heif)** to access HEIF Web-Site.
You can also check the **[technical information](https://nokiatech.github.io/heif/technical.html)** pages for more detail.

## News:
[11.07.2019] MIAF features: The HEIF writer checks additional constraints from files which contain 'miaf' brand in compatible brands of FileTypeBox. A MIAF gallery application for Android is included, with an extension to Java API to help with content selection.

## Features:
HEIF is a media container format. It is not an image or video encoder per se. Hence, the quality of the visual media depends highly on the proper usage of visual media encoder (e.g. HEVC). Current standard allows containing HEVC/AVC/JPEG encoded bitstreams. This can be easily extended to future visual media codecs. It has many powerful features which are currently not present in other image file formats. MIAF further enhances HEIF interoperability. Some of MIAF features are:
* Additional constraints to HEIF
* Application brands (e.g. Progressive application brand, Burst capture application brand)
* Coding profile brands
* Specific metadata formats
* Rules for extending MIAF format

## Contents of the Repository:
This repository contains the following items:
* MIAF checker add-on for writer (under srcs/api/writer/)
* MIAF player example (under srcs/android-miaf-gallery)
* ISO Base Media File Format (ISOBMFF) box parse/write source code (under srcs/common/)
* HEIF Reader API and Library (under srcs/api/reader/)
* HEIF Writer API and Library (under srcs/api/writer/)
* HEIF Reader/Writer Java API (under srcs/api-java/)
* HEIF Reader/Writer convenience C++ API (under srcs/api-cpp/)
* HEIF Reader API Usage Example Code (under srcs/examples/)
* HEIF Source Code Documentation (**[Doxygen](http://www.stack.nl/~dimitri/doxygen/)** generator under docs/)
* HEIF Web Site Content (in **[gh-pages branch](https://github.com/nokiatech/heif/tree/gh-pages)**)
* HEIF Reader JavaScript Implementation (in **[gh-pages branch](https://github.com/nokiatech/heif/tree/gh-pages)**)

## Building source:
Prerequisites: **[cmake](https://cmake.org/)** and compiler supporting C++11 in PATH.
```
cd heif/build
cmake --help
cmake ../srcs -G"<Generator listed by above command for your target platform>"
cmake --build .
```

## Building Java API for Windows or Linux
Prerequisites: Java version 8 or newer, Gradle.

First build the C/C++ library as described above.

After that
```
cd heif/build/java-desktop
gradle build
```
Note that in order to run the Java API you need to have the HEIF JNI library built in the earlier step (heifjni.dll or heifjni.so) in the Java library search path.

## Building Java API for Android:
Prerequisites: Android SDK & NDK
Import the project files under heif/build/android into Android Studio and build the library

## Building Android MIAF gallery
Import the project files under heif/build/android into Android Studio and build the app. Alternatively, you can build it from command line. E.g. in Linux:

```
cd heif/build/android-miaf
./gradlew build
```

See **[wiki page](https://github.com/nokiatech/heif/wiki/I.-How-to-build-HEIF-Source-Code)** for more information and platform specific instructions.

## License:
Please see **[LICENSE.TXT](https://github.com/nokiatech/heif/blob/master/LICENSE.TXT)** file for the terms of use of the contents of this repository.

All the example media files (*.heic, *.png, *.jpg, *.gif) in this repository are under copyright © Nokia Technologies 2015-2018.

For more information/questions/source code/commercial licensing related issues, please contact: <heif@nokia.com>

### **Copyright (c) 2015-2019, Nokia Technologies Ltd.**
### **All rights reserved.**

