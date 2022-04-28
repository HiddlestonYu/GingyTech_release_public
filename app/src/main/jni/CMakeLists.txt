# Sets the minimum version of CMake required to build your native library.
# This ensures that a certain set of CMake features is available to
# your build.

cmake_minimum_required(VERSION 3.10.1)

# Specifies a library name, specifies whether the library is STATIC or
# SHARED, and provides relative paths to the source code. You can
# define multiple libraries by adding multiple add_library() commands,
# and CMake builds them for you. When you build your app, Gradle
# automatically packages shared libraries with your APK.

MESSAGE(STATUS "Platform:" ${ANDROID_ABI})

add_library(ISP-lib
            STATIC
            IMPORTED)
set_target_properties( # Specifies the target library.
                       ISP-lib

                       # Specifies the parameter you want to define.
                       PROPERTIES IMPORTED_LOCATION

                       # Provides the path to the library you want to import.
                       ${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_ABI}/libISP.a )
add_library(ALG-lib
            STATIC
            IMPORTED)
set_target_properties( # Specifies the target library.
                       ALG-lib

                       # Specifies the parameter you want to define.
                       PROPERTIES IMPORTED_LOCATION

                       # Provides the path to the library you want to import.
                       ${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_ABI}/libGingyAlgor.a )
add_library(IMG-lib
		STATIC
		IMPORTED)
set_target_properties( # Specifies the target library.
		IMG-lib

		# Specifies the parameter you want to define.
		PROPERTIES IMPORTED_LOCATION

		# Provides the path to the library you want to import.
		${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_ABI}/libgingyimg.a )
add_library( # Specifies the name of the library.
             gfp

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             gfp.cpp
			 )

find_library( # Defines the name of the path variable that stores the
              # location of the NDK library.
              log-lib

              # Specifies the name of the NDK library that
              # CMake needs to locate.
              log )

target_link_libraries(gfp ISP-lib ALG-lib IMG-lib ${log-lib})

# Specifies a path to native header files.
include_directories(. inc/)
