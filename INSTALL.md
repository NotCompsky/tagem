# Docker Installation on Single Machine

## Description

This sets up both the web server and the MySQL server on a docker container on the machine these commands are being executed on.

## Installation

	docker pull notcompsky/tagem

## Configuration

The first time the server is run, a utility will run which requires some manual input to set up the MySQL server.

The options you should use are:

* Absolute path to save the config file to: `/tagem-auth.cfg`
* Host: `localhost`
* Socket file path: `/var/run/mysqld/mysqld.sock`
* Username: **This option is up to you**
* Password: **This option is up to you**
* Database name: `tagem`
* MySQL Server port number: **Leave blank**
* MySQL admin username: `root`
* MySQL admin password: **Leave blank**

## Run

    docker run -p 80:80 [[OPTIONS]] notcompsky/tagem

where `[[OPTIONS]]` is a list of directories you want the server to be able to access, e.g. `-v /path/to/directory`


# Docker Installation using Remote Database

This sets up the web server on a docker container on a different machine to the database.

## Installation

Same as for the [Basic Docker Installation](#Docker%20Installation%20on%20Single%20Machine)

## Configuration

The first time the server is run, a utility will run which requires some manual input to set up the MySQL server.

The options you should use are:

* Absolute path to save the config file to: `/tagem-auth.cfg`
* Host: **Domain name or IP address of the MySQL/MariaDB server machine**
* Socket file path: **Leave blank**
* Username: **This option is up to you**
* Password: **This option is up to you**
* Database name: `tagem`
* MySQL Server port number: **Remote server's port number**

At this point, the configuration file has been writte, but the database has not been set up by this utility. If the database already exists, you can quit this script now. Otherwise, assuming the remote database allows foreign root access, you can set the database up by continuing with this script:

* MySQL admin username: **Remote server's admin username**
* MySQL admin password: **Remote server's admin password**

## Run

Same as for the [Basic Docker Installation](#Docker%20Installation%20on%20Single%20Machine)


# Bare Metal Install with Shared Libraries

## Dependencies

The easiest method is to build [Proxygen](https://github.com/facebook/proxygen) (just to automatically build/install all its dependencies, building proxygen itself isn't important).

On Ubuntu 18.04, the other dependencies can be installed with:

	sudo apt install -y --no-install-recommends default-mysql-client default-libmysqlclient-dev libsodium23 libboost-context1.71.0 libevent-2.1-7 libdouble-conversion3 libunwind8 libgoogle-glog0v5 

## Installation

Copy a provided binary and hope it was linked against all the same libraries as on your machine.
Or compile the project, as described in [COMPILING.md](COMPILING.md).

## Configuration

Almost the same as for Docker installs, except the initialisation utility must be run directly:

	tagem-init
