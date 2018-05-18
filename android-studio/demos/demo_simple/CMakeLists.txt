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
add_subdirectory("../../../../april/android-studio/lib" april)
add_subdirectory("../../../../hltypes/android-studio/lib" hltypes)
add_subdirectory("../../../../gtypes/android-studio/lib" gtypes)
add_subdirectory("../../../lib/tremor/android-studio/lib" tremor)
add_subdirectory("../../../lib/ogg/android-studio/lib" ogg)
add_subdirectory("../../../../april/lib/png/android-studio/lib" png)
add_subdirectory("../../../../april/lib/jpeg/android-studio/lib" jpeg)

target_link_libraries(
	demo_simple
	april
	xal
	hltypes
	gtypes
	tremor
	ogg
	jpeg
	png
	${liblog}
	${libz}
	${libc}
	${libdl}
	${libandroid}
	${libGLESv2}
	${libOpenSLES}
)
