
SRC_DIR = $(shell pwd)

CC    := $(COMPILER_PREFIX)gcc
CXX   := $(COMPILER_PREFIX)g++
AR    := $(COMPILER_PREFIX)ar
STRIP := $(COMPILER_PREFIX)strip

DESTDIR ?= ./destdir


SOURCES   := file_ini.c

INCLUDES  := file_ini.h


CFLAGS += -Wall \
          -fPIC \
          -g

LDFLAGS :=

TARGET := libini

OBJ_DIR := obj

OBJ_LIB := -Lobj \
           -lini

INC_DIR := -I$(SRC_DIR)

OBJ_CFLAGS := $(INC_DIR) \
              -c \
              $(CFLAGS)

OBJ_LDFLAGS := $(LDFLAGS) \
               $(LIB_PATH)


OBJ_SRC    := $(SOURCES:%.c=$(OBJ_DIR)/%.o)

.PHONY: all lib bin clean install

all:
	make clean
	make lib

bin: $(OBJ_DIR)/test_$(TARGET)

lib: $(OBJ_DIR)/$(TARGET).a

$(OBJ_DIR)/test_$(TARGET):
	@echo "mkdir $(OBJ_DIR)..."
	$(CC) -o $@ main.c $(OBJ_LIB) $(OBJ_LDFLAGS) -Wl,-Map=$(OBJ_DIR)/$(TARGET).map

$(OBJ_DIR)/$(TARGET).a: $(OBJ_SRC)
	@echo "mkdir $(OBJ_DIR)..."
	@mkdir -p $(SRC_DIR)/$(OBJ_DIR)
	$(AR) -r $@ $(OBJ_SRC)

$(OBJ_DIR)/%.o : %.c
	@echo "mkdir $(OBJ_DIR)..."
	@mkdir -p $(SRC_DIR)/$(OBJ_DIR)
	$(CC) $(DEFS) $(OBJ_CFLAGS) -o $@ $<

clean:
	@echo "Clean $(SRC_DIR)/$(OBJ_DIR)..."
	rm -rvf $(SRC_DIR)/$(OBJ_DIR)

install:
	@echo "DEST Path = $(DESTDIR)"
	install -d $(DESTDIR)/inc
	install -d $(DESTDIR)/lib
	install -m 644 $(INCLUDES) $(DESTDIR)/inc
	install -m 644 $(OBJ_DIR)/$(TARGET).a $(DESTDIR)/lib

uninstall:
	@echo "DEST Path = $(DESTDIR)"
	@rm -rvf $(DESTDIR)
	# @rm -rf $(DESTDIR)/bin/$(TARGET)
	# @rm -rf $(DESTDIR)/lib/lib$(TARGET).a
	# @rm -rf $(DESTDIR)/inc/$(INCLUDES)


