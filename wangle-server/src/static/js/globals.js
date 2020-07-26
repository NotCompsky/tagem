const $$$MAX_RESULTS_PER_PAGE = 100;

const $$$YOUTUBE_DEVICE_ID = "1";

var $$$currently_viewing_object_type;

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
var t2p;
var x;
var mt; // mimetypes
var f2; // CSV of file2 variable names that needs to be split into an array
var $$$f2_as_array;
var $$$yt_player;

// User options stored in cookies
var $$$use_regex;

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
