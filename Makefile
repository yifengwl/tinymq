

#######################################################################
 
###################项目路径和程序名称#################################
DIR=$(shell pwd)
BIN_DIR=$(DIR)
LIB_DIR=$(DIR)
SRC_DIR=$(DIR)
INCLUDE_DIR=$(DIR)
OBJ_DIR=$(DIR)
DEPS_DIR=$(DIR)
#PROGRAM=$(BIN_DIR)/test
PROGRAM=$(BIN_DIR)/tinymq
		 
###################OBJ文件及路径############################################
EXTENSION=cpp
OBJS=$(patsubst $(SRC_DIR)/%.$(EXTENSION), $(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.$(EXTENSION)))
DEPS=$(patsubst $(OBJ_DIR)/%.o, $(DEPS_DIR)/%.d, $(OBJS))
																								   
###################include头文件路径##################################
INCLUDE=\
        -I$(INCLUDE_DIR) 
		         
###################lib文件及路径######################################
LIB=
###################编译选项及编译器###################################
CC=g++
CFLAGS=-Wall -W -g  -std=c++11
LDFLAGS= -lpthread
 
###################编译目标###########################################
.PHONY: all clean rebuild
 
all:$(OBJS) 
	@echo $(DEPS_DIR)
	$(CC) -o $(PROGRAM) $(OBJS) $(LDFLAGS) 
		 
		 
$(DEPS_DIR)/%.d: $(SRC_DIR)/%.$(EXTENSION)
	$(CC) -MM $(INCLUDE) $(CFLAGS) $< | sed -e 1's,^,$(OBJ_DIR)/,' > $@
			 
sinclude $(DEPS)
			 
$(OBJ_DIR)/%.o:$(SRC_DIR)/%.$(EXTENSION) 
	$(CC) $< -o $@ -c $(CFLAGS) $(INCLUDE) 
				 
rebuild: clean all
				 
clean:
	rm -rf $(OBJS)  $(PROGRAM)

