# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# include headers
include_directories(
        ${PROJECT_SOURCE_DIR}
        include/ffmpeg
        utils/
        decoder/
        player/
)

# copy source file path
aux_source_directory(${PROJECT_SOURCE_DIR} FFMPEG_ENGINE)
aux_source_directory(${PROJECT_SOURCE_DIR}/decoder/ DECODER)
aux_source_directory(${PROJECT_SOURCE_DIR}/player/ PLAYER)


# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp
        ${FFMPEG_ENGINE}
        ${DECODER}
        ${PLAYER})

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# define ffmpeg lib's path and name
set(MY_FFMPEG_LIB_PATH ${CMAKE_SOURCE_DIR}/../jniLibs)
set(MY_FFMPEG_LIB_NAME libffmpeg.so)


# add ffmpeg lib
add_library(
        ffmpeg
        SHARED
        IMPORTED)
set_target_properties(
        ffmpeg
        PROPERTIES IMPORTED_LOCATION
        ${MY_FFMPEG_LIB_PATH}/${CMAKE_ANDROID_ARCH_ABI}/${MY_FFMPEG_LIB_NAME}
)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        native-lib

        ffmpeg
        android
        OpenSLES
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})