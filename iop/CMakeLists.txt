
cmake_minimum_required(VERSION 3.25)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/cmake/toolchain.cmake")

# Can you write a simple assembly implementation of `setjmp()` and `longjmp()` for the RV32E architecture?

project(
	spicydeckIIDX-iop
	LANGUAGES    C CXX ASM
	VERSION      1.0.0
	DESCRIPTION  "IOP firmware for spicydeckIIDX"
	HOMEPAGE_URL "https://github.com/spicyjpeg/spicydeckIIDX"
)

include(cmake/setup.cmake)

# Build a "common" library containing code shared across all examples and link
# it by default into every executable.
add_library(
	common OBJECT
	src/libc/crt0.c
	src/libc/crt0.s
	src/libc/cxxsupport.cpp
	src/libc/malloc.c
	src/libc/misc.c
	src/libc/setjmp.s
	src/libc/string.c
	src/vendor/printf.c
)
target_include_directories(
	common PUBLIC
	src
	src/libc
)
link_libraries(common)

function(addFirmwareBinary name)
	add_executable(${name} ${ARGN})

	# As the GCC linker outputs executables in ELF format, the objcopy utility
	# must be run on the compiled binary to convert it to a raw flash image.
	add_custom_command(
		TARGET     ${name} POST_BUILD
		BYPRODUCTS ${name}.bin
		COMMAND
			"${CMAKE_OBJCOPY}"
			-O binary
			"$<TARGET_FILE:${name}>"
			${name}.bin
		VERBATIM
	)
endfunction()

addFirmwareBinary(
	main
	src/main/util/hardware.cpp
	src/main/i2c.cpp
	src/main/input.cpp
	src/main/main.cpp
	src/main/vectors.s
)

target_compile_definitions(
	main PRIVATE
	VERSION="${PROJECT_VERSION}"
	#ENABLE_LOGGING=1
)
