# Copyright (c) 2015, Nokia Technologies Ltd.
# All rights reserved.
#
# Licensed under the Nokia High-Efficiency Image File Format (HEIF) License (the "License").
#
# You may not use the High-Efficiency Image File Format except in compliance with the License.
# The License accompanies the software and can be found in the file "LICENSE.TXT".
#
# You may also obtain the License at:
# https://nokiatech.github.io/heif/license.txt

include_directories(${PROJECT_SOURCE_DIR}/Srcs/common)
include_directories(${PROJECT_SOURCE_DIR}/Srcs/jsonlib)
include_directories(${PROJECT_SOURCE_DIR}/Srcs/writer)
include_directories("${PROJECT_BINARY_DIR}")

set(WRITERAPP_SRCS
	main.cpp
	writerapp.cpp
	writercfg.cpp)

add_executable(${WRITER_EXE} ${WRITERAPP_SRCS})
target_link_libraries(${WRITER_EXE} json heifwriter)
