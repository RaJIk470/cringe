final:
	gcc -pthread lib/util.c lib/inet_wrap.c -g server.c -o server
	gcc -pthread lib/util.c lib/inet_wrap.c -g client.c -o client
