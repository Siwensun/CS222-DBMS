# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.15

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\JetBrains\CLion 2019.2.4\bin\cmake\win\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\JetBrains\CLion 2019.2.4\bin\cmake\win\bin\cmake.exe" -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Workspace\cs222-fall19-team-34

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Workspace\cs222-fall19-team-34\cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/cli_example_03.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/cli_example_03.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/cli_example_03.dir/flags.make

CMakeFiles/cli_example_03.dir/cli/cli_example_03.cc.obj: CMakeFiles/cli_example_03.dir/flags.make
CMakeFiles/cli_example_03.dir/cli/cli_example_03.cc.obj: ../cli/cli_example_03.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Workspace\cs222-fall19-team-34\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/cli_example_03.dir/cli/cli_example_03.cc.obj"
	C:\Apps\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\cli_example_03.dir\cli\cli_example_03.cc.obj -c C:\Workspace\cs222-fall19-team-34\cli\cli_example_03.cc

CMakeFiles/cli_example_03.dir/cli/cli_example_03.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cli_example_03.dir/cli/cli_example_03.cc.i"
	C:\Apps\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Workspace\cs222-fall19-team-34\cli\cli_example_03.cc > CMakeFiles\cli_example_03.dir\cli\cli_example_03.cc.i

CMakeFiles/cli_example_03.dir/cli/cli_example_03.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cli_example_03.dir/cli/cli_example_03.cc.s"
	C:\Apps\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Workspace\cs222-fall19-team-34\cli\cli_example_03.cc -o CMakeFiles\cli_example_03.dir\cli\cli_example_03.cc.s

# Object files for target cli_example_03
cli_example_03_OBJECTS = \
"CMakeFiles/cli_example_03.dir/cli/cli_example_03.cc.obj"

# External object files for target cli_example_03
cli_example_03_EXTERNAL_OBJECTS =

cli_example_03.exe: CMakeFiles/cli_example_03.dir/cli/cli_example_03.cc.obj
cli_example_03.exe: CMakeFiles/cli_example_03.dir/build.make
cli_example_03.exe: libCLI.a
cli_example_03.exe: libQE.a
cli_example_03.exe: libIX.a
cli_example_03.exe: libRM.a
cli_example_03.exe: libRBFM.a
cli_example_03.exe: libPFM.a
cli_example_03.exe: CMakeFiles/cli_example_03.dir/linklibs.rsp
cli_example_03.exe: CMakeFiles/cli_example_03.dir/objects1.rsp
cli_example_03.exe: CMakeFiles/cli_example_03.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\Workspace\cs222-fall19-team-34\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable cli_example_03.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\cli_example_03.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/cli_example_03.dir/build: cli_example_03.exe

.PHONY : CMakeFiles/cli_example_03.dir/build

CMakeFiles/cli_example_03.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\cli_example_03.dir\cmake_clean.cmake
.PHONY : CMakeFiles/cli_example_03.dir/clean

CMakeFiles/cli_example_03.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Workspace\cs222-fall19-team-34 C:\Workspace\cs222-fall19-team-34 C:\Workspace\cs222-fall19-team-34\cmake-build-debug C:\Workspace\cs222-fall19-team-34\cmake-build-debug C:\Workspace\cs222-fall19-team-34\cmake-build-debug\CMakeFiles\cli_example_03.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/cli_example_03.dir/depend

