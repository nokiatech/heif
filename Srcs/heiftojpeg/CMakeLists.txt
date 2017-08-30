find_package(ImageMagick COMPONENTS Magick++ MagickWand MagickCore montage)

include_directories(${ImageMagick_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR}/Srcs/common)
include_directories(${PROJECT_SOURCE_DIR}/Srcs/reader)

set(HEIFTOJPEG_SRCS
    heiftojpeg.cpp)

add_executable(${HEIFTOJPEG_EXE} ${HEIFTOJPEG_SRCS})
target_link_libraries(${HEIFTOJPEG_EXE} heifreader)
target_link_libraries(${HEIFTOJPEG_EXE} ${ImageMagick_LIBRARIES})
target_compile_options(${HEIFTOJPEG_EXE} PRIVATE -DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16)
