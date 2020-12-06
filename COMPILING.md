## Docker

### Single Binary Build

This build uses a custom docker image where the entire dependency chain is in static libraries, rather than dynamically linked. The resulting binary can run on any Linux platform with the same type of CPU (e.g. x86 or x64).

	docker build -t notcompsky/tagem-min .

To copy the binary from the image, you must create a dummy container and copy from that:

	docker create -ti --name dummy notcompsky/tagem-min bash
	docker cp dummy:/tagem-server ./tagem-server
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
