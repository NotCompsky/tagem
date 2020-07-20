R"=====(

CREATE TABLE IF NOT EXISTS user (
	id INT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
	name VARBINARY(100) NOT NULL UNIQUE KEY
);
INSERT INTO user
(id,name)
VALUES
(1,"Guest"), -- Necessary user (name does not matter)
(2,"Admin"),
(99,"Invalid")
ON DUPLICATE KEY UPDATE id=id;
UPDATE user SET id=0 WHERE name="Invalid";

CREATE TABLE IF NOT EXISTS protocol (
	id INT UNSIGNED NOT NULL PRIMARY KEY,
	name VARBINARY(16) NOT NULL UNIQUE KEY
);
INSERT INTO protocol (id, name) VALUES
(0, "NONE!"),
(1, "file://"),
(2, "http://"),
(3, "https://"),
(4, "youtube-dl")
ON DUPLICATE KEY UPDATE id=id;

CREATE TABLE IF NOT EXISTS device (
	-- Storage device - such as a hard drive, or a website (a remote storage device)
	-- Name is the prefix - allowing 'youtube-dl' protocol for 'https://youtube.com/watch?v=' and 'https' protocol for 'https://youtube.com/user/' prefixes
	id BIGINT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
	user INT UNSIGNED NOT NULL,
	name VARBINARY(1024) NOT NULL UNIQUE KEY,
	protocol INT UNSIGNED NOT NULL,
	embed_pre  VARBINARY(1000) NOT NULL DEFAULT "",
	embed_post VARBINARY(1000) NOT NULL DEFAULT "",
	FOREIGN KEY (user) REFERENCES user (id),
	FOREIGN KEY (protocol) REFERENCES protocol (id)
);
INSERT INTO device (name,permissions,protocol,embed_pre,embed_post) VALUES
("https://youtube.com/watch?v=",0,(SELECT id FROM protocol WHERE name='youtube-dl'), 'https://www.youtube.com/embed/', '?enablejsapi=1'),
("https://twitter.com/",0,(SELECT id FROM protocol WHERE name='https://'), '<blockquote class="twitter-tweet"><a href="https://twitter.com/AnyUsernameWorksHere/status/', '?ref_src=twsrc%5Etfw">Link</a></blockquote><script async src="https://platform.twitter.com/widgets.js" charset="utf-8"></script>')
ON DUPLICATE KEY UPDATE name=name;
-- WARNING: The device IDs are assumed in the scripts, so these must be inserted in this order even if they are unused.

INSERT INTO device (name, protocol) VALUES
("https://www.google.com/", (SELECT id FROM protocol WHERE name="https://")),
("https://stackoverflow.com/", (SELECT id FROM protocol WHERE name="https://")),
("https://en.wikipedia.org/", (SELECT id FROM protocol WHERE name="https://")),
("https://github.com/", (SELECT id FROM protocol WHERE name="https://"))
ON DUPLICATE KEY UPDATE protocol=protocol;

CREATE TABLE IF NOT EXISTS dir (
	id BIGINT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
	parent BIGINT UNSIGNED,
	parent_not_null BIGINT AS (IFNULL(parent,0)), -- Allows foreign key checks to work
	device BIGINT UNSIGNED NOT NULL,
	user INT UNSIGNED NOT NULL,
	name VARBINARY(255) NOT NULL,
	full_path VARBINARY(4096) NOT NULL,
	FOREIGN KEY (parent) REFERENCES dir (id),
	FOREIGN KEY (user) REFERENCES user (id),
	FOREIGN KEY (device) REFERENCES device (id),
	UNIQUE KEY (parent_not_null,name), -- Only one name if parent is NULL
	--UNIQUE KEY (full_path), -- Field is too long to be a key
	UNIQUE KEY (parent,name)
);

CREATE TABLE IF NOT EXISTS dir2parent_tree (
	dir BIGINT UNSIGNED NOT NULL,
	parent BIGINT UNSIGNED NOT NULL,
	depth INT UNSIGNED NOT NULL,
	FOREIGN KEY (dir) REFERENCES dir (id),
	FOREIGN KEY (parent) REFERENCES dir (id),
	UNIQUE KEY (dir,parent)
);

