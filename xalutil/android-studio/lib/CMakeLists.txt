cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

project(xalutil)

include("../../../../hltypes/android-studio/generic/CMakeLists.txt")

file(
	GLOB_RECURSE CppSrc
	"../../src/*.c"
	"../../src/*.cpp"
)

add_library(xalutil STATIC ${CppSrc})

include_directories(AFTER "../../../../hltypes/include")
include_directories(AFTER "../../../include")
include_directories(AFTER "../../include/xalutil")
include_directories(AFTER "../../src")

add_definitions(
	-DXALUTIL_EXPORTS
)

target_link_libraries(
	xalutil
)
