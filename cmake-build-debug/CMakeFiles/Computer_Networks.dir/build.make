# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/cmake-3.17.1-Linux-x86_64/bin/cmake

# The command to remove a file.
RM = /opt/cmake-3.17.1-Linux-x86_64/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/mnt/c/Users/tomma/Coding/CProjects/Computer Networks"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/mnt/c/Users/tomma/Coding/CProjects/Computer Networks/cmake-build-debug"

# Include any dependencies generated for this target.
include CMakeFiles/Computer_Networks.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/Computer_Networks.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Computer_Networks.dir/flags.make

CMakeFiles/Computer_Networks.dir/wc20.10_explained.c.o: CMakeFiles/Computer_Networks.dir/flags.make
CMakeFiles/Computer_Networks.dir/wc20.10_explained.c.o: ../wc20.10_explained.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/mnt/c/Users/tomma/Coding/CProjects/Computer Networks/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/Computer_Networks.dir/wc20.10_explained.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/Computer_Networks.dir/wc20.10_explained.c.o   -c "/mnt/c/Users/tomma/Coding/CProjects/Computer Networks/wc20.10_explained.c"

CMakeFiles/Computer_Networks.dir/wc20.10_explained.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/Computer_Networks.dir/wc20.10_explained.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/mnt/c/Users/tomma/Coding/CProjects/Computer Networks/wc20.10_explained.c" > CMakeFiles/Computer_Networks.dir/wc20.10_explained.c.i

CMakeFiles/Computer_Networks.dir/wc20.10_explained.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/Computer_Networks.dir/wc20.10_explained.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/mnt/c/Users/tomma/Coding/CProjects/Computer Networks/wc20.10_explained.c" -o CMakeFiles/Computer_Networks.dir/wc20.10_explained.c.s

# Object files for target Computer_Networks
Computer_Networks_OBJECTS = \
"CMakeFiles/Computer_Networks.dir/wc20.10_explained.c.o"

# External object files for target Computer_Networks
Computer_Networks_EXTERNAL_OBJECTS =

Computer_Networks: CMakeFiles/Computer_Networks.dir/wc20.10_explained.c.o
Computer_Networks: CMakeFiles/Computer_Networks.dir/build.make
Computer_Networks: CMakeFiles/Computer_Networks.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/mnt/c/Users/tomma/Coding/CProjects/Computer Networks/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable Computer_Networks"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Computer_Networks.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Computer_Networks.dir/build: Computer_Networks

.PHONY : CMakeFiles/Computer_Networks.dir/build

CMakeFiles/Computer_Networks.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Computer_Networks.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Computer_Networks.dir/clean

CMakeFiles/Computer_Networks.dir/depend:
	cd "/mnt/c/Users/tomma/Coding/CProjects/Computer Networks/cmake-build-debug" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/mnt/c/Users/tomma/Coding/CProjects/Computer Networks" "/mnt/c/Users/tomma/Coding/CProjects/Computer Networks" "/mnt/c/Users/tomma/Coding/CProjects/Computer Networks/cmake-build-debug" "/mnt/c/Users/tomma/Coding/CProjects/Computer Networks/cmake-build-debug" "/mnt/c/Users/tomma/Coding/CProjects/Computer Networks/cmake-build-debug/CMakeFiles/Computer_Networks.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : CMakeFiles/Computer_Networks.dir/depend