CREATE TABLE IF NOT EXISTS dir2tag (
	dir BIGINT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	user INT UNSIGNED NOT NULL,
	FOREIGN KEY (dir) REFERENCES dir (id),
	FOREIGN KEY (tag) REFERENCES tag (id),
	FOREIGN KEY (user) REFERENCES user (id),
	PRIMARY KEY (dir,tag)
);
CREATE TABLE IF NOT EXISTS device2tag (
	device BIGINT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	user INT UNSIGNED NOT NULL,
	FOREIGN KEY (device) REFERENCES device (id),
	FOREIGN KEY (tag) REFERENCES tag (id),
	FOREIGN KEY (user) REFERENCES user (id),
	PRIMARY KEY (device,tag)
);

CREATE TABLE IF NOT EXISTS mimetype (
	id INT UNSIGNED NOT NULL PRIMARY KEY,
	name VARBINARY(32) NOT NULL UNIQUE KEY
);
SET @i := -1;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"!!NONE!!") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"audio/aac") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"image/bmp") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"text/css") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"text/csv") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"image/gif") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"text/html") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"image/jpeg") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"textjavascript") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"application/json") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"audio/mpeg") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"video/mpeg") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"audio/ogg") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"video/ogg") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"audio/opus") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"image/png") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"image/tiff") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"text/plain") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"audio/wav") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"audio/webm") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"video/webm") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"image/webp") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"video/avi") ON DUPLICATE KEY UPDATE id=id;

CREATE TABLE IF NOT EXISTS ext2mimetype (
	name VARBINARY(10) NOT NULL PRIMARY KEY,
	mimetype INT UNSIGNED NOT NULL,
	FOREIGN KEY (mimetype) REFERENCES mimetype (id)
);
INSERT INTO ext2mimetype
(ext,mimetype)
SELECT REGEXP_REPLACE(SUBSTRING_INDEX(name, '/', -1), '^x-', ''), id
FROM mimetype
WHERE SUBSTRING_INDEX(name, '/', -1) NOT IN ("plain","mpeg","webm","javascript","!!NONE!!","quicktime","x-ms-asf")
  AND name NOT IN ("video/ogg")
UNION
SELECT "mp4", id FROM mimetype WHERE name='video/mpeg'
UNION
SELECT "mp3", id FROM mimetype WHERE name='audio/mpeg'
UNION
SELECT "jpg", id FROM mimetype WHERE name='image/jpeg'
UNION
SELECT "webm", id FROM mimetype WHERE name='video/webm'
ON DUPLICATE KEY UPDATE mimetype=mimetype
;


CREATE TABLE IF NOT EXISTS file (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
	dir BIGINT UNSIGNED NOT NULL,
	size BIGINT UNSIGNED,
	duration BIGINT UNSIGNED,
	sha256 BINARY(32),
	md5 BINARY(16),
	md5_of_path BINARY(16),
    name VARBINARY(1024),
    added_on DATETIME DEFAULT CURRENT_TIMESTAMP,
	t_origin BIGINT DEFAULT 0,
	mimetype INT UNSIGNED NOT NULL DEFAULT 0,
	user INT UNSIGNED NOT NULL,
	w INT UNSIGNED,
	h INT UNSIGNED,
	views INT UNSIGNED,
	likes INT UNSIGNED,
	dislikes INT UNSIGNED,
	fps FLOAT,
	title VARCHAR(100),
	description VARCHAR(1000),
	status INT UNSIGNED NOT NULL DEFAULT 0, -- 0 is available, 1 is deleted, 2 is private
    UNIQUE KEY (dir, name),
	FOREIGN KEY (dir) REFERENCES file (id),
	FOREIGN KEY (mimetype) REFERENCES mimetype (id),
	FOREIGN KEY (user) REFERENCES user (id),
    PRIMARY KEY (id)
);

-- Can get dir name from full file path: SELECT SUBSTR(name, 1, LENGTH(name) - LOCATE('/',REVERSE(name))) FROM file


