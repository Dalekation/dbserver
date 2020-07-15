all: client server
server: mainServ.o dataBase.o datetime.o phoneservice.o query.o server.o
	g++ mainServ.o dataBase.o datetime.o phoneservice.o query.o server.o -o server
mainServ.o: mainServ.cpp dataBase.h phoneservice.h datetime.h query.h
	g++ -Wall -Wextra -Werror -pedantic -std=gnu++0x -c mainServ.cpp
client: mainCl.o client.o datetime.o phoneservice.o query.o dataBase.o
	g++ mainCl.o client.o datetime.o phoneservice.o query.o dataBase.o -o client
mainCl.o: mainCl.cpp 
	g++ -Wall -Wextra -Werror -pedantic -std=gnu++0x -c mainCl.cpp
client.o: client.cpp phoneservice.h datetime.h
	g++ -Wall -Wextra -Werror -pedantic -std=gnu++0x -c client.cpp
server.o: server.cpp server.h
	g++ -Wall -Wextra -Werror -pedantic -std=gnu++0x -c server.cpp
datetime.o: datetime.cpp datetime.h
	g++ -Wall -Wextra -Werror -pedantic -std=gnu++0x -c datetime.cpp
phoneservice.o: phoneservice.cpp phoneservice.h datetime.h
	g++ -Wall -Wextra -Werror -pedantic -std=gnu++0x -c phoneservice.cpp
dataBase.o: dataBase.cpp dataBase.h phoneservice.h datetime.h query.h
	g++ -Wall -Wextra -Werror -pedantic -std=gnu++0x -c dataBase.cpp
query.o: query.cpp query.h phoneservice.h datetime.h dataBase.h
	g++ -Wall -Wextra -Werror -pedantic -std=gnu++0x -c query.cpp
clean:
	rm -f *.o server client