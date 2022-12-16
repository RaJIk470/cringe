final:
	gcc -pthread lib/util.c lib/inet_wrap.c -g server.c -o server
	gcc -lncursesw -lformw -pthread lib/util.c lib/inet_wrap.c -g client.c -o client
