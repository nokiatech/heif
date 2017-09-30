# High Efficiency Image File Format (HEIF) 
HEIF is a visual media container format standardized by the Moving Picture Experts Group (MPEG) for storage and sharing of images and image sequences. It is based on the well-known ISO Base Media File Format (ISOBMFF) standard. HEIF Reader/Writer Engine is an implementation of HEIF standard in order to demonstrate its powerful features and capabilities.

Please follow this **[link](https://nokiatech.github.io/heif)** to access HEIF Web-Site. 
You can also check the **[Wiki](https://github.com/nokiatech/heif/wiki)** pages for more information.

## News:
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
* ISO Base Media File Format (ISOBMFF) parser/writer source code
* HEIF Reader API and Library
* HEIF Writer Library and Example Application
* HEIF Reader API Usage Example Code
* HEIF Source Code Documentation 
* HEIF Web Site Content
* HEIF Reader JavaScript Implementation
* heifttojpeg conversion tool (for iOS 11 photos)

## heiftojpeg

This tool is compatible with Linux and Mac environments. You will need ImageMagick, Magick++, and ffmpeg to compile and run it.

To compile:

`cd heif/build && cmake .. && make`

The executable will be in `heif/Bins/heiftojpeg`

You can run it on any HEIF file:

`heiftojpeg [-v] [-s <max_dimension>] <input.heic> <output.jpg>`

e.g.

`heiftojpeg fixtures/test_001.heic out.jpg`


## License:
Please see **[LICENSE.TXT](https://github.com/nokiatech/heif/blob/master/LICENSE.TXT)** file for the terms of use of the contents of this repository.

All the example media files (*.heic, *.png, *.jpg, *.gif) in this repository are under copyright © Nokia Technologies 2015-2017.

For more information/questions/source code related issues, please contact: <heif@nokia.com>

### **Copyright (c) 2015-2017, Nokia Technologies Ltd.**
### **All rights reserved.**
