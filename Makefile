# Makefile - Build the 64-bit RTL-SDR IQ Recorder
# 2025-05-31 ChatGPT - 

CC = gcc
CFLAGS = -O2 -Wall
TARGET = rtl_sdr64
SRC = rtl_sdr64.c
LIBS = -lrtlsdr

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

clean:
	rm -f $(TARGET) *.o core
