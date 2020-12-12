# Has to be in the root directory, otherwise the docker build system will not allow copying the necessary files from the host to the container
# All the other Dockerfiles are under wangle-server/docker

FROM notcompsky/static-wangle-ffmpeg:latest AS intermediate
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

ARG libmagic_version=5.39

RUN apk add --no-cache python3-dev=3.8.5-r0 \
	&& for d in /usr/lib/python3.*; do \
		cp $(find "$d" -type f -name '*python*.a') /usr/lib/ \
	; done \
	\
	&& curl -s ftp://ftp.astron.com/pub/file/file-${libmagic_version}.tar.gz | tar -xz \
	&& cd file-${libmagic_version} \
	&& ./configure \
		--enable-static \
		--disable-shared \
	&& ( \
		make && make install || ( \
			echo "Tries to build linked executable despite options" \
			&& mv src/.libs/libmagic.a /usr/local/lib/libmagic.a \
			&& mv src/magic.h /usr/local/include/magic.h \
		) \
	) \
	\
	&& git clone --depth 1 https://github.com/Tencent/rapidjson \
	&& mv rapidjson/include/rapidjson /usr/include/rapidjson \
	\
	&& git clone --depth 1 https://github.com/dirkvdb/ffmpegthumbnailer \
	&& cd ffmpegthumbnailer \
	&& git apply /ffmpegthumbnailer-static.patch \
	&& addlocalinclude() { \
		mv CMakeLists.txt CMakeLists.old.txt \
		&& echo 'include_directories("/usr/local/include" "/usr/include")' > CMakeLists.txt \
		&& cat CMakeLists.old.txt >> CMakeLists.txt \
		; \
	} \
	&& addlocalinclude \
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
	&& cd libcompsky \
	&& addlocalinclude \
	&& mkdir build \
	&& cd build \
	&& cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-DWHICH_MYSQL_CLIENT=mariadbclient \
		-DMYSQL_IS_UNDER_MARIADB_DIR=1 \
		-DMYSQL_UNDER_DIR_OVERRIDE=1 \
		.. \
	&& make install \
	\
	&& mv /usr/include/python3.8/* /usr/include/ \
	\
	&& chmod +x /tagem/wangle-server/scripts/* \
	&& ( \
		rm -rf /tagem/build \
		;  mkdir /tagem/build \
	) && cd /tagem/wangle-server \
	&& addlocalinclude \
	&& cd /tagem/build \
	&& LD_LIBRARY_PATH="/usr/local/lib64:$LD_LIBRARY_PATH" cmake \
		\
		-DWHICH_MYSQL_CLIENT=mariadbclient \
		-DCMAKE_BUILD_TYPE=Release \
		-DENABLE_STATIC=ON \
		-DEMBED_PYTHON=ON \
		/tagem/wangle-server \
	&& ( \
		make server \
		|| (\
			/usr/local/bin/x86_64-linux-musl-g++ -flto -static  -Wl,-Bstatic -s CMakeFiles/server.dir/src/server.cpp.o CMakeFiles/server.dir/src/qry.cpp.o CMakeFiles/server.dir/src/curl_utils.cpp.o CMakeFiles/server.dir/src/db_info.cpp.o CMakeFiles/server.dir/src/initialise_tagem_db.cpp.o  -o server  /usr/local/lib/mariadb/libmariadbclient.a /usr/local/lib64/libcurl.a /usr/local/lib64/libssl.a /usr/local/lib64/libcrypto.a /usr/local/lib64/libffmpegthumbnailer.a /usr/local/lib/libevent.a           /usr/local/lib/libavdevice.a                 /usr/local/lib/libavfilter.a  /usr/local/lib/libpostproc.a  /usr/local/lib/libavformat.a                /usr/local/lib/libavcodec.a  /usr/local/lib/libavutil.a     /usr/lib/python3.8/config-3.8-x86_64-linux-gnu/libpython3.8.a          /usr/local/lib/libx264.a /usr/lib/libboost_context.a /usr/local/lib/libx264.a /usr/lib/libboost_thread.a /tagem/wangle-server/docker/fix-missing-symbol.monkeypatch.cpp \
			&& strip --strip-all server \
		) \
	)

# Final compile command, if make server fails, was constructed from the final command that it would normally execute, with numerous consecutive spaces indicating that arguments (linked libraries) were removed from the final command (compared with the CMake generated command) - e.g. libboost*.so

FROM alpine:latest
COPY --from=intermediate /tagem/build/server /tagem-server
EXPOSE 80
ENTRYPOINT ["/tagem-server", "p", "80"]
