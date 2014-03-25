#######################################################
# Change following variable to match your configuration
#######################################################

# put your favorite compilation options here
CUSTOM_CFLAGS := -msse -msse2 -ffast-math -mfpmath=sse -Wall -fPIC

# switch comment to build DEBUG version 
CUSTOM_CFLAGS	:= $(CUSTOM_CFLAGS) -DNDEBUG -O3 -g0
#CFLAGS	:= $(CFLAGS) -DDEBUG  -O0 -g2

# Add include locations and library locations
CUSTOM_CFLAGS := $(CUSTOM_FLAGS) -I.
CUSTOM_LDFLAGS := $(CUSTOM_LDFLAGS) -L.

##############################################
# Following lines should not be edited by user
##############################################

VERSION := 0.3

# align double allocation
CUSTOM_CFLAGS := $(CUSTOM_CFLAGS) -malign-double

# smarc use c99 standard
CUSTOM_CFLAGS := $(CUSTOM_CFLAGS) -std=c99

#add VERSION flag
CUSTOM_CFLAGS := $(CUSTOM_CFLAGS) -DVERSION='"$(VERSION)"'

CUSTOM_LDFLAGS := $(CUSTOM_LDFLAGS) -lm -lsndfile

CFLAGS = $(CUSTOM_CFLAGS) $(CFLAGS)
LDFLAGS = $(CUSTOM_LDFLAGS) $(LDFLAGS)

LIBSRC = remez_lp.c smarc.c stage_impl.c filtering.c polyfilt.c multi_stage.c
LIBOBJECTS = $(patsubst %.c,%.o,$(LIBSRC))
LIBTARGET = libsmarc.so

SRC = main.c
TARGET = smarc
OBJECTS = $(patsubst %.c,%.o,$(SRC)) $(LIBOBJECTS)

ARGTABLE_LIB_A := /usr/lib/libargtable2.a

all: $(TARGET) $(LIBTARGET)

$(TARGET): $(OBJECTS) 
	gcc $(LDFLAGS) $(OBJECTS) $(ARGTABLE_LIB_A) -o $(TARGET)

$(LIBTARGET): $(LIBOBJECTS)
	gcc -shared -Wl,-soname,libsmarc.so -o libsmarc.so.$(VERSION) $(LIBOBJECTS)
	ln -sf libsmarc.so.$(VERSION) libsmarc.so

%.o: %.c
	echo "compiling $<"
	gcc $(CFLAGS) -c $<
	
clean:
	rm -f $(OBJECTS)

