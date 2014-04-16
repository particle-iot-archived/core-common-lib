# This file is a makefile included from the top level makefile which
# defines the sources built for the target.

# Define the prefix to this directory. 
# Note: The name must be unique within this build and should be
#       based on the root of the project
TARGET_FATFS_PATH = FatFs_Module
TARGET_FATFS_SRC_PATH = $(TARGET_FATFS_PATH)/src

# Add to all objects built for this target
INCLUDE_DIRS += $(TARGET_FATFS_SRC_PATH)

# C source files included in this build.
CSRC += $(TARGET_FATFS_SRC_PATH)/diskio.c
CSRC += $(TARGET_FATFS_SRC_PATH)/ff.c

# C++ source files included in this build.
CPPSRC +=

# ASM source files included in this build.
ASRC +=

