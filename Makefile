all: words

bing:
	g++ -g -Wall -o bing -std=c++14 bing.cc -lstdc++  -lcrypto -lssl -lcpprest -lboost_system

ftp:
	g++ -g -Wall -o ftp -std=c++14 ftp.cc -lstdc++  -lcrypto -lssl -lcpprest -lboost_system -lboost_regex

tut1:
	g++ -g -Wall -o tut1 -std=c++14 tut1.cc -lstdc++  -lcrypto -lssl -lcpprest -lboost_system

wanstat: wanstat.cc
	g++ -g -Wall -o wanstat -std=c++17 wanstat.cc -lstdc++  -lcrypto -lssl -lcpprest -lboost_system -lpthread  -lboost_program_options -lboost_regex

opencv=$(shell pkg-config --cflags --libs opencv)

evil: evil.cc
	g++ -g -Wall -o evil -std=c++14 evil.cc -lstdc++  -lcrypto -lssl -lcpprest -lboost_system -lpthread  -lboost_program_options $(opencv)

netrc:netrc.cc
	g++ -g3 -Wall -pedantic -o netrc -std=c++17 netrc.cc -lstdc++ -lpthread -lboost_system -lboost_filesystem -lboost_regex -lstdc++fs

words:words.cc
	g++ -O3 -Wall -o words -std=c++17 words.cc -lstdc++ -lpthread -lboost_system -lboost_filesystem -lboost_regex
#	g++ -g -Wall -o words -std=c++17 words.cc -lstdc++ -lpthread -lboost_system -lboost_filesystem -lboost_regex

clean:
	$(RM) words evil wanstat
