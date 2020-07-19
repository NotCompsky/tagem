FROM ubuntu:20.04 AS base-img
ENV TZ=Europe/London
RUN apt-get update \
	&& apt-get install -y --no-install-recommends ca-certificates curl \
	&& update-ca-certificates
# Dependencies of Facebook stack
RUN apt install -y --no-install-recommends libsodium23 libboost-context1.71.0 libevent-2.1-7 libdouble-conversion3
# Dependencies that seem to me like they could be stripped from the Facebook stack (for releases)
RUN apt install -y --no-install-recommends libunwind8 libgoogle-glog0v5
# Direct dependencies
RUN apt install -y --no-install-recommends default-mysql-client default-libmysqlclient-dev
