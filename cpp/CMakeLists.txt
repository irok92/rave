cmake_minimum_required(VERSION 4.0)

project(
	rave
	VERSION 0.1.0
	DESCRIPTION "Rave is a programming language for everyone."
	HOMEPAGE_URL "https://github.com/rave-lang/rave"
	LANGUAGES C CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel)
	string(TOUPPER ${config} config_upper)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper} ${CMAKE_BINARY_DIR}/bin)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${config_upper} ${CMAKE_BINARY_DIR}/bin)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${config_upper} ${CMAKE_BINARY_DIR}/bin)
endforeach()


find_package(LLVM CONFIG REQUIRED)
find_program(LLVM_CONFIG_EXECUTABLE llvm-config REQUIRED)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
enable_testing()


execute_process(
	COMMAND ${LLVM_CONFIG_EXECUTABLE} --system-libs
	OUTPUT_VARIABLE LLVM_SYSTEM_LIBS
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
separate_arguments(LLVM_SYSTEM_LIBS)
message(STATUS "LLVM System Libraries: ${LLVM_SYSTEM_LIBS}")

execute_process(
	COMMAND ${LLVM_CONFIG_EXECUTABLE} --libfiles core orcjit native analysis codegen passes target mc
	OUTPUT_VARIABLE LLVM_LIB_FILES
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
separate_arguments(LLVM_LIB_FILES)
message(STATUS "LLVM Lib Files: ${LLVM_LIB_FILES}")


file(GLOB_RECURSE RAVE_SOURCES CONFIGURE_DEPENDS ./*.cpp ./language/*.hpp)
add_executable(rave ${RAVE_SOURCES})
target_include_directories(rave PRIVATE ./ ${LLVM_INCLUDE_DIRS})
target_link_directories(rave PRIVATE .deps/libs)
target_link_libraries(rave PRIVATE ${LLVM_LIB_FILES} ${LLVM_SYSTEM_LIBS})

# function(configure_rave)
# 	find_package(LLVM CONFIG REQUIRED)
# 	message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
# 	message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
# 	enable_testing()

# 	find_program(LLVM_CONFIG_EXECUTABLE llvm-config REQUIRED)

# 	execute_process(
# 		COMMAND ${LLVM_CONFIG_EXECUTABLE} --system-libs
# 		OUTPUT_VARIABLE LLVM_SYSTEM_LIBS
# 		OUTPUT_STRIP_TRAILING_WHITESPACE
# 	)
# 	separate_arguments(LLVM_SYSTEM_LIBS)

# 	execute_process(
# 		COMMAND ${LLVM_CONFIG_EXECUTABLE} --libfiles core orcjit native analysis codegen passes target mc
# 		OUTPUT_VARIABLE LLVM_LIB_FILES
# 		OUTPUT_STRIP_TRAILING_WHITESPACE
# 	)
# 	separate_arguments(LLVM_LIB_FILES)

# 	message(STATUS "LLVM System Libraries: ${LLVM_SYSTEM_LIBS}")
# 	message(STATUS "LLVM Lib Files: ${LLVM_LIB_FILES}")
# 	message(STATUS "LLVM Include Dirs: ${LLVM_INCLUDE_DIRS}")

# 	file(GLOB_RECURSE RAVE_SOURCES CONFIGURE_DEPENDS language/*.cpp)

# 	add_executable(rave ${RAVE_SOURCES})
# 	target_include_directories(rave PRIVATE ${LLVM_INCLUDE_DIRS})
# 	target_link_directories(rave PRIVATE .deps/libs)
# 	target_link_libraries(rave PRIVATE ${LLVM_LIB_FILES} ${LLVM_SYSTEM_LIBS})
# endfunction()

# function(configure_space)
# 	set(SPACE_SOURCES
# 		space/main.cpp
# 	)

# 	add_executable(space ${SPACE_SOURCES})
# 	add_custom_command(TARGET space POST_BUILD
# 		COMMAND ${CMAKE_COMMAND} -E copy_directory
# 			${CMAKE_SOURCE_DIR}/assets
# 			$<TARGET_FILE_DIR:space>/assets
# 	)

# 	target_compile_definitions(space PRIVATE)
# 	target_include_directories(space PRIVATE
# 		.deps/SDL3/include
# 		.deps/SDL3_image/include
# 		.deps/SDL3_mixer/include
# 		.deps/SDL3_ttf/include
# 		.deps/SDL3_shadercross/include
# 	)
# 	target_link_directories(space PRIVATE
# 		.deps/SDL3/lib/x64
# 		.deps/SDL3_image/lib/x64
# 		.deps/SDL3_mixer/lib/x64
# 		.deps/SDL3_ttf/lib/x64
# 		.deps/SDL3_shadercross/lib/x64
# 	)
# 	target_link_libraries(space PRIVATE
# 		SDL3
# 		SDL3_image
# 		SDL3_mixer
# 		SDL3_ttf
# 		SDL3_shadercross
# 	)
# endfunction()

# #sconfigure_rave()
# configure_space()
