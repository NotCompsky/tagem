debug-main:
	g++-8 main.cpp -o /tmp/mytags -g -DDEBUG -I"${HOME}/bin/bpcs/src" -lpthread -lreadline

release-main:
	g++-8 main.cpp -o /tmp/mytags.min -Ofast -I"${HOME}/bin/bpcs/src" -lpthread -lreadline

default:
	debug-main
