cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

project(demo_test)

include("../../../../hltypes/android-studio/generic/cmake-preprocessors.txt")

file(
	GLOB_RECURSE CppSrc
	"../../../demos/demo_test/*.cpp"
)

add_library(demo_test SHARED ${CppSrc})

include_directories(AFTER "../../../include")
include_directories(AFTER "../../../../hltypes/include")

add_subdirectory("../../lib" xal)
add_subdirectory("../../../../hltypes/android-studio/lib" hltypes)
add_subdirectory("../../../lib/tremor/android-studio/lib" tremor)
add_subdirectory("../../../lib/ogg/android-studio/lib" ogg)

target_link_libraries(
	demo_test
	xal
	hltypes
	tremor
	ogg
	${liblog}
	${libz}
	${libc}
	${libdl}
	${libandroid}
	${libGLESv2}
	${libOpenSLES}
)
