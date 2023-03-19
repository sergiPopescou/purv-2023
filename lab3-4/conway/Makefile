
DESTDIR?=/usr
PREFIX?=/local
LOCAL_INCL=./inc

ifneq ($V,1)
Q ?= @
endif

#DEBUG	= -g -O0
DEBUG		= -O2
CC		= gcc
INCLUDE		= -I$(DESTDIR)$(PREFIX)/include -I$(LOCAL_INCL)
CFLAGS		= $(DEBUG) -Wall -Wextra $(INCLUDE) -Winline -pipe

LDFLAGS		= -L$(DESTDIR)$(PREFIX)/lib
LIBS    	= -lpthread -lrt
SRC_DIR 	= src
OUT_DIR		= bin
OBJ_DIR		= obj
EXEC_NAME 	= conway

# May not need to  alter anything below this line
###############################################################################

SRC	=	$(SRC_DIR)/conway.c

OBJ	= 	$(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
all:		$(EXEC_NAME)

version.h:	VERSION
	$Q echo Need to run newVersion above.

$(EXEC_NAME):	$(OBJ)
	$Q echo [Link] $<
	$Q $(CC) -o $(OUT_DIR)/$@ $(OBJ) $(LDFLAGS) $(LIBS)

$(OBJ): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$Q echo [Compile] $<
	$Q $(CC) -c $(CFLAGS) $< -o $@

.PHONY:	clean
clean:
	$Q echo "[Clean]"
	$Q rm -f $(OBJ) $(OUT_DIR)/gpio *~ core tags *.bak

.PHONY:	tags
tags:	$(SRC)
	$Q echo [ctags]
	$Q ctags $(SRC)

# DO NOT DELETE

gpio.o: ../version.h