CREATE TABLE IF NOT EXISTS file_backup (
	-- WARNING: Not sure how best to deal with the fact that some remote 'files' have different options, e.g. video format
	-- NOTE: A backup might still be remote too. For instance, web.archive.org.
	file BIGINT UNSIGNED NOT NULL,
	dir BIGINT UNSIGNED NOT NULL,
	name VARBINARY(1024) NOT NULL,
	mimetype INT UNSIGNED NOT NULL,
	user INT UNSIGNED NOT NULL,
	PRIMARY KEY (dir, name),
	FOREIGN KEY (dir) REFERENCES file (id),
	FOREIGN KEY (file) REFERENCES file (id),
	FOREIGN KEY (mimetype) REFERENCES mimetype (id),
	FOREIGN KEY (user) REFERENCES user (id)
);

 
CREATE TABLE IF NOT EXISTS file2 (
	-- Stores the user-defined variable tables
	id INT UNSIGNED NOT NULL AUTO_INCREMENT,
	min BIGINT NOT NULL,
	max BIGINT NOT NULL,
	-- NOTE: min and max are only relevant for GUI input. File sizes above 4GB would be impossible to input via Qt5 getInt method, but would be otherwise supported if the int type used is BIGINT or BIGINT UNSIGNED.
	conversion INT UNSIGNED NOT NULL, -- 0 if no conversion (i.e. the end data type is integer), otherwise the integer is not input directly by a human but relates to something else like a string
	name VARBINARY(128),
	UNIQUE KEY (name),
	PRIMARY KEY (id)
);
INSERT INTO file2
(id,min, max, conversion, name)
VALUES
(1,0, 100, 0, "Score")
ON DUPLICATE KEY UPDATE id=id;

CREATE TABLE IF NOT EXISTS user2shown_file2 (
	user INT UNSIGNED NOT NULL,
	file2 INT UNSIGNED NOT NULL,
	FOREIGN KEY (file2) REFERENCES file2 (id),
	PRIMARY KEY (user,file2)
);
INSERT INTO user2shown_file2
(user,file2)
SELECT id, 1
FROM user
ON DUPLICATE KEY UPDATE user=user;


CREATE TABLE IF NOT EXISTS tag (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
	user INT UNSIGNED NOT NULL,
    name VARBINARY(128),
	thumbnail VARBINARY(200),
	cover VARBINARY(200) NOT NULL DEFAULT "",
    UNIQUE KEY (name),
	FOREIGN KEY (user) REFERENCES user (id),
    PRIMARY KEY (id)
);
INSERT INTO tag (id,name) VALUES (0,"!!ROOT TAG!!") ON DUPLICATE KEY UPDATE id=id;
UPDATE tag SET id=0 WHERE name="!!ROOT TAG!!";
-- NOTE: Permissions are AND (each non-zero bit is another required permission)
-- NOTE: A permission of 0 allows everyone to see, since the permission mask is applied as if(f.permission & u.permission == f.permission)

CREATE TABLE IF NOT EXISTS tag2parent (
	tag BIGINT UNSIGNED NOT NULL,
	parent BIGINT UNSIGNED NOT NULL,
	user INT UNSIGNED NOT NULL,
	FOREIGN KEY (tag) REFERENCES tag (id),
	FOREIGN KEY (parent) REFERENCES tag (id),
	FOREIGN KEY (user) REFERENCES user (id),
	PRIMARY KEY (tag, parent)
);
CREATE TABLE IF NOT EXISTS tag2parent_tree (
	tag BIGINT UNSIGNED NOT NULL,
	parent BIGINT UNSIGNED NOT NULL,
	depth INT UNSIGNED NOT NULL,
	FOREIGN KEY (tag) REFERENCES tag (id),
	FOREIGN KEY (parent) REFERENCES tag (id),
	PRIMARY KEY (tag, parent)
);

CREATE TABLE IF NOT EXISTS user2blacklist_tag (
	-- Anything tagged with any of these tags are invisible to the user
	user INT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	FOREIGN KEY (user) REFERENCES user (id),
	FOREIGN KEY (tag) REFERENCES tag (id),
	PRIMARY KEY (user,tag)
);

CREATE TABLE IF NOT EXISTS user2hidden_tag (
	-- The tags are invisible to the user
	-- Only stops tags from appearing to the client. Does not, for instance, filter out qry results.
	user INT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	max_depth INT UNSIGNED DEFAULT TRUE,
	FOREIGN KEY (user) REFERENCES user (id),
	FOREIGN KEY (tag) REFERENCES tag (id),
	PRIMARY KEY (user,tag)
);


