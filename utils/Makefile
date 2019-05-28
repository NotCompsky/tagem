default: build/mydefmt build/mydefmts build/myfindmedia build/myfindimg build/fmt-sql-tags build/mytagged_score build/tagtree build/bulk-tag build/view-cropped-tagged
	#gcc src/myfind.c     -O3 -o build/myfindvid
	#gcc src/myfind.c    -O3 -o build/myfindmedia -DFIND_MUSIC -DFIND_IMG -DFIND_VID
	#gcc src/myfind.c    -O3 -o build/myfindsvid -DFIND_VID -DPRINT_FSIZE
	#gcc src/myfind.c    -O3 -o build/myfindsmedia -DFIND_MUSIC -DFIND_IMG -DFIND_VID -DPRINT_FSIZE
	#gcc src/myfind.c    -O3 -o build/myfindtxt -DFIND_TXT -DFIND_NOEXT
	#g++ src/myfmt.cpp   -O3 -o build/myfmt

build/mydefmt:
	g++ src/mydefmt.cpp -O3 -o build/mydefmt
build/mydefmts:
	g++ src/mydefmt.cpp -O3 -o build/mydefmt -DSORT_FSIZE

build/myfindmedia:
	gcc src/myfind.c    -O3 -o build/myfindimg -DFIND_MUSIC -DFIND_IMG -DFIND_VID

build/myfindimg:
	gcc src/myfind.c    -O3 -o build/myfindimg -DFIND_IMG

build/fmt-sql-tags:
	g++ src/fmt-sql-tags.cpp -O3 -o build/fmt-sql-tags -I ~/bin/rscraper++/src -lmysqlcppconn

build/mytagged_score:
	g++ src/mytagged_score.cpp -O3 -o build/mytagged_score -I ~/bin/rscraper++/src -lmysqlcppconn

build/tagtree:
	g++ src/tagtree.cpp -O3 -o build/tagtree -I ~/bin/rscraper++/src -lmysqlcppconn

build/bulk-tag:
	g++ src/bulk-tag.cpp -o build/bulk-tag -O3 -lmysqlcppconn -I../include

build/view-cropped-tagged:
	g++ src/view-cropped-tagged.cpp -o build/view-cropped-tagged -O3 -I../include -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lmysqlcppconn
