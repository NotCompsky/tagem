# General

This project is statically compiled within a Docker container - hence it's tiny size.

I will release the executable - which can be run directly - with every major release, however to get the very latest release, you will have to either use the Docker image, or extract the executable from that Docker image.

# Docker Installation

## Description

This sets up both the web server on a docker container on the machine these commands are being executed on.

Note that the docker image does not also host the MySQL/MariaDB server, as the web server updates far more frequently than the database.

## Installation

	docker pull notcompsky/tagem

## Configuration

The container needs to know how to connect to the database, and for that we use a config file.

**NOTE**: If the MySQL server is on your local machine, and your machine is not running Windows, use `localhost` as the host name, and leave the port number blank. MySQL is weird about connecting to localhost - as an optimisation, it overrides connection settings when it thinks a UNIX socket might be available, and this somehow makes it crash when a port number is assigned to localhost.

### Recommended Method

`CREATE` the database and `GRANT SELECT, INSERT, UPDATE, DELETE, EXECUTE, ALTER, CREATE, TRIGGER` permissions to a user. You can revoke the `ALTER, CREATE, TRIGGER` permissions once the server is up and running.

Edit [example.cfg](example.cfg) to be correct for your use case, and pass its location to the container using the command line options: `--env TAGEM_MYSQL_CFG=/path/to/edited.cfg` - ensuring that the container has access to the directory `/path/to` (achieved with the options: `-v /path/to:/path/to`.

### Alternative Method: Inbuilt Config Generator

If the environmental variable `TAGEM_MYSQL_CFG` is not set, when the docker contaienr is run, a utility will run which requires some manual input to set up the MySQL server.

The options you should use are:

* Absolute path to save the config file to: `/tagem-auth.cfg`
* Host: **Domain name or IP address of the MySQL/MariaDB server machine. Even if the MySQL server is on the same machine as the docker container, it will NOT be localhost unless you run the docker container with `--network="host"` - instead, it would be the loopback address.**
* Socket file path: **Leave blank**
* Username: **This option is up to you**
* Password: **This option is up to you**
* Database name: `tagem`
* MySQL Server port number: **Remote server's port number**

At this point, the configuration file has been writte, but the database has not been set up by this utility. If the database already exists, you can quit this script now. Otherwise, assuming the remote database allows foreign root access, you can set the database up by continuing with this script:

* MySQL admin username: **Remote server's admin username**
* MySQL admin password: **Remote server's admin password**

## Run

	docker run -p 80:80 [[OPTIONS]] notcompsky/tagem

where `[[OPTIONS]]` is a list of directories you want the server to be able to access, e.g. `-v /path/to/directory`

**The first time it is run, the `--interactive` option should be used**.


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
