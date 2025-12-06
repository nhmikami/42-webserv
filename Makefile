# program name
NAME		=	webserv

# compiler and flags
CC			=	c++
CFLAGS		=	-Wall -Wextra -Werror -std=c++98

# directories
SRC_DIR		=	src/
CONF_DIR	=	src/config/
SERV_DIR	=	src/server/
UTIL_DIR	= 	src/utils/
INC_DIR		=	inc/
OBJ_DIR		=	obj/

# source files
SRC			=	$(addprefix $(SRC_DIR), main.cpp \
					Request.cpp ParseHttp.cpp ParseHttpReader.cpp ParseHttpValidator.cpp ParseUri.cpp ParseCookie.cpp \
					Response.cpp AMethod.cpp MethodGET.cpp MethodPOST.cpp MethodDELETE.cpp CgiHandler.cpp) \
				$(addprefix $(CONF_DIR), ParseConfig.cpp BaseConfig.cpp ServerConfig.cpp LocationConfig.cpp) \
				$(addprefix $(SERV_DIR), Server.cpp Client.cpp) \
				$(addprefix $(UTIL_DIR), Logger.cpp ParseUtils.cpp)
OBJ			=	$(SRC:$(SRC_DIR)%.cpp=$(OBJ_DIR)%.o)
INC			=	-I $(INC_DIR)

# colors
RESET		= \033[0m
RED			= \033[0;31m
YELLOW		= \033[0;33m
GREEN_BOLD	= \033[1;32m

# rules
all:			$(NAME)

$(NAME):		$(OBJ)
					@$(CC) $(CFLAGS) $(OBJ) -o $(NAME)
					@echo "$(GREEN_BOLD)$(NAME) compiled successfully!$(RESET)"

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INC) -c $< -o $@
	@echo "$(YELLOW)[compiling]$(RESET) $@"

val: re
	@valgrind -q --leak-check=full \
				--show-leak-kinds=all \
				--track-origins=yes \
				--track-fds=yes \
				--trace-children=yes \
				--trace-children-skip='*/bin/*,*/sbin/*,/usr/bin/*' \
				./${NAME} ./config/complete.conf
						
clean:
				@rm -rf $(OBJ_DIR)
				@echo "$(RED)[cleaning]$(RESET) objects cleaned"

fclean:			clean
				@rm -f $(NAME)
				@echo "$(RED)[cleaning]$(RESET) $(NAME) removed"

re:				fclean all

.PHONY: all clean fclean re
