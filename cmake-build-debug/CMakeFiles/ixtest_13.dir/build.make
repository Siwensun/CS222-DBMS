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
include CMakeFiles/ixtest_13.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/ixtest_13.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ixtest_13.dir/flags.make

CMakeFiles/ixtest_13.dir/ix/ixtest_13.cc.obj: CMakeFiles/ixtest_13.dir/flags.make
CMakeFiles/ixtest_13.dir/ix/ixtest_13.cc.obj: ../ix/ixtest_13.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Workspace\cs222-fall19-team-34\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/ixtest_13.dir/ix/ixtest_13.cc.obj"
	C:\Apps\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\ixtest_13.dir\ix\ixtest_13.cc.obj -c C:\Workspace\cs222-fall19-team-34\ix\ixtest_13.cc

CMakeFiles/ixtest_13.dir/ix/ixtest_13.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ixtest_13.dir/ix/ixtest_13.cc.i"
	C:\Apps\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Workspace\cs222-fall19-team-34\ix\ixtest_13.cc > CMakeFiles\ixtest_13.dir\ix\ixtest_13.cc.i

CMakeFiles/ixtest_13.dir/ix/ixtest_13.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ixtest_13.dir/ix/ixtest_13.cc.s"
	C:\Apps\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Workspace\cs222-fall19-team-34\ix\ixtest_13.cc -o CMakeFiles\ixtest_13.dir\ix\ixtest_13.cc.s

# Object files for target ixtest_13
ixtest_13_OBJECTS = \
"CMakeFiles/ixtest_13.dir/ix/ixtest_13.cc.obj"

# External object files for target ixtest_13
ixtest_13_EXTERNAL_OBJECTS =

ixtest_13.exe: CMakeFiles/ixtest_13.dir/ix/ixtest_13.cc.obj
ixtest_13.exe: CMakeFiles/ixtest_13.dir/build.make
ixtest_13.exe: libIX.a
ixtest_13.exe: libRM.a
ixtest_13.exe: libRBFM.a
ixtest_13.exe: libPFM.a
ixtest_13.exe: CMakeFiles/ixtest_13.dir/linklibs.rsp
ixtest_13.exe: CMakeFiles/ixtest_13.dir/objects1.rsp
ixtest_13.exe: CMakeFiles/ixtest_13.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\Workspace\cs222-fall19-team-34\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ixtest_13.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\ixtest_13.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ixtest_13.dir/build: ixtest_13.exe

.PHONY : CMakeFiles/ixtest_13.dir/build

CMakeFiles/ixtest_13.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\ixtest_13.dir\cmake_clean.cmake
.PHONY : CMakeFiles/ixtest_13.dir/clean

CMakeFiles/ixtest_13.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Workspace\cs222-fall19-team-34 C:\Workspace\cs222-fall19-team-34 C:\Workspace\cs222-fall19-team-34\cmake-build-debug C:\Workspace\cs222-fall19-team-34\cmake-build-debug C:\Workspace\cs222-fall19-team-34\cmake-build-debug\CMakeFiles\ixtest_13.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ixtest_13.dir/depend

