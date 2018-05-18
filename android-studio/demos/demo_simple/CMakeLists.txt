cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

project(demo_simple)

include("../../../../hltypes/android-studio/generic/cmake-preprocessors.txt")

file(
	GLOB_RECURSE CppSrc
	"../../../demos/demo_simple/*.cpp"
)

add_library(demo_simple SHARED ${CppSrc})

include_directories(AFTER "../../../include")
include_directories(AFTER "../../../../hltypes/include")

add_subdirectory("../../lib" xal)
add_subdirectory("../../../../hltypes/android-studio/lib" hltypes)
add_subdirectory("../../../lib/ogg/android-studio/lib" ogg)
add_subdirectory("../../../lib/tremor/android-studio/lib" tremor)

target_link_libraries(
	demo_simple
	xal
	hltypes
	ogg
	tremor
	${liblog}
	${libz}
	${libc}
	${libdl}
	${libandroid}
	${libGLESv2}
	${libSLES}
)
