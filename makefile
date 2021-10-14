DEPENDENCY= input_parser.o send_command.o
all: client

client: client.c $(DEPENDENCY)
	gcc -g client.c $(DEPENDENCY) -o $@

%.o: %.c
	gcc -c $^
