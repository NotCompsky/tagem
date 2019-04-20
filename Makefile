default:
	gcc findmedia.c -Ofast -o myfindvid
	gcc findmedia.c -Ofast -o myfindmedia -DFIND_MUSIC -DFIND_IMG
	gcc findmedia.c -Ofast -o myfindsvid -DPRINT_FSIZE
	gcc findmedia.c -Ofast -o myfindsmedia -DFIND_MUSIC -DFIND_IMG -DPRINT_FSIZE
	g++ mydefmt.cpp -o mydefmt -O3
	g++ myfmt.cpp -o myfmt -O3
