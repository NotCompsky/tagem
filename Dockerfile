# Has to be in the root directory, otherwise the docker build system will not allow copying the necessary files from the host to the container
# All the other Dockerfiles are under wangle-server/docker

FROM notcompsky/tagem-compile-1 AS compile-2
#ADD "https://www.random.org/cgi-bin/randbyte?nbytes=10&format=h" skipcache
# The above effectively disables the hash
RUN git clone https://github.com/NotCompsky/libcompsky \
	&& mkdir libcompsky/build \
	&& cd libcompsky/build \
	&& cmake -DCMAKE_BUILD_TYPE=Release .. \
	&& make install
WORKDIR /tagem
COPY wangle-server /tagem/wangle-server
COPY utils /tagem/utils
COPY caffe /tagem/caffe
COPY include /tagem/include
COPY scripts /tagem/scripts
# cimg-dev pulls in about a billion dependencies, so we'll just clone the repository to get the headers
RUN apt install -y --no-install-recommends libavformat-dev libavcodec-dev \
	&& cd /tagem \
	&& git clone https://github.com/dtschump/CImg \
	&& mkdir /tagem/build \
	&& mkdir /tagem/build/server \
	&& mkdir /tagem/build/utils \
	&& cd /tagem/build/utils \
	&& cmake -DCMAKE_BUILD_TYPE=Release -DCIMG_H_DIR=/tagem/CImg /tagem/utils \
	&& make \
	&& cd /tagem/build/server \
	&& cmake /tagem/wangle-server \
	&& make \
	&& chmod +x /tagem/wangle-server/scripts/* \
	&& apt-get install -y --no-install-recommends libcurl4-openssl-dev \
	&& cd /tagem/build \
	&& cmake /tagem/wangle-server -DWHICH_MYSQL_CLIENT=mysqlclient -DCURL_INCLUDE_DIR=/usr/local/include -DCURL_LIBRARY=/usr/lib/x86_64-linux-gnu/libcurl.so -DCMAKE_BUILD_TYPE=Release -Dwangle_DIR=/bob-the-builder/wangle/ -Dfolly_DIR=/bob-the-builder/folly/ -Dfizz_DIR=/bob-the-builder/fizz/ \
	&& make server

FROM notcompsky/tagem-base
COPY --from=compile-2 /tagem/build/server/server /tagem-server
COPY --from=compile-2 /tagem/build/utils/tagem-init /tagem-init
RUN apt purge -y libc-dev-bin libssl-dev linux-libc-dev libcrypt-dev \
	&& rm -rf \
		/var/lib/apt/lists/* /var/cache/apt/lists/* \
		/usr/sbin/* \
		/usr/lib/x86_64-linux-gnu/*.a \
		/usr/include/* \
		/usr/share/doc/*

EXPOSE 80
CMD if [ "$TAGEM_MYSQL_CFG" = "" ]; then \
		if [ -f /tagem-auth.cfg ]; then \
			export TAGEM_MYSQL_CFG=/tagem-auth.cfg \
		else \
			/tagem-init \
		fi \
	fi \
	&& /tagem-server p 80
