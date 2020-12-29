# Has to be in the root directory, otherwise the docker build system will not allow copying the necessary files from the host to the container

FROM notcompsky/amd64-static-mariadb-ffmpeg:latest AS intermediate
WORKDIR /tagem
# NOTE: libcompsky should be rebuilt every time, there is a reasonable chance that it is upgraded when tagem is
COPY ffmpegthumbnailer-static.patch /ffmpegthumbnailer-static.patch

ENV PATH=$PATH:/usr/local/x86_64-linux-musl/lib
ENV CC=/usr/local/bin/x86_64-linux-musl-gcc
ENV CXX=/usr/local/bin/x86_64-linux-musl-g++
ENV C_INCLUDE_PATH=/usr/local/x86_64-linux-musl/include
ENV LDFLAGS="-Wl,-Bstatic"
ENV CFLAGS="-static"
ENV CXXFLAGS="-static"

WORKDIR /tagem
COPY server /tagem/server
COPY utils /tagem/utils

ARG libmagic_version=5.39

RUN git clone --depth 1 https://github.com/lexbor/lexbor \
	\
	&& apk add --no-cache python3-dev \
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
	&& cd /tagem/lexbor \
	&& cmake \
		-DLEXBOR_BUILD_SHARED=OFF \
		-DLEXBOR_BUILD_STATIC=ON \
		-DLEXBOR_BUILD_TESTS=OFF \
		-DLEXBOR_BUILD_TESTS_CPP=OFF \
		-DLEXBOR_BUILD_UTILS=OFF \
		-DLEXBOR_BUILD_EXAMPLES=OFF \
		-DLEXBOR_BUILD_SEPARATELY=ON \
		. \
	&& make \
	&& make install \
	\
	&& mv /usr/include/python3.8/* /usr/include/ \
	\
	&& chmod +x /tagem/server/scripts/* \
	&& ( \
		rm -rf /tagem/build \
		;  mkdir /tagem/build \
	) && cd /tagem/server \
	&& addlocalinclude \
	&& cd /tagem/build \
	&& LD_LIBRARY_PATH="/usr/local/lib64:$LD_LIBRARY_PATH" cmake \
		\
		-DWHICH_MYSQL_CLIENT=mariadbclient \
		-DCMAKE_BUILD_TYPE=Release \
		-DENABLE_STATIC=ON \
		-DEMBED_PYTHON=ON \
		/tagem/server \
	&& make server

FROM alpine:latest
COPY --from=intermediate /tagem/build/server /tagem-server
EXPOSE 80
ENTRYPOINT ["/tagem-server", "p", "80"]
