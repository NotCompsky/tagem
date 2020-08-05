# Has to be in the root directory, otherwise the docker build system will not allow copying the necessary files from the host to the container
# All the other Dockerfiles are under wangle-server/docker

FROM notcompsky/wangle-static-and-ffmpeg:latest AS intermediate
WORKDIR /tagem
# NOTE: libcompsky should be rebuilt every time, there is a reasonable chance that it is upgraded when tagem is
COPY wangle-server/docker/ffmpegthumbnailer-static.patch /ffmpegthumbnailer-static.patch

ENV PATH=$PATH:/usr/local/x86_64-linux-musl/lib
ENV CC=/usr/local/bin/x86_64-linux-musl-gcc
ENV CXX=/usr/local/bin/x86_64-linux-musl-g++
ENV C_INCLUDE_PATH=/usr/local/x86_64-linux-musl/include
ENV LDFLAGS="-Wl,-Bstatic"
ENV CFLAGS="-static"
ENV CXXFLAGS="-static"

WORKDIR /tagem
COPY wangle-server /tagem/wangle-server
COPY utils /tagem/utils
COPY include /tagem/include
# WARNING: -fpermissive is used because Facebook's wangle library currently has a line in its logging library that tries to convert void** to void*

RUN git clone --depth 1 https://github.com/dirkvdb/ffmpegthumbnailer \
	&& cd ffmpegthumbnailer \
	&& git apply /ffmpegthumbnailer-static.patch \
	&& mkdir build \
	&& cd build \
	&& cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES=/FFmpeg \
		-DENABLE_SHARED=OFF \
		-DENABLE_STATIC=ON \
		-DENABLE_TESTS=OFF .. \
	&& make install \
	\
	&& git clone --depth 1 https://github.com/NotCompsky/libcompsky \
	&& mkdir libcompsky/build \
	&& cd libcompsky/build \
	&& cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-DWHICH_MYSQL_CLIENT=mariadbclient \
		-DMYSQL_IS_UNDER_MARIADB_DIR=1 \
		-DMYSQL_UNDER_DIR_OVERRIDE=1 \
		-DCMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES=/usr/local/include \
		.. \
	&& make install \
	\
	&& chmod +x /tagem/wangle-server/scripts/* \
	&& rm -rf /tagem/build \
	;  mkdir /tagem/build \
	&& cd /tagem/build \
	&& LD_LIBRARY_PATH="/usr/local/lib64:$LD_LIBRARY_PATH" cmake \
		-DCMAKE_CXX_FLAGS_RELEASE='-fpermissive' \
		\
		-DWHICH_MYSQL_CLIENT=mariadbclient \
		-DCMAKE_BUILD_TYPE=Release \
		-DENABLE_STATIC=ON \
		-Dwangle_DIR=/bob-the-builder/wangle/ \
		-Dfolly_DIR=/bob-the-builder/folly/ \
		-Dfizz_DIR=/bob-the-builder/fizz/ \
		/tagem/wangle-server \
	&& ( \
		make server \
		|| (\
			/usr/local/bin/x86_64-linux-musl-g++  -static -fpermissive  -Wl,-Bstatic -s CMakeFiles/server.dir/src/server.cpp.o CMakeFiles/server.dir/src/FrameDecoder.cpp.o CMakeFiles/server.dir/src/qry.cpp.o  -o server  /usr/local/lib/mariadb/libmariadbclient.a /usr/local/lib/libwangle.a /usr/local/lib64/libcurl.a /usr/local/lib64/libffmpegthumbnailer.a /usr/local/lib/libfizz.a /usr/local/lib/libfolly.a /usr/local/lib64/libfmt.a /usr/local/lib64/liblz4.a /usr/local/lib64/libzstd.a /usr/local/lib64/libsnappy.a /usr/lib/libdwarf.a /usr/local/lib/libunwind.a /usr/local/lib/libsodium.a  /usr/local/lib64/libssl.a /usr/local/lib64/libcrypto.a /usr/local/lib64/libglog.a /usr/local/lib/libgflags.a /usr/local/lib/libevent.a /usr/local/lib64/libdouble-conversion.a -ldl /usr/lib/librt.a           /usr/local/lib/libavdevice.a                 /usr/local/lib/libavfilter.a  /usr/local/lib/libpostproc.a  /usr/local/lib/libavformat.a                /usr/local/lib/libavcodec.a  /usr/local/lib/libavutil.a           /usr/local/lib/libx264.a /usr/lib/libboost_context.a /tagem/wangle-server/docker/fix-missing-symbol.monkeypatch.cpp \
			&& strip --strip-all rscrape-cmnts \
		) \
	)

# Final compile command, if make server fails, was constructed from the final command that it would normally execute, with numerous consecutive spaces indicating that arguments (linked libraries) were removed from the final command (compared with the CMake generated command) - e.g. libboost*.so

FROM alpine:latest
COPY --from=intermediate /tagem/build/server /tagem-server
EXPOSE 80
ENTRYPOINT ["/tagem-server", "p", "80"]
