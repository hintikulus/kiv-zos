# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.20

# Default target executed when no arguments are given to make.
default_target: all
.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:

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

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/hintik/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/212.5457.51/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/hintik/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/212.5457.51/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/hintik/Plocha/zos/sp

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/hintik/Plocha/zos/sp

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/home/hintik/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/212.5457.51/bin/cmake/linux/bin/cmake --regenerate-during-build -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache
.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake cache editor..."
	/home/hintik/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/212.5457.51/bin/cmake/linux/bin/ccmake -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache
.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/hintik/Plocha/zos/sp/CMakeFiles /home/hintik/Plocha/zos/sp//CMakeFiles/progress.marks
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/hintik/Plocha/zos/sp/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean
.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named sp

# Build rule for target.
sp: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 sp
.PHONY : sp

# fast build rule for target.
sp/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sp.dir/build.make CMakeFiles/sp.dir/build
.PHONY : sp/fast

commands.o: commands.c.o
.PHONY : commands.o

# target to build an object file
commands.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sp.dir/build.make CMakeFiles/sp.dir/commands.c.o
.PHONY : commands.c.o

commands.i: commands.c.i
.PHONY : commands.i

# target to preprocess a source file
commands.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sp.dir/build.make CMakeFiles/sp.dir/commands.c.i
.PHONY : commands.c.i

commands.s: commands.c.s
.PHONY : commands.s

# target to generate assembly for a file
commands.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sp.dir/build.make CMakeFiles/sp.dir/commands.c.s
.PHONY : commands.c.s

filesystem.o: filesystem.c.o
.PHONY : filesystem.o

# target to build an object file
filesystem.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sp.dir/build.make CMakeFiles/sp.dir/filesystem.c.o
.PHONY : filesystem.c.o

filesystem.i: filesystem.c.i
.PHONY : filesystem.i

# target to preprocess a source file
filesystem.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sp.dir/build.make CMakeFiles/sp.dir/filesystem.c.i
.PHONY : filesystem.c.i

filesystem.s: filesystem.c.s
.PHONY : filesystem.s

# target to generate assembly for a file
filesystem.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sp.dir/build.make CMakeFiles/sp.dir/filesystem.c.s
.PHONY : filesystem.c.s

inode.o: inode.c.o
.PHONY : inode.o

# target to build an object file
inode.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sp.dir/build.make CMakeFiles/sp.dir/inode.c.o
.PHONY : inode.c.o

inode.i: inode.c.i
.PHONY : inode.i

# target to preprocess a source file
inode.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sp.dir/build.make CMakeFiles/sp.dir/inode.c.i
.PHONY : inode.c.i

inode.s: inode.c.s
.PHONY : inode.s

# target to generate assembly for a file
inode.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sp.dir/build.make CMakeFiles/sp.dir/inode.c.s
.PHONY : inode.c.s

linkedlist.o: linkedlist.c.o
.PHONY : linkedlist.o

# target to build an object file
linkedlist.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sp.dir/build.make CMakeFiles/sp.dir/linkedlist.c.o
.PHONY : linkedlist.c.o

linkedlist.i: linkedlist.c.i
.PHONY : linkedlist.i

# target to preprocess a source file
linkedlist.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sp.dir/build.make CMakeFiles/sp.dir/linkedlist.c.i
.PHONY : linkedlist.c.i

linkedlist.s: linkedlist.c.s
.PHONY : linkedlist.s

# target to generate assembly for a file
linkedlist.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sp.dir/build.make CMakeFiles/sp.dir/linkedlist.c.s
.PHONY : linkedlist.c.s

main.o: main.c.o
.PHONY : main.o

# target to build an object file
main.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sp.dir/build.make CMakeFiles/sp.dir/main.c.o
.PHONY : main.c.o

main.i: main.c.i
.PHONY : main.i

# target to preprocess a source file
main.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sp.dir/build.make CMakeFiles/sp.dir/main.c.i
.PHONY : main.c.i

main.s: main.c.s
.PHONY : main.s

# target to generate assembly for a file
main.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/sp.dir/build.make CMakeFiles/sp.dir/main.c.s
.PHONY : main.c.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... edit_cache"
	@echo "... rebuild_cache"
	@echo "... sp"
	@echo "... commands.o"
	@echo "... commands.i"
	@echo "... commands.s"
	@echo "... filesystem.o"
	@echo "... filesystem.i"
	@echo "... filesystem.s"
	@echo "... inode.o"
	@echo "... inode.i"
	@echo "... inode.s"
	@echo "... linkedlist.o"
	@echo "... linkedlist.i"
	@echo "... linkedlist.s"
	@echo "... main.o"
	@echo "... main.i"
	@echo "... main.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