CREATE TABLE IF NOT EXISTS file2tag (
	file BIGINT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	user INT UNSIGNED NOT NULL,
	FOREIGN KEY (file) REFERENCES file (id),
	FOREIGN KEY (tag) REFERENCES tag (id),
	FOREIGN KEY (user) REFERENCES user (id),
	PRIMARY KEY (file, tag)
);

CREATE TABLE IF NOT EXISTS task (
	id INT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
	name VARBINARY(100) NOT NULL UNIQUE KEY,
	description VARCHAR(10000) NOT NULL,
	content VARCHAR(10000) NOT NULL
);
CREATE TABLE IF NOT EXISTS user2authorised_task (
	user INT UNSIGNED NOT NULL,
	task INT UNSIGNED NOT NULL,
	FOREIGN KEY (user) REFERENCES user (id),
	FOREIGN KEY (task) REFERENCES task (id),
	PRIMARY KEY (user, task)
);

CREATE TABLE IF NOT EXISTS file2dct_hash (
	file BIGINT UNSIGNED NOT NULL,
	x BIGINT UNSIGNED NOT NULL,
	PRIMARY KEY (file, x)
);

CREATE TABLE IF NOT EXISTS file2audio_hash (
	file BIGINT UNSIGNED NOT NULL,
	x BIGINT UNSIGNED NOT NULL,
	PRIMARY KEY (file, x)
);

