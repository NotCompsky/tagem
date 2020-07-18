# Description

Programs to rapidly categorise and access files, based on assignable attributes such as (heirarchical) tags, named variables, file sizes, hashes, and audio duration.

# Status

In active development.

Breaking changes are still sometimes made to the database, although breaking commits will detail how to upgrade.

The server is leading development at the moment, while the native app and most scripts are updated when I have the time.

## Web App

A neutered version of this app is hosted [here](https://notcompsky.github.io/tagem-eg/). It is not allowed to use POST requests, so most features are disabled. However, one can view the files and file table.

![Query3](https://user-images.githubusercontent.com/30552567/86843555-a000e380-c09e-11ea-9a0d-5a5e4ae38261.png)

## Native App

![Screenshot of tagemvid](https://i.imgur.com/OmgWJmk.png)
![Screenshot of tagemvid](https://i.imgur.com/rk7wynk.png)

# Installation

## Dependencies

### Server

* MySQL or MariaDB client
    * I recommend libmariadb
* All of the dependencies of Facebook's core C++ stack (gflags, glog, libsqlite3, etc.)

## Installation

## Configuration

### Optional

You'll probably want to add the [scripts](scripts/) directory to your `PATH` environmental variable, or perhaps just copy the scripts to `/usr/local/bin`.

The [Reddit userscript](browser-extensions/userscripts/reddit.js) can be added the usual way you add userscripts.

# Development

## Dependencies

### Web Server

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

The project started as a Python CLI text file 'utility hub' for rapid tagging, deleting and moving of text files, as a kind of ersatz note-taking system.

It merged with another Python project, a GUI utility for aiding the generation of computer vision datasets, using TKinter to allow the drawing and tagging of rectangles on images, using those rectangles to generate cropped subimages.

The two databases were upgraded into a single hierarchical tagging database.

To enable more control of the UI, I decided to move to Qt. This meant most code had to be rewritten, so I rewrote it all in C++.

It became my music playlist creator - I had a pipeline (in this repo) to quickly copy all music/videos to my smartphone, and to generate and transfer playlist files recognised by VLC, with playlists specified by intersections of tags and scores.

The server was developed because VLC on Android had an annoying issue which made playing all media files in a playlist as audio very difficult - the option to play a video file as audio had to either be accessed from the currently playing file's menu every time a new playlist was selected; and even if all media files were audio-only, VLC would attempt to play WEBMs as video first, which meant playlists would stop whenever a WEBM was encountered.

I created the server in order to avoid those issues. I kept on adding features because HTML5 and modern Javascript make it surprisingly easy to.

Some further features were transplants from yet more projects. For instance, the qry language is evolved from a shell script I used for querying large scraped databases. It was easier to write in C++ than in shell script - the biggest headache is now not trusting user input.

Since qry was ported over, I integrated it with several other projects that had previously had their own tagging systems, such as two extremely simple scrapers for [Twitter](https://github.com/NotCompsky/scrape-twitter) and [Reddit](scripts/record-reddit-post).

Going forwards, I expect [RScraper](https://github.com/NotCompsky/rscraper) will also be integrated into this project's tagging system.
