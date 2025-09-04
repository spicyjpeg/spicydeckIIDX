
cmake_minimum_required(VERSION 3.30)

cmake_path(SET idfPath NORMALIZE "$ENV{IDF_PATH}")
if(NOT IS_DIRECTORY "${idfPath}")
	message(
		FATAL_ERROR
		"The IDF_PATH environment variable must be set to a valid directory "
		"prior to building the project. Use the export script included with "
		"ESP-IDF to set it in your current terminal session."
	)
endif()

include("${idfPath}/tools/cmake/project.cmake")

# Set up ESP-IDF. The list of components enabled by default is shrunk in order
# to minimize code bloat.
set(COMPONENTS main)
set(
	COMPONENT_DIRS
	"${idfPath}/components"
	src
)

# Tell CMake about the project. This will also trigger ESP-IDF's build scripts
# to run the CMake script in src/main.
# NOTE: due to a faulty regex, some versions of the ESP-IDF VS Code extension
# will refuse to load the folder if the project() call is not fully contained in
# a single line. A temporary variable is used here as a workaround.
set(
	projectArgs
	LANGUAGES    C CXX ASM
	VERSION      0.1.0
	DESCRIPTION  "Minimal, self-contained, fully standalone DJ mixing console"
	HOMEPAGE_URL "https://github.com/spicyjpeg/spicydeckIIDX"
)
project(spicydeckIIDX ${projectArgs})
