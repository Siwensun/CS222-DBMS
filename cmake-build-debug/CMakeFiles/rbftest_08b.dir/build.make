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
include CMakeFiles/rbftest_08b.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/rbftest_08b.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/rbftest_08b.dir/flags.make

CMakeFiles/rbftest_08b.dir/rbf/rbftest_08b.cc.obj: CMakeFiles/rbftest_08b.dir/flags.make
CMakeFiles/rbftest_08b.dir/rbf/rbftest_08b.cc.obj: ../rbf/rbftest_08b.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Workspace\cs222-fall19-team-34\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/rbftest_08b.dir/rbf/rbftest_08b.cc.obj"
	C:\Apps\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\rbftest_08b.dir\rbf\rbftest_08b.cc.obj -c C:\Workspace\cs222-fall19-team-34\rbf\rbftest_08b.cc

CMakeFiles/rbftest_08b.dir/rbf/rbftest_08b.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/rbftest_08b.dir/rbf/rbftest_08b.cc.i"
	C:\Apps\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Workspace\cs222-fall19-team-34\rbf\rbftest_08b.cc > CMakeFiles\rbftest_08b.dir\rbf\rbftest_08b.cc.i

CMakeFiles/rbftest_08b.dir/rbf/rbftest_08b.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/rbftest_08b.dir/rbf/rbftest_08b.cc.s"
	C:\Apps\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Workspace\cs222-fall19-team-34\rbf\rbftest_08b.cc -o CMakeFiles\rbftest_08b.dir\rbf\rbftest_08b.cc.s

# Object files for target rbftest_08b
rbftest_08b_OBJECTS = \
"CMakeFiles/rbftest_08b.dir/rbf/rbftest_08b.cc.obj"

# External object files for target rbftest_08b
rbftest_08b_EXTERNAL_OBJECTS =

rbftest_08b.exe: CMakeFiles/rbftest_08b.dir/rbf/rbftest_08b.cc.obj
rbftest_08b.exe: CMakeFiles/rbftest_08b.dir/build.make
rbftest_08b.exe: libRBFM.a
rbftest_08b.exe: libPFM.a
rbftest_08b.exe: CMakeFiles/rbftest_08b.dir/linklibs.rsp
rbftest_08b.exe: CMakeFiles/rbftest_08b.dir/objects1.rsp
rbftest_08b.exe: CMakeFiles/rbftest_08b.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\Workspace\cs222-fall19-team-34\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable rbftest_08b.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\rbftest_08b.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/rbftest_08b.dir/build: rbftest_08b.exe

.PHONY : CMakeFiles/rbftest_08b.dir/build

CMakeFiles/rbftest_08b.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\rbftest_08b.dir\cmake_clean.cmake
.PHONY : CMakeFiles/rbftest_08b.dir/clean

CMakeFiles/rbftest_08b.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Workspace\cs222-fall19-team-34 C:\Workspace\cs222-fall19-team-34 C:\Workspace\cs222-fall19-team-34\cmake-build-debug C:\Workspace\cs222-fall19-team-34\cmake-build-debug C:\Workspace\cs222-fall19-team-34\cmake-build-debug\CMakeFiles\rbftest_08b.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/rbftest_08b.dir/depend

