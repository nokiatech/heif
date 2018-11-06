# High Efficiency Image File Format (HEIF)
HEIF is a visual media container format standardized by the Moving Picture Experts Group (MPEG) for storage and sharing of images and image sequences. It is based on the well-known ISO Base Media File Format (ISOBMFF) standard. HEIF Reader/Writer Engine is an implementation of HEIF standard in order to demonstrate its powerful features and capabilities.

Please follow this **[link](https://nokiatech.github.io/heif)** to access HEIF Web-Site.
You can also check the **[Wiki](https://github.com/nokiatech/heif/wiki)** pages for more information.

## News:
[06.11.2018] v3.4.0 Release: Java API improvements. Bug fixes, code cleanup and robustness improvements.

[13.06.2018] v3.3.0 Release: Java desktop build added, Java API includes track and image sequence as well as entity grouping support.

[06.04.2018] v3.2 Release. Two new APIs added, a Java API which wraps the C++ libraries using JNI and a convenience C++ API (which is used by the Java API) that wraps the underlying reader and writer. The Java API is expected to remain relatively stable, however the new C++ API will most likely change in the future.

[14.03.2018] v3.1 Release. Reader API update, bug fixes and examples update.

[05.03.2018] Code updated and tagged v3.0. Writer executable replaced with writer library / API. Implementation is based on HEIF standard specification [ISO/IEC 23008-12:2017](http://standards.iso.org/ittf/PubliclyAvailableStandards/c066067_ISO_IEC_23008-12_2017.zip) that is available from iso.org web pages.

[09.03.2017] ISO/IEC 23008-12 second edition includes support for the interchange of multi-layered images. The source code now includes structures that are specified in this second edition. Simple support for AVC is also added. Minor bug fixes are included in this update. Example configuration files for generating multi-layered streams can be found in the Wiki.

[24.02.2016] HEIF source code and website is updated to reflect the latest HEIF specification changes and various fixes. Please note that backwards compatibility is not maintained during this update.

[24.02.2016] HEIF conformance test candidate files can be found **[here](https://github.com/nokiatech/heif_conformance)**

## Features:
HEIF is a media container format. It is not an image or video encoder per se. Hence, the quality of the visual media depends highly on the proper usage of visual media encoder (e.g. HEVC). Current standard allows containing HEVC/AVC/JPEG encoded bitstreams. This can be easily extended to future visual media codecs. It has many powerful features which are currently not present in other image file formats. Some of these features are:
* Encapsulate images coded using HEVC/SHVC/MV-HEVC/AVC/JPEG.
* Encapsulate image sequences coded using HEVC/SHVC/MV-HEVC/AVC.
* Storage based on widely adopted ISO Base Media File Format (ISOBMFF)
* Supports efficient storage of image bursts and cinemagraphs
* Supports computational photography use cases
* Supports both lossy and lossless image data storage
* A better and easy way to distribute still images, image collections and related metadata.

Please follow this **[link](https://nokiatech.github.io/heif/examples.html)** to see HEIF file examples.

## Contents of the Repository:
This repository contains the following items:
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
Prerequisites: Java version 8 or newer, Gradle
First build the C/C++ library as described above
After that
```
cd heif/build/java-desktop
gradle build
```
Note that in order to run the Java API you need to have the HEIF JNI library built in the earlier step (heifjni.dll or heifjni.so) in the Java library search path.

## Building Java API for Android:
Prerequisites: Android SDK & NDK
Import the project files under heif/build/android into Android Studio and build the library

See **[wiki page](https://github.com/nokiatech/heif/wiki/I.-How-to-build-HEIF-Source-Code)** for more information and platform specific instructions.

## License:
Please see **[LICENSE.TXT](https://github.com/nokiatech/heif/blob/master/LICENSE.TXT)** file for the terms of use of the contents of this repository.

All the example media files (*.heic, *.png, *.jpg, *.gif) in this repository are under copyright © Nokia Technologies 2015-2018.

For more information/questions/source code/commercial licensing related issues, please contact: <heif@nokia.com>

### **Copyright (c) 2015-2018, Nokia Technologies Ltd.**
### **All rights reserved.**

