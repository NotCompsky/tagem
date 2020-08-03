FROM ubuntu:20.04 AS base-img
ENV TZ=Europe/London
RUN apt-get update \
	&& apt-get install -y --no-install-recommends ca-certificates \
	&& update-ca-certificates \
	\
	&& apt install -y --no-install-recommends libsodium23 libboost-context1.71.0 libevent-2.1-7 libdouble-conversion3 \
	\
	&& apt install -y --no-install-recommends libunwind8 libgoogle-glog0v5 \
	\
	&& apt install -y --no-install-recommends default-mysql-client default-libmysqlclient-dev
