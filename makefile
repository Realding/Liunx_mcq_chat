mcq_chat:mcq_server mcq_client

mcq_server:mcq_server_ch.c
	gcc $^ -o $@ -lpthread

mcq_client:mcq_client_ch.c
	gcc $^ -o $@ -lpthread

.PHONY:clean
clean:
	rm -f *.o
.PHONY:remove
remove:
	rm -f mcq_client mcq_server
