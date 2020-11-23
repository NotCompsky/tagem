// Copyright 2020 Adam Gray
// This file is part of the tagem program.
// The tagem program is free software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published by the
// Free Software Foundation version 3 of the License.
// The tagem program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// This copyright notice should be included in any copy or substantial copy of the tagem source code.
// The absense of this copyright notices on some other files in this project does not indicate that those files do not also fall under this license, unless they have a different license written at the top of the file.

const $$$MAX_RESULTS_PER_PAGE = 100;

const $$$YOUTUBE_DEVICE_ID = "1";

var $$$currently_viewing_object_type;

var $$$mouse_is_down = false;

var $$$tag_id;
var $$$file_id;
var $$$file_name;
var $$$dir_id;
var $$$dir_name;
var $$$device_id;
var $$$mimetype;
var $$$file_tags;

// External database vars
var $$$db_id;
var $$$post_id;
var $$$user_id;
var $$$user_name;

// Don't mangle these names
var t;
var f;
var d;
var D;
var P;
var x;
var mt; // mimetypes
var f2; // CSV of file2 variable names that needs to be split into an array
var $$$f2_as_array;
var $$$yt_player;

// User options stored in cookies
var $$$use_regex;
var $$$prioritise_local_autoplay;

const $$$stylesheet_opts = ["blocks","table"];

const $$$tbl2selector = {"f2":"#file2-select","t":".tagselect","d":"#dirselect","D":"#deviceselect","P":"#protocolselect","$$$users_dict":".userselect"};

const $$$tbl2namecol = {"f2":0,"t":0,"d":0,"D":0,"P":0,"$$$users_dict":0};

const $$$BLANK_IMG_SRC = "data:,";

var $$$get_file_ids;
var $$$get_tag_ids;
var $$$get_dir_ids;
var $$$file_tagger_fn; // Allows for updating file tag lists when they have tags manually added
var $$$dir_tagger_fn;


// Admin variables
var $$$users_dict; // Only really used by functions in this file, but *referred to* - for initialisation of select2 selects - in document-on-ready.js and here

// fake async
var $$$tagselect_popup_fn; // Function that is called after user closes the tagselect popup
