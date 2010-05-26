all:	clean provider server test

provider:
	gcc pc.c provider.c -o provider 
	
reader:
	gcc pc.c reader.c -o reader
	
server:
	gcc buffer.c list.c pc.c server.c -o server

test: 
	gcc buffer.c list.c fixtures_buffer.c \
		test_buffer.c test_main.c  -o tests -lpthread -lcunit
	./tests

clean:
	rm tests
	rm writer
	rm server
