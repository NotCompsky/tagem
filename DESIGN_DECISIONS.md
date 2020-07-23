# Outline

I've been creating this project since mid-2018, when I started to learn programming - the first version was written in mid-2018 while learning Python, the second version in mid-2019 as a C++ GUI app written while learning C++. This web version began in mid-2020. This project has evolved along a very jagged path, and consequently design decisions shouldn't be assumed to exist for a reason, but are in fact open to change, if there is a compelling reason.

If you want a detailed reasoning behind design decisions, [here's a blog](https://gist.github.com/NotCompsky/f1ab63fa2f191b156b9187b111449d20) detailing the various visions for this project.

# C++ Libraries

A core part of this project is the use of [libcompsky](/NotCompsky/libcompsky) for string concatenation and using MySQL. It is this project that is largely driving the development of libcompsky; even [rather bizarre uses](blob/638a4b2a798520859b2741ea4197104d143cfb9b/wangle-server/src/server.cpp#L449) can be [easily integrated into that library](/NotCompsky/libcompsky/blob/5da03a9743cfecc5ff9b594c25f67e1aa5b4071c/include/compsky/mysql/alternating_qry.hpp)'s template metaprogramming.

# Database

## Engine

One design decision that has remained is the use of MySQL/MariaDB.

SQLite has thus far been avoided because I believe allowing multiple concurrent connections to the database will continue to be useful - however, the goal is eventually to ship this server as a single binary, and that will probably utilise SQLite due to it being easy to embed. SQLite will probably never be the main DB engine, however, because allowing for simultaneous connections from multiple processes is always going to be important - this isn't just a web server, the design is meant to allow for writing to the database from multiple machines running various utilities (be they perceptual hashes of video files, object recognition on image files, sentiment analysis on linked web pages - the latter two being utilities that don't exist yet, just to be clear).

Sticking to MySQL itself - rather than PostgreSQL, for instance - is only because I have not seen significant reason to migrate. However, the decision to support *older* versions of the DB engine is deliberate, to support the surprisingly outdated Google Cloud Compute Engine instances.
