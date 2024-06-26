PROJECT_DIR ?= $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
TARGET_BIN ?= $(shell basename $(PROJECT_DIR))

export CMD
CMDS ?= $(wildcard $(CMD_DIR)/*)

BUILD_DIR ?= ./build
CMD_DIR ?= ./cmd
INC_DIR ?= ./inc
LIB_DIR ?= ./lib
TEST_DIR ?= ./tests

INCS := $(shell find $(INC_DIR) -regex '.+\.h')
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)

CC := clang
CFLAGS ?=
CFLAGS := $(CFLAGS) -std=c11 -D _DEFAULT_SOURCE \
	-I$(INC_DIR) -I$$PWD
TEST_CFLAGS := -Wall -I$(INC_DIR)
TEST_LDFLAGS := $(shell pkg-config --cflags --libs check)

MKDIR := mkdir
MKDIR_P := mkdir -p
