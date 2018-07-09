cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

project(xal)

include("../../../hltypes/android-studio/generic/CMakeLists.txt")

file(
	GLOB_RECURSE CppSrc
	"../../src/*.c"
	"../../src/*.cpp"
)

add_library(xal STATIC ${CppSrc})

include_directories(AFTER "../../include/xal")
include_directories(AFTER "../../src/audiosystems/OpenSLES")
include_directories(AFTER "../../src/audiosystems/NoAudio")
include_directories(AFTER "../../src/audiosources")
include_directories(AFTER "../../../hltypes/include")
include_directories(AFTER "../../lib/vorbis/include")
include_directories(AFTER "../../lib/ogg/include")

add_definitions(
	-DXAL_EXPORTS
	-D_FORMAT_FLAC
	-D_FORMAT_OGG
	-D_FORMAT_WAV
	-D_OPENSLES
)
