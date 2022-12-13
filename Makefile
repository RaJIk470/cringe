final:
	gcc -lpthread lib/util.c lib/inet_wrap.c -g server.c -o server
	gcc -lpthread lib/util.c lib/inet_wrap.c -g client.c -o client
