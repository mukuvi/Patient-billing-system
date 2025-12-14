CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LDFLAGS = -lsqlite3 -lm

TARGET = hospital_billing
SRC = hospital_billing.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o hospital.db

run: $(TARGET)
	./$(TARGET)

backup:
	cp hospital.db hospital_backup_$(shell date +%Y%m%d_%H%M%S).db

.PHONY: all clean run backup
