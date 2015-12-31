TARGETS = 264bitstream

PWD=$(pwd)
PROJECT_SRC_DIR=src
PROJECT_INC_DIR=include
PROJECT_LIB_DIR=lib
PROJECT_OBJ_DIR=obj
MKDIR := mkdir -p

CC := g++

CFLAGS := -g -I$(PROJECT_INC_DIR)
LDFLAG := -L$(PROJECT_LIB_DIR)

src :=$(wildcard $(PROJECT_SRC_DIR)/*.c)
dir := $(notdir $(src))
PROJECT_OBJ := $(patsubst %.c,%.o,$(dir) )

#addprefix加前缀函数 $(addprefix src/,foo bar) 返回值为“src/foo src/bar”	
PROJECT_ALL_OBJS := $(addprefix $(PROJECT_OBJ_DIR)/, $(PROJECT_OBJ))
	
all:$(PROJECT_SRC_DIR) $(PROJECT_ALL_OBJS)
	$(CC) $(CFLAGS) $(PROJECT_ALL_OBJS) -o $(TARGETS) $(LDFLAG)

$(PROJECT_OBJ_DIR)/%.o : $(PROJECT_SRC_DIR)/%.c
	$(MKDIR) $(PROJECT_OBJ_DIR)
	$(CC) -c $(CFLAGS) $< -o $@
	
clean:
	rm -fr $(PROJECT_OBJ_DIR)
	rm -fr $(TARGETS)	
