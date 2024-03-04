CC = gcc
CFLAGS = -Wall -Wextra -std=c99

all: TCP_Sender TCP_Receiver

TCP_Sender: 𝑻𝑪𝑷_Sender.o
	$(CC) $(CFLAGS) 𝑻𝑪𝑷_Sender.o -o 𝑻𝑪𝑷_Sender

TCP_Receiver: 𝑻𝑪𝑷_𝑹𝒆𝒄𝒆𝒊𝒗𝒆𝒓.o
	$(CC) $(CFLAGS) 𝑻𝑪𝑷_𝑹𝒆𝒄𝒆𝒊𝒗𝒆𝒓.o -o 𝑻𝑪𝑷_𝑹𝒆𝒄𝒆𝒊𝒗𝒆𝒓

clean:
	rm -f 𝑻𝑪𝑷_Sender 𝑻𝑪𝑷_𝑹𝒆𝒄𝒆𝒊𝒗𝒆𝒓 𝑻𝑪𝑷_Sender.o 𝑻𝑪𝑷_𝑹𝒆𝒄𝒆𝒊𝒗𝒆𝒓.o
