default:
	gcc src/myfind.c     -O3 -o build/myfindvid
	gcc src/myfind.c    -O3 -o build/myfindmedia -DFIND_MUSIC -DFIND_IMG
	gcc src/myfind.c    -O3 -o build/myfindsvid -DPRINT_FSIZE
	gcc src/myfind.c    -O3 -o build/myfindsmedia -DFIND_MUSIC -DFIND_IMG -DPRINT_FSIZE
	g++ src/mydefmt.cpp -O3 -o build/mydefmt
	g++ src/myfmt.cpp   -O3 -o build/myfmt

misc:
	g++ src/fmt-sql-tags.cpp -O3 -o build/fmt-sql-tags -lmysqlcppconn
	gcc src/mystrlen.c       -O3 -o build/mystrlen
