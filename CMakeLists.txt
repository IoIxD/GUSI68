cmake_minimum_required(VERSION 3.0)

project(os9fetch LANGUAGES C CXX)

###############################################################################
## file globbing ##############################################################
###############################################################################

# these instructions search the directory tree when CMake is
# invoked and put all files that match the pattern in the variables
# `sources` and `data`

include_directories(include/)
include_directories(src/)
add_compile_definitions(__STD_C)
add_compile_definitions(_hdr_stdarg)
#add_compile_definitions(GUSI_INTERNAL)

#GUSIConfig/**/**/**/*.c GUSIConfig/**/**/**/*.h GUSIConfig/**/**/**/*.cpp GUSIConfig/**/**/**/*.hpp 
#GUSIConfig/**/**/*.c GUSIConfig/**/**/*.h GUSIConfig/**/**/*.cpp GUSIConfig/**/**/*.hpp 
#GUSIConfig/**/*.c GUSIConfig/**/*.h GUSIConfig/**/*.cpp GUSIConfig/**/*.hpp 
#GUSIConfig/*.c GUSIConfig/*.h GUSIConfig/*.cpp GUSIConfig/*.hpp

file(GLOB_RECURSE sources
    examples/*.cpp
    # GUSIConfig/*.h GUSIConfig/*.c GUSIConfig/*.cp GUSIConfig/*.r GUSIConfig/*.rsrc
    src/**/**/**/*.c src/**/**/**/*.h src/**/**/**/*.cpp src/**/**/**/*.hpp 
    src/**/**/*.c src/**/**/*.h src/**/**/*.cpp src/**/**/*.hpp 
    src/**/*.c src/**/*.h src/**/*.cpp src/**/*.hpp 
    src/*.c src/*.h src/*.cpp src/*.hpp
)
file(GLOB_RECURSE data resources/*)
# you can use set(sources src/main.cpp) etc if you don't want to
# use globbing to find files automatically

###############################################################################
## target definitions #########################################################
###############################################################################

# add the data to the target, so it becomes visible in some IDE

if(CMAKE_SYSTEM_NAME MATCHES Retro)
    add_application(os9fetch ${sources} ${data})
else()
    add_executable(os9fetch ${sources} ${data})
endif()
target_link_directories(os9fetch PUBLIC ${CMAKE_SOURCE_DIR}/lib )

set_property(TARGET os9fetch PROPERTY CXX_STANDARD 20)

# just for os9fetch add some compiler flags
target_compile_options(os9fetch PUBLIC -std=c++20 -Wall -Wfloat-conversion)

set_target_properties(os9fetch PROPERTIES COMPILE_OPTIONS -ffunction-sections)
#target_link_libraries(os9fetch "${RETRO68_ROOT}/InterfaceAndLibraries/Libraries/StubLibraries/ThreadsLib")

#target_link_libraries( os9fetch ThreadsLib)

if(PLATFORM MATCHES retro68)
    add_compile_definitions(FOR_68K)
endif()

if(PLATFORM MATCHES retroppc)
    add_compile_definitions(FOR_PPC)
    set_target_properties(os9fetch PROPERTIES COMPILE_FLAGS "-ffunction-sections -mcpu=601 -O3 -Wall -Wextra -Wno-unused-parameter")
    set_target_properties(os9fetch PROPERTIES LINK_FLAGS "-Wl,-gc-sections")
endif()


include_directories( "${RETRO68_TOOLCHAIN}universal/CIncludes/")
message(STATUS "Making for ${CMAKE_SYSTEM_NAME}")
if(CMAKE_SYSTEM_NAME MATCHES Retro68)
    set_target_properties(os9fetch PROPERTIES LINK_FLAGS "-Wl,--mac-strip-macsbug")
endif()

# this copies all resource files in the build directory
# we need this, because we want to work with paths relative to the executable
file(COPY ${data} DESTINATION resources)