var $$$tag_id;
var $$$file_id;
var $$$file_name;
var $$$dir_id;
var $$$dir_name;
var $$$device_id;
var $$$mimetype;
var $$$file_tags;
var $$$file2post;

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
var $$$yt_player;

// User options stored in cookies
var $$$use_regex;

const $$$tbl2selector = {"f2":"#file2-select","t":".tagselect","d":"#dirselect","D":"#deviceselect","P":"#protocolselect"};

const $$$tbl2namecol = {"f2":null,"t":0,"d":0,"D":0,"P":0};

var $$$get_file_ids;
var $$$file_tagger_fn; // Allows for updating file tag lists when they have tags manually added
