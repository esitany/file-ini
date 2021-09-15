
SRC_DIR = $(shell pwd)

CC     := $(COMPILER_PREFIX)gcc
CXX    := $(COMPILER_PREFIX)g++
AR     := $(COMPILER_PREFIX)ar
STRIP  := $(COMPILER_PREFIX)strip
RANLIB := $(COMPILER_PREFIX)ranlib

DESTDIR ?= ./destdir


SOURCES  := file_ini.c

INCLUDES := file_ini.h

override DEFINES +=

CFLAGS   := -Wall \
            -fPIC \
            -g

LDFLAGS  :=

TARGET   := libini

OBJ_DIR  := obj

OBJ_LIB  := -Lobj \
            -lini

override INC_DIR += -I$(SRC_DIR)

ifeq ($(ARCH), x86)
  CFLAGS  += -m32
  LDFLAGS += -m32
endif

OBJ_CFLAGS := $(DEFINES) \
              $(INC_DIR) \
              -c \
              $(CFLAGS)

OBJ_LDFLAGS := $(LDFLAGS) \
               $(LIB_PATH)


OBJ_SRC    := $(SOURCES:%.c=$(OBJ_DIR)/%.o)

.PHONY: all lib bin clean install

all:
	make clean
	make lib
	make bin

bin:
	@echo "Compile..."
	$(CC) -o $(OBJ_DIR)/ini_test ini_test.c $(OBJ_LIB) $(OBJ_LDFLAGS) -Wl,-Map=$(OBJ_DIR)/$(TARGET).map

lib: $(OBJ_SRC)
	@echo "Compile...Library"
	$(CC) -shared -Wl,-soname,$(OBJ_DIR)/$(TARGET).so -o $(OBJ_DIR)/$(TARGET).so $(OBJ_SRC) $(OBJ_LDFLAGS)
	$(AR) -r $(OBJ_DIR)/$(TARGET).a $(OBJ_SRC)
	$(RANLIB) $(OBJ_DIR)/$(TARGET).a

$(OBJ_DIR)/%.o : %.c
	@echo "mkdir $(OBJ_DIR)..."
	@mkdir -p $(SRC_DIR)/$(OBJ_DIR)
	$(CC) $(OBJ_CFLAGS) -o $@ $<

clean:
	@echo "Clean $(SRC_DIR)/$(OBJ_DIR)..."
	rm -rvf $(SRC_DIR)/$(OBJ_DIR)

install:
	@echo "DEST Path = $(DESTDIR)"
	install -d $(DESTDIR)/inc
	install -d $(DESTDIR)/lib
	install -m 644 $(INCLUDES) $(DESTDIR)/inc
	install -m 644 $(OBJ_DIR)/$(TARGET).a $(DESTDIR)/lib
	# install -m 644 $(OBJ_DIR)/$(TARGET).so $(DESTDIR)/lib

