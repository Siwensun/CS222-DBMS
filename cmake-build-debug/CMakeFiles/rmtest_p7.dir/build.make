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
include CMakeFiles/rmtest_p7.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/rmtest_p7.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/rmtest_p7.dir/flags.make

CMakeFiles/rmtest_p7.dir/rm/rmtest_p7.cc.obj: CMakeFiles/rmtest_p7.dir/flags.make
CMakeFiles/rmtest_p7.dir/rm/rmtest_p7.cc.obj: ../rm/rmtest_p7.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Workspace\cs222-fall19-team-34\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/rmtest_p7.dir/rm/rmtest_p7.cc.obj"
	C:\Apps\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\rmtest_p7.dir\rm\rmtest_p7.cc.obj -c C:\Workspace\cs222-fall19-team-34\rm\rmtest_p7.cc

CMakeFiles/rmtest_p7.dir/rm/rmtest_p7.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/rmtest_p7.dir/rm/rmtest_p7.cc.i"
	C:\Apps\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Workspace\cs222-fall19-team-34\rm\rmtest_p7.cc > CMakeFiles\rmtest_p7.dir\rm\rmtest_p7.cc.i

CMakeFiles/rmtest_p7.dir/rm/rmtest_p7.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/rmtest_p7.dir/rm/rmtest_p7.cc.s"
	C:\Apps\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Workspace\cs222-fall19-team-34\rm\rmtest_p7.cc -o CMakeFiles\rmtest_p7.dir\rm\rmtest_p7.cc.s

# Object files for target rmtest_p7
rmtest_p7_OBJECTS = \
"CMakeFiles/rmtest_p7.dir/rm/rmtest_p7.cc.obj"

# External object files for target rmtest_p7
rmtest_p7_EXTERNAL_OBJECTS =

rmtest_p7.exe: CMakeFiles/rmtest_p7.dir/rm/rmtest_p7.cc.obj
rmtest_p7.exe: CMakeFiles/rmtest_p7.dir/build.make
rmtest_p7.exe: libRM.a
rmtest_p7.exe: libRBFM.a
rmtest_p7.exe: libPFM.a
rmtest_p7.exe: CMakeFiles/rmtest_p7.dir/linklibs.rsp
rmtest_p7.exe: CMakeFiles/rmtest_p7.dir/objects1.rsp
rmtest_p7.exe: CMakeFiles/rmtest_p7.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\Workspace\cs222-fall19-team-34\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable rmtest_p7.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\rmtest_p7.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/rmtest_p7.dir/build: rmtest_p7.exe

.PHONY : CMakeFiles/rmtest_p7.dir/build

CMakeFiles/rmtest_p7.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\rmtest_p7.dir\cmake_clean.cmake
.PHONY : CMakeFiles/rmtest_p7.dir/clean

CMakeFiles/rmtest_p7.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Workspace\cs222-fall19-team-34 C:\Workspace\cs222-fall19-team-34 C:\Workspace\cs222-fall19-team-34\cmake-build-debug C:\Workspace\cs222-fall19-team-34\cmake-build-debug C:\Workspace\cs222-fall19-team-34\cmake-build-debug\CMakeFiles\rmtest_p7.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/rmtest_p7.dir/depend

