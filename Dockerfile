# Has to be in the root directory, otherwise the docker build system will not allow copying the necessary files from the host to the container
# All the other Dockerfiles are under wangle-server/docker

FROM notcompsky/tagem-compile-1 AS compile-2
WORKDIR /tagem
COPY wangle-server /tagem/wangle-server
COPY utils /tagem/utils
COPY caffe /tagem/caffe
COPY include /tagem/include
COPY scripts /tagem/scripts
# NOTE: libcompsky should be rebuilt every time, there is a reasonable chance that it is upgraded when tagem is
RUN git clone https://github.com/NotCompsky/libcompsky \
	&& mkdir libcompsky/build \
	&& cd libcompsky/build \
	&& cmake -DCMAKE_BUILD_TYPE=Release .. \
	&& make install \
	&& make \
	&& cd /tagem \
	&& mkdir /tagem/build \
	&& mkdir /tagem/build/webserver \
	&& mkdir /tagem/build/utils \
	&& cd /tagem/build/utils \
	&& cmake -DCMAKE_BUILD_TYPE=Release /tagem/utils \
	&& make \
	&& chmod +x /tagem/wangle-server/scripts/* \
	&& cd /tagem/build/webserver \
	&& cmake /tagem/wangle-server -DWHICH_MYSQL_CLIENT=mysqlclient -DCURL_INCLUDE_DIR=/usr/local/include -DCURL_LIBRARY=/usr/lib/x86_64-linux-gnu/libcurl.so -DCMAKE_BUILD_TYPE=Release -Dwangle_DIR=/bob-the-builder/wangle/ -Dfolly_DIR=/bob-the-builder/folly/ -Dfizz_DIR=/bob-the-builder/fizz/ \
	&& make server

FROM notcompsky/tagem-base
COPY --from=compile-2 /tagem/build/webserver/server /tagem-server
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
			dummy=1 \
		;else \
			/tagem-init \
		;fi \
	;fi \
	&& /tagem-server p 80
