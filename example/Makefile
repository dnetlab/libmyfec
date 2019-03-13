TARGET = myvpn
MYFEC_DIR_PREFIX:=/usr/local

SRC = myvpn.c myudp.c mytun.c mynode.c myaddr.c
OBJ = $(SRC:%.c=%.o)

MYFEC_INC_DIR = $(MYFEC_DIR_PREFIX)/myfec/include
MYFEC_LIB_DIR = $(MYFEC_DIR_PREFIX)/myfec/lib

CFLAGS += -I$(MYFEC_INC_DIR)
CFLAGS += -L$(MYFEC_LIB_DIR) -lmyfec -lev

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) -c $< $(CFLAGS)

clean:
	rm -rf $(TARGET) $(OBJ)

.PHONY: all clean
