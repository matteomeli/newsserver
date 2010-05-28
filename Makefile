all:	clean provider server test

provider:
	gcc pc.c provider.c -o provider 
	
reader:
	gcc list.c pc.c reader.c -o reader -lpthread
	
server:
	gcc buffer.c list.c pc.c client_id.c news.c server.c -o server -lpthread

test: 
	gcc buffer.c list.c fixtures_buffer.c \
		test_buffer.c test_main.c  -o tests -lpthread -lcunit
	./tests

clean:
	rm tests
	rm writer
	rm server