CREATE TABLE IF NOT EXISTS box (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    file BIGINT UNSIGNED NOT NULL,
    frame_n BIGINT UNSIGNED NOT NULL,
    x DOUBLE NOT NULL,  -- As proportion of (image|frame) width
    y DOUBLE NOT NULL,
    w DOUBLE NOT NULL,
    h DOUBLE NOT NULL,
    PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS box2tag (
	box BIGINT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	PRIMARY KEY (box, tag)
);


CREATE TABLE IF NOT EXISTS method (
	id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
	name VARBINARY(128),
	UNIQUE KEY (name),
	PRIMARY KEY (id)
);
INSERT IGNORE INTO method (name) VALUES
("next_subtitle"),
("wipe_subtitle"),
("skip"),
("menu"),
("python_script")
ON DUPLICATE KEY UPDATE name=name;

CREATE TABLE IF NOT EXISTS era (
	id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
	file BIGINT UNSIGNED NOT NULL,
	start DOUBLE NOT NULL,
	end DOUBLE NOT NULL,
	UNIQUE KEY (file, start, end)
);

CREATE TABLE IF NOT EXISTS era2tag (
	era BIGINT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	PRIMARY KEY (era, tag)
);



-- The following tables populate the tags of relationships, not boxes.
-- For instance, a man typing on a keyboard would be represented by (a box tagged "man") related to (a box tagged "keyboard") via (a relationship tagged "typing").
-- relationtag2tag generates extra relationship tags based on the master and slave box tags. For instance, if the keyboard is a "mechanical" keyboard, it could generate another tag ("man typing on mechanical keyboard") for the relationship.

CREATE TABLE IF NOT EXISTS relation (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    master BIGINT UNSIGNED NOT NULL,  -- ID of master box
    slave BIGINT UNSIGNED NOT NULL,
    PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS relation2tag (
    relation BIGINT UNSIGNED NOT NULL,
    tag BIGINT UNSIGNED NOT NULL,
    PRIMARY KEY (relation, tag)
);



-- For instance, suppose there is (a box tagged "human" and "male") which is related to (a box tagged "arm") via (the "body part" tag). We would like the arm to inherit (the "human" and "male" tags).
-- The following tables describe the rules. Each rule is of the form (ID,  a = list of master tag IDs,  b = list of slave tag IDs,  c = list of resulting master tags,  d =list of resulting slave tags).
-- If a and b hold for a given relation (AND rather than OR - i.e. each tag ID in a must be present in the master box), then all tags of c are added to the master box, and all tags of d are added to the slave box.
-- NOTE: 0 is a special 'tag ID'. No tags have an ID of 0. Instead, it means the same 'tag ID' as the triggering tags - effectively propagating the tags to the related box. For instance, (a "body part" box) related to (a "human" box) via a ("body part" relation) (where "human" is the master and "body part" the slave) would propagate the "human" tag to the "body part" box, if the 'res_slave' tag ID was 0. Another use of this may be in a relation designating two things as "identical".

CREATE TABLE IF NOT EXISTS relation_add_box_tags__rules (
	id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
	name VARBINARY(128) NOT NULL,
	req_relation_operator INT UNSIGNED NOT NULL DEFAULT 0,
	req_master_operator INT UNSIGNED NOT NULL DEFAULT 0,
	req_slave_operator INT UNSIGNED NOT NULL DEFAULT 0,
	compiled_init VARBINARY(1024), -- e.g. 'CALL descendant_tags_id_from_ids("foobar", "5,17");'
	compiled_tbls VARBINARY(1024), -- e.g. 'foobar'
	compiled_fltr VARBINARY(4096), -- e.g. ' AND r2t.tag_id=foobar.node'
	compiled_hvng VARBINARY(4096),
	UNIQUE KEY (name),
	PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS relation_add_box_tags__req_relation_tags (
	rule BIGINT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	descendants_too BOOLEAN NOT NULL DEFAULT FALSE,
	PRIMARY KEY (rule, tag)
);

CREATE TABLE IF NOT EXISTS relation_add_box_tags__req_master_tags (
	rule BIGINT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	descendants_too BOOLEAN NOT NULL DEFAULT FALSE,
	PRIMARY KEY (rule, tag)
);

CREATE TABLE IF NOT EXISTS relation_add_box_tags__req_slave_tags (
	rule BIGINT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	descendants_too BOOLEAN NOT NULL DEFAULT FALSE,
	PRIMARY KEY (rule, tag)
);

CREATE TABLE IF NOT EXISTS relation_add_box_tags__res_relation_tags (
	rule BIGINT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	descendants_too BOOLEAN NOT NULL DEFAULT FALSE,
	PRIMARY KEY (rule, tag)
);

CREATE TABLE IF NOT EXISTS relation_add_box_tags__res_master_tags (
	rule BIGINT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	descendants_too BOOLEAN NOT NULL DEFAULT FALSE,
	PRIMARY KEY (rule, tag)
);

CREATE TABLE IF NOT EXISTS relation_add_box_tags__res_slave_tags (
	rule BIGINT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	descendants_too BOOLEAN NOT NULL DEFAULT FALSE,
	PRIMARY KEY (rule, tag)
);

CREATE TABLE IF NOT EXISTS operators (
	id INT UNSIGNED NOT NULL,
	string VARBINARY(16) NOT NULL,
	UNIQUE KEY (string),
	PRIMARY KEY (id)
);
INSERT INTO operators
(id, string)
VALUES
(0,"AND"),
(1,"OR"),
(2,"XOR"),
(3,"NOT")
ON DUPLICATE KEY UPDATE id=id;

CREATE TABLE IF NOT EXISTS file2thumbnail (
	file BIGINT UNSIGNED NOT NULL PRIMARY KEY,
	x VARBINARY(1024) NOT NULL,
	FOREIGN KEY (file) REFERENCES file (id)
);
-- INSERT INTO file2thumbnail (file,x) SELECT f.id, CONCAT('https://i.ytimg.com/vi/', f.name, '/hqdefault.jpg') FROM file f JOIN dir d ON d.id=f.dir WHERE d.name='https://www.youtube.com/watch?v=';



CREATE TABLE IF NOT EXISTS external_db (
	id INT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
	name VARBINARY(32) NOT NULL UNIQUE KEY
);

CREATE TABLE IF NOT EXISTS file2post (
	-- This links posts in EXTERNAL databases to files in this database
	-- An external database cannot be assumed to have at most one post for a given file. For instance, Reddit posts have duplicates.
	-- A given post may have multiple files.
	file BIGINT UNSIGNED NOT NULL,
	post BIGINT UNSIGNED NOT NULL,
	db INT UNSIGNED NOT NULL,
	PRIMARY KEY (file, post, db),
	FOREIGN KEY (file) REFERENCES file (id),
	FOREIGN KEY (db) REFERENCES external_db (id)
);

ALTER TABLE file2post ADD INDEX IF NOT EXISTS (file);
-- Add an index for performance reasons - almost all queries we do will be joining on file ID, not post or db IDs.


)====="
