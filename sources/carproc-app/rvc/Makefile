#
# Simple Makefile for cross compilation
#
# Author : Pierre-Yves MORDRET
#
CFLAGS	   =`pkg-config --cflags  gstreamer-1.0`
LDFLAGS	   =`pkg-config --libs gstreamer-1.0`

CFLAGS += -g -O0 -D_REENTRANT
LDFLGS += -lpthread

EXE:= rvc_handler
install_dir = .

all:  $(EXE)

clean:
	rm -f *.o $(EXE)

%.o: %.c
	@echo ""
	@echo "Compiling $<"
	$(CC) -Wall -c $(CFLAGS) $< -o $@

$(EXE):	$(EXE).o
	@echo ""
	@echo "Linking $<"
	$(CC) $(LDFLAGS) $< -o $@

install:
	install -d $(install_dir)
	@set -e; for i in $(EXE) ; do install $$i $(install_dir)/$$i ; done ;

