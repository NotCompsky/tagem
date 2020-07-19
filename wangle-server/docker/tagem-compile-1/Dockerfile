FROM notcompsky/tagem-base:0.0.1
ARG DEBIAN_FRONTEND=noninteractive
# Compile-time env var to tell apt to avoid asking for inputs
RUN apt-get update \
	&& apt-get install -y --no-install-recommends git g++ cmake libboost-all-dev libevent-dev libdouble-conversion-dev libgoogle-glog-dev libgflags-dev libiberty-dev liblz4-dev liblzma-dev libsnappy-dev make zlib1g-dev binutils-dev libjemalloc-dev libssl-dev pkg-config libunwind-dev libsodium-dev

WORKDIR /bob-the-builder
RUN git clone https://github.com/fmtlib/fmt.git \
	&& cd fmt \
	&& mkdir _build \
	&& cd _build \
	&& cmake .. -DCMAKE_BUILD_TYPE=Release -DFMT_TEST=OFF -DFMT_DOC=OFF \
	&& make \
	&& make install
RUN git clone https://github.com/google/double-conversion.git \
	&& cd double-conversion \
	&& cmake -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release . \
	&& make \
	&& make install
RUN git clone https://github.com/facebook/folly.git \
	&& cd folly \
	&& mkdir _build \
	&& cd _build \
	&& cmake .. -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release \
	&& make \
	&& make install

# Install fizz, using their instructions
RUN git clone https://github.com/facebookincubator/fizz \
	&& mkdir fizz/build_ \
	&& cd fizz/build_ \
	&& cmake ../fizz -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release \
	&& make \
	&& make install

# Install wangle
RUN git clone https://github.com/facebook/wangle \
	&& cd wangle/wangle \
	&& cmake . \
	&& make \
	&& make install

RUN apt install -y --no-install-recommends default-mysql-client default-libmysqlclient-dev
