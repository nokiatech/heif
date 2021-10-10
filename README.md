# Carriage of VVC in ISOBMF

This branch (VVC_MP4) extends the Nokia HEIF reader/writer engine to be able to handle MP4 files with several features related to Carriage of VVC in ISOBMF:

* VVC configuration box ('vvcC')
* VVC NAL unit configuration box (‘vvnC’)
* Track reference ‘subp’ from VVC merge base track to VVC subpicture tracks
* Track reference ‘minp’ from VVC merge base track to VVC subpicture tracks
* Subpicture order sample group ('spor')
* Subpicture ID sample group ('spid')
* Mixed NAL unit type pictures sample group ('minp')
* VVC track ('vvc1')
* VVC subpicture track ('vvc1')
* Track group of VVC subpicture tracks ('alte')
* Track group of VVC subpicture tracks ('snut')

Please note current limitations:

* Files created by the writer are VVC-encoded bitstreams carried in ISOBMFF container
* C++ and Java APIs are only for HEIF files, excluding VVC encoded bitstreams in ISOBMFF container

## News

[11.10.2021] Add initial support for handling MP4 files with VVC-encoded content.

## Contents of the repository

This repository contains the following items:

* ISO Base Media File Format (ISOBMFF) box parse/write source code (under srcs/common/)
* HEIF Reader API and Library (under srcs/api/reader/)
* HEIF Writer API and Library (under srcs/api/writer/)
* HEIF Reader/Writer Java API (under srcs/api-java/)
* HEIF Reader/Writer convenience C++ API (under srcs/api-cpp/)
* HEIF Reader API Usage Example Code (under srcs/examples/)
* HEIF Source Code Documentation (**[Doxygen](http://www.stack.nl/~dimitri/doxygen/)** generator under docs/)

## Building source

At the moment only Linux build of the VVC_MP4 branch is recommended.

Prerequisites: **[cmake](https://cmake.org/)** and compiler supporting C++11 in PATH.

```
git clone https://github.com/nokiatech/heif.git
cd heif
git checkout VVC_MP4
# VVC bitstream parsing is done using VTM reference software for VVC. It must be loaded as a submodule before building the source code.
git submodule init
git submodule update
cd build
cmake --help
cmake ../srcs -G"<Generator listed by above command for your target platform>"
cmake --build .
```

See **[wiki page](https://github.com/nokiatech/heif/wiki/I.-How-to-build-HEIF-Source-Code)** for more information and platform specific instructions.

## License

Please see **[LICENSE.TXT](https://github.com/nokiatech/heif/blob/master/LICENSE.TXT)** file for the terms of use of the contents of this repository.

All the example media files (*.heic, *.png, *.jpg, *.gif) in this repository are under copyright © Nokia Technologies 2015-2021.

For more information/questions/source code/commercial licensing related issues, please contact: <heif@nokia.com>

### **Copyright (c) 2015-2021, Nokia Technologies Ltd.**

### **All rights reserved.**
