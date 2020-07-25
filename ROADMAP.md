# Planned Features

* Automatic thumbnail generation. I know it's surprising it hasn't already been implemented, but so far I'm using remote thumbnails for remote files, and my OS's (desktop manager's, really) thumbnails for local files.
* User creation, assigning of tags to users' blocklists, etc. Web admin tasks that people might need if they want to have multiple users.
* Ability to upload files from clients to the server.
* TLS/SSL (HTTPS)
* Ability to edit text files from the browser
* Transferring the image dataset creation tools to the web client. 
* More features for `qry`
* The ability to view *all* websites within the app.
  This would allow the viewing of any webpage whatsoever, for instance Netflix videos, Twitter threads (as opposed to only individual Tweets), etc.
  The issue is that some sites ask the browser (via CORS settings) not to be displayed within `iframe`s. All that would need to be done would be add something functionally like [CORS Anywhere](https://github.com/Rob--W/cors-anywhere/) to the server.
* Tools for the rapid creation of computer vision datasets. The formerly-included GUI application used to allow one to cycle through images and draw rectangles in images, tag these rectangles, and use those tags to extract the rectangles into a caffe2 database. (e.g. selecting a human head in a photo and tagging it "Human Head"). This seems like it would be easy to add to the web page.

# Planned Distribution

Of course I would like to have this packaged up in popular package managers. But by far the most frustrating part of this project has been packaging it, so I'm not going to spearhead these packaging efforts.

A single portable binary would be nice to have, so I am making efforts here. Given the chain of dependencies in this project, that might take some time.

# Planned Coding Style

I cannot get a `clang-tidy` ruleset I am happy with, so it is not used. The issue is primarily with the inline SQL, and how that - appearing to `clang-tidy` as merely raw strings - needs to be inlined in a particular manner for easy reading.

