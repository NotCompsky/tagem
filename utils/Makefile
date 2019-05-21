default: build/mydefmt
	gcc src/myfind.c     -O3 -o build/myfindvid
	gcc src/myfind.c    -O3 -o build/myfindmedia -DFIND_MUSIC -DFIND_IMG -DFIND_VID
	gcc src/myfind.c    -O3 -o build/myfindsvid -DFIND_VID -DPRINT_FSIZE
	gcc src/myfind.c    -O3 -o build/myfindsmedia -DFIND_MUSIC -DFIND_IMG -DFIND_VID -DPRINT_FSIZE
	gcc src/myfind.c    -O3 -o build/myfindtxt -DFIND_TXT -DFIND_NOEXT
	g++ src/mydefmt.cpp -O3 -o build/mydefmts -DSORT_FSIZE
	g++ src/myfmt.cpp   -O3 -o build/myfmt

misc:
	g++ src/fmt-sql-tags.cpp -O3 -o build/fmt-sql-tags -lmysqlcppconn
	gcc src/mystrlen.c       -O3 -o build/mystrlen

build/mydefmt:
	g++ src/mydefmt.cpp -O3 -o build/mydefmt
