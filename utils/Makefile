default: build/mydefmt
	gcc src/myfind.c     -O3 -o build/myfindvid
	gcc src/myfind.c    -O3 -o build/myfindmedia -DFIND_MUSIC -DFIND_IMG -DFIND_VID
	gcc src/myfind.c    -O3 -o build/myfindsvid -DFIND_VID -DPRINT_FSIZE
	gcc src/myfind.c    -O3 -o build/myfindsmedia -DFIND_MUSIC -DFIND_IMG -DFIND_VID -DPRINT_FSIZE
	gcc src/myfind.c    -O3 -o build/myfindtxt -DFIND_TXT -DFIND_NOEXT
	g++ src/mydefmt.cpp -O3 -o build/mydefmts -DSORT_FSIZE
	g++ src/myfmt.cpp   -O3 -o build/myfmt

misc:
	gcc src/mystrlen.c       -O3 -o build/mystrlen

build/fmt-sql-tags:
	g++ src/fmt-sql-tags.cpp -O3 -o build/fmt-sql-tags -I ~/bin/rscraper++/src -lmysqlcppconn

build/mydefmt:
	g++ src/mydefmt.cpp -O3 -o build/mydefmt

build/mytagged_score:
	g++ src/mytagged_score.cpp -O3 -o build/mytagged_score -I ~/bin/rscraper++/src -lmysqlcppconn

build/tagtree:
	g++ src/tagtree.cpp -O3 -o build/tagtree -I ~/bin/rscraper++/src -lmysqlcppconn
