CC = gcc

MP = mp

S01_SRC = example01.c
S01_OBJ = $(S01_SRC:.c=.o)
S01 = ./example01

S02_SRC = example02.c
S02_OBJ = $(S02_SRC:.c=.o)
S02 = ./example02

S03_SRC = example03.c
S03_OBJ = $(S03_SRC:.c=.o)
S03 = ./example03

S04_SRC = example04.c
S04_OBJ = $(S04_SRC:.c=.o)
S04 = ./example04

CFLAGS = -Wall -g -pedantic -std=c99

%.o: %.c
	$(CC) $(CFLAGS) -I../src -c -o $@ $<

all: $(S01) $(S02) $(S03) $(S04)

$(S01):	$(S01_OBJ)
	$(CC) $< -L.. -l$(MP) -pthread -o $@

$(S02):	$(S02_OBJ)
	$(CC) $< -L.. -l$(MP) -pthread -o $@

$(S03):	$(S03_OBJ)
	$(CC) $< -L.. -l$(MP) -pthread -o $@

$(S04):	$(S04_OBJ)
	$(CC) $< -L.. -l$(MP) -pthread -o $@

clean:
	rm -rf *.o example??
