Platform | CI Status
---------|----------
OSX      | [![OSX Build Status](http://badges.herokuapp.com/travis/NotCompsky/tagem?env=BADGE=osx&label=build&branch=master)](https://travis-ci.org/NotCompsky/tagem)
Linux    | [![Linux Build Status](http://badges.herokuapp.com/travis/NotCompsky/tagem?env=BADGE=linux&label=build&branch=master)](https://travis-ci.org/NotCompsky/tagem) [![CircleCI](https://circleci.com/gh/NotCompsky/tagem.svg?style=shield)](https://circleci.com/gh/NotCompsky/tagem)

# Description

Programs to rapidly categorise and access files, based on assignable attributes such as (heirarchical) tags, named variables, file sizes, hashes, and audio duration.

# Status

In active development.

# Demonstration

A neutered version of this app is hosted [here](https://notcompsky.github.io/tagem-eg/). GitHub does not allow it to be interactive, so most features are disabled.

![Query3](https://user-images.githubusercontent.com/30552567/86843555-a000e380-c09e-11ea-9a0d-5a5e4ae38261.png)

# Installation

## Docker Image

* Docker

## Native

### Dependencies

* MySQL or MariaDB client
    sudo apt install -y --no-install-recommends default-mysql-client default-libmysqlclient-dev
* All of the dependencies of Facebook's core C++ web stack
    sudo apt install -y --no-install-recommends libsodium23 libboost-context1.71.0 libevent-2.1-7 libdouble-conversion3 libunwind8 libgoogle-glog0v5 

## Installation

## Configuration

### Mandatory

You must set up a MySQL/MariaDB database for this project, using the commands [here](utils/src/init.sql).



### Optional

You'll probably want to add the [scripts](scripts/) directory to your `PATH` environmental variable, or perhaps just copy the scripts to `/usr/local/bin`.

The [Reddit userscript](browser-extensions/userscripts/reddit.js) can be added the usual way you add userscripts.

# Development

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

# Design Decisions

This project has evolved along a very jagged path. Many design decisions made in the past have had to be significantly altered.

A core part of this project is the use of [libcompsky](https://github.com/NotCompsky/libcompsky) for string concatenation and using MySQL. It is this project that is largely driving the development of libcompsky; even [rather bizarre uses]() can be [easily integrated into that library](https://github.com/NotCompsky/libcompsky/blob/5da03a9743cfecc5ff9b594c25f67e1aa5b4071c/include/compsky/mysql/alternating_qry.hpp)'s template metaprogramming.

One design decision that has remained is the use of MySQL/MariaDB. SQLite3 is avoided because I believe allowing multiple concurrent connections to the database will continue to be useful. However, sticking to MySQL itself is only because I have not seen significant reason to migrate; however, the decision to support older versions of MySQL is deliberate, to support the surprisingly outdated Google Cloud Compute Engine instances.

# Background

If you feel like there aren't enough blogs on the internet, [here's another](https://gist.github.com/NotCompsky/f1ab63fa2f191b156b9187b111449d20). It's a look at how this project evolved from some unlikely decisions, as I'm personally interested in how [the Butterfly Effect](https://en.wikipedia.org/wiki/Butterfly_effect) occurs in software development.
