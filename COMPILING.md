## Docker

### Normal Build

This build uses shared binaries etc.

    docker build -t notcompsky/tagem --build-arg TAGEM_ROOT_DIR="$PWD" .
    docker run --env TAGEM_MYSQL_CFG="$TAGEM_MYSQL_CFG" -p 80:80 -v /media:/media -v /home:/home -v /var/run/mysqld/:/var/run/mysqld/ notcompsky/tagem

### Single Binary Build

Not entirely static yet.

This build uses a custom docker image where the entire dependency chain is in static libraries, rather than dynamically linked. The resulting binary can run on any Linux platform with the same type of CPU (e.g. x86 or x64).

Docker's build permissions means that one cannot `COPY` from the host unless the files are in the same context as the Dockerfile. Piping a Dockerfile from stdin, and using softlinks/hardlinks do not work around this. Dockerfiles cannot be named custom things.

Hence, in order to build the static (alpine/musl) single binary build, one must switch out [the main Dockerfile](Dockerfile) with [the musl Dockerfile](wangle-server/docker/musl/Dockerfile).

	mv Dockerfile Dockerfile.main
	mv wangle-server/docker/musl/Dockerfile Dockerfile
	docker build -t notcompsky/tagem-min .
	mv Dockerfile wangle-server/docker/musl/Dockerfile
	mv Dockerfile.main Dockerfile

To copy the binary from the image, you must create a dummy container and copy from that:

	docker create -ti --name dummy notcompsky/tagem-min bash
	docker cp dummy:/tagem/build/server ./tagem-server
	docker rm -f dummy

## Non-Docker

### Dependencies

#### Required

* Facebook's core C++ web stack: [wangle](https://github.com/facebook/wangle), [folly](https://github.com/facebook/folly) and [fizz](https://github.com/facebookincubator/fizz). I recommend building all of these by building [Proxygen](https://github.com/facebook/proxygen), for ease of installation.
* Python 3 interpreter
* CMake

#### Optional

* FFMPEG/libAV
    * Used for assigning video statistics (such as duration, fps, width, height).
* OpenCV and Caffe
    * Used for generating image databases for machine learning. Untested for a long time, will probably be fixed when 'instances' are recognised by the server.
