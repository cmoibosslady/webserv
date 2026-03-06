MAKE = make
MAKEFLAGS += --no-print-directory


CC = c++

CFLAGS = -Wall -Wextra -Werror -MMD -MP -std=c++98 -g

NAME = webserv


SRC_DIR = srcs

OBJ_DIR = objs

BUILD_DIR = build

DEP_DIR = deps

HEADER_DIR = include


SRCS = main.cpp \
	   add_to_config.cpp \
	   config.cpp \
	   ft_mem.cpp \
	   overload_config.cpp \
	   stoul.cpp \
	   tokeniser.cpp 

HEADER =  main.hpp \
		  config.hpp \
		  ft_mem.hpp \
		  stoul.hpp \
		  tokeniser.hpp
			

OBJS = $(SRCS:%.cpp=${OBJ_DIR}/%.o)

DEPS = $(SRCS:%.cpp=$(DEP_DIR)/%.d)


INCLUDE = -I $(HEADER_DIR) 

BUILD = $(BUILD_DIR)/main.out


all: $(NAME)
-include $(DEPS)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDE) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR) $(DEP_DIR)
	$(CC) $(CFLAGS) $(INCLUDE) \
		-c $< -o $@ \
		-MF $(DEP_DIR)/$(notdir $(basename $<)).d -MT $@

clean:
	rm -rf $(OBJ_DIR) $(BUILD_DIR) $(DEP_DIR)

fclean: 
	$(MAKE) clean
	rm -rf $(NAME)

re:
	$(MAKE) fclean
	$(MAKE) all

.PHONY: all hex flash clean re
