# Nome do executável
EXEC = ep1

# Lista de arquivos fonte
SRCS = main.c

# Lista de arquivos objeto
OBJS = $(SRCS:.c=.o)

# Compilador
CC = gcc

# Flags de compilação
CFLAGS = -Wall -pthread

# Regra padrão é build
build: $(EXEC)

# Linka os arquivos objeto e gera o executável
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS)

# Regra para compilar os arquivos .c em .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para executar o programa
run: $(EXEC)
	./$(EXEC)

# Regra clean para remover executáveis e arquivos objeto
clean:
	rm -f $(EXEC) $(OBJS)

# Como rodar:
#   'make' compilará o programa
#   'make run' executará o programa
#   'make clean' limpará o ambiente de compilação
