find_package(ImageMagick COMPONENTS Magick++)

include_directories(${ImageMagick_INCLUDE_DIRS})

include(CheckIncludeFiles)
check_include_files(libavcodec/avcodec.h HAVE_AVCODEC_H)
check_include_files(libavutil/avformat.h HAVE_AVFORMAT_H)
check_include_files(libavutil/avutil.h HAVE_AVUTIL_H)

include_directories(${PROJECT_SOURCE_DIR}/Srcs/common)
include_directories(${PROJECT_SOURCE_DIR}/Srcs/reader)

set(HEIFTOJPEG_SRCS
    heiftojpeg.cpp)

add_executable(${HEIFTOJPEG_EXE} ${HEIFTOJPEG_SRCS})
target_link_libraries(${HEIFTOJPEG_EXE} heifreader)
target_link_libraries(${HEIFTOJPEG_EXE} ${ImageMagick_LIBRARIES})
target_compile_options(${HEIFTOJPEG_EXE} PRIVATE -DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16)
target_link_libraries(${HEIFTOJPEG_EXE} avcodec)
target_link_libraries(${HEIFTOJPEG_EXE} avformat)
target_link_libraries(${HEIFTOJPEG_EXE} avutil)

