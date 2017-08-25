find_package(ImageMagick COMPONENTS Magick++)

include_directories(${ImageMagick_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR}/Srcs/common)
include_directories(${PROJECT_SOURCE_DIR}/Srcs/reader)

set(HEIFTOJPEG_SRCS
    heiftojpeg.cpp)
add_executable(${HEIFTOJPEG_EXE} ${HEIFTOJPEG_SRCS})
target_link_libraries(${HEIFTOJPEG_EXE} heifreader)
target_link_libraries(${HEIFTOJPEG_EXE} ${ImageMagick_LIBRARIES})


