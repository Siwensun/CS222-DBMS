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
include CMakeFiles/rmtest_14.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/rmtest_14.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/rmtest_14.dir/flags.make

CMakeFiles/rmtest_14.dir/rm/rmtest_14.cc.obj: CMakeFiles/rmtest_14.dir/flags.make
CMakeFiles/rmtest_14.dir/rm/rmtest_14.cc.obj: ../rm/rmtest_14.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Workspace\cs222-fall19-team-34\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/rmtest_14.dir/rm/rmtest_14.cc.obj"
	C:\Apps\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\rmtest_14.dir\rm\rmtest_14.cc.obj -c C:\Workspace\cs222-fall19-team-34\rm\rmtest_14.cc

CMakeFiles/rmtest_14.dir/rm/rmtest_14.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/rmtest_14.dir/rm/rmtest_14.cc.i"
	C:\Apps\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Workspace\cs222-fall19-team-34\rm\rmtest_14.cc > CMakeFiles\rmtest_14.dir\rm\rmtest_14.cc.i

CMakeFiles/rmtest_14.dir/rm/rmtest_14.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/rmtest_14.dir/rm/rmtest_14.cc.s"
	C:\Apps\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Workspace\cs222-fall19-team-34\rm\rmtest_14.cc -o CMakeFiles\rmtest_14.dir\rm\rmtest_14.cc.s

# Object files for target rmtest_14
rmtest_14_OBJECTS = \
"CMakeFiles/rmtest_14.dir/rm/rmtest_14.cc.obj"

# External object files for target rmtest_14
rmtest_14_EXTERNAL_OBJECTS =

rmtest_14.exe: CMakeFiles/rmtest_14.dir/rm/rmtest_14.cc.obj
rmtest_14.exe: CMakeFiles/rmtest_14.dir/build.make
rmtest_14.exe: libRM.a
rmtest_14.exe: libRBFM.a
rmtest_14.exe: libPFM.a
rmtest_14.exe: CMakeFiles/rmtest_14.dir/linklibs.rsp
rmtest_14.exe: CMakeFiles/rmtest_14.dir/objects1.rsp
rmtest_14.exe: CMakeFiles/rmtest_14.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\Workspace\cs222-fall19-team-34\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable rmtest_14.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\rmtest_14.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/rmtest_14.dir/build: rmtest_14.exe

.PHONY : CMakeFiles/rmtest_14.dir/build

CMakeFiles/rmtest_14.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\rmtest_14.dir\cmake_clean.cmake
.PHONY : CMakeFiles/rmtest_14.dir/clean

CMakeFiles/rmtest_14.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Workspace\cs222-fall19-team-34 C:\Workspace\cs222-fall19-team-34 C:\Workspace\cs222-fall19-team-34\cmake-build-debug C:\Workspace\cs222-fall19-team-34\cmake-build-debug C:\Workspace\cs222-fall19-team-34\cmake-build-debug\CMakeFiles\rmtest_14.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/rmtest_14.dir/depend

