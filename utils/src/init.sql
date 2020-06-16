R"=====(

CREATE TABLE user (
	id INT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
	name VARBINARY(100) NOT NULL UNIQUE KEY
);

CREATE TABLE protocol (
	id INT UNSIGNED NOT NULL PRIMARY KEY,
	name VARBINARY(16) NOT NULL UNIQUE KEY
);
INSERT INTO protocol (id, name) VALUES
(0, "NONE!"),
(1, "file://"),
(2, "http://"),
(3, "https://"),
(4, "youtube-dl");

CREATE TABLE _device (
	# Storage device - such as a hard drive, or a website (a remote storage device)
	# Name is the prefix - allowing 'youtube-dl' protocol for 'https://youtube.com/watch?v=' and 'https' protocol for 'https://youtube.com/user/' prefixes
	id BIGINT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
	user INT UNSIGNED NOT NULL,
	name VARBINARY(1024) NOT NULL UNIQUE KEY,
	protocol INT UNSIGNED NOT NULL,
	embed_pre  VARBINARY(1000) NOT NULL DEFAULT "",
	embed_post VARBINARY(1000) NOT NULL DEFAULT "",
	FOREIGN KEY (user) REFERENCES user (id),
	FOREIGN KEY (protocol) REFERENCES protocol (id)
);
CREATE TABLE user2hidden_device (
	user INT UNSIGNED NOT NULL,
	device BIGINT UNSIGNED NOT NULL,
	FOREIGN KEY (user) REFERENCES user (id),
	FOREIGN KEY (device) REFERENCES _device (id),
	PRIMARY KEY (user,device)
);
INSERT INTO _device (name, protocol) VALUES
("https://www.google.com/", (SELECT id FROM protocol WHERE name="https://")),
("https://stackoverflow.com/", (SELECT id FROM protocol WHERE name="https://")),
("https://en.wikipedia.org/", (SELECT id FROM protocol WHERE name="https://")),
("https://github.com/", (SELECT id FROM protocol WHERE name="https://"));
INSERT INTO _device (name,permissions,protocol,embed_pre,embed_post) VALUES
("https://twitter.com/",0,(SELECT id FROM protocol WHERE name='https://'), '<blockquote class="twitter-tweet"><a href="https://twitter.com/AnyUsernameWorksHere/status/', '?ref_src=twsrc%5Etfw">Link</a></blockquote><script async src="https://platform.twitter.com/widgets.js" charset="utf-8"></script>'),
("https://youtube.com/watch?v=",0,(SELECT id FROM protocol WHERE name='youtube-dl'), '<iframe width="1280" height="720" src="https://www.youtube.com/embed/', '" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>');

CREATE TABLE _dir (
	id BIGINT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
	device BIGINT UNSIGNED NOT NULL,
	user INT UNSIGNED NOT NULL,
	name VARBINARY(1024) NOT NULL UNIQUE KEY,
	FOREIGN KEY (user) REFERENCES user (id),
	FOREIGN KEY (device) REFERENCES _device (id)
);
CREATE TABLE user2hidden_dir (
	user INT UNSIGNED NOT NULL,
	dir BIGINT UNSIGNED NOT NULL,
	FOREIGN KEY (user) REFERENCES user (id),
	FOREIGN KEY (dir) REFERENCES _dir (id),
	PRIMARY KEY (user,dir)
);

CREATE TABLE mimetype (
	id INT UNSIGNED NOT NULL PRIMARY KEY,
	name VARBINARY(32) NOT NULL UNIQUE KEY
);
SET @i := -1;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"!!NONE!!");
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"audio/aac");
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"image/bmp");
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"text/css");
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"text/csv");
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"image/gif");
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"text/html");
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"image/jpeg");
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"textjavascript");
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"application/json");
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"audio/mpeg");
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"video/mpeg");
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"audio/ogg");
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"video/ogg");
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"audio/opus");
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"image/png");
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"image/tiff");
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"text/plain");
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"audio/wav");
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"audio/webm");
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"video/webm");
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"image/webp");
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"video/avi");

CREATE TABLE _file (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
	dir BIGINT UNSIGNED NOT NULL,
    name VARBINARY(1024),
    added_on DATETIME DEFAULT CURRENT_TIMESTAMP,
	mimetype INT UNSIGNED NOT NULL DEFAULT 0,
	user INT UNSIGNED NOT NULL,
    UNIQUE KEY (dir, name),
	FOREIGN KEY (dir) REFERENCES _dir (id),
	FOREIGN KEY (mimetype) REFERENCES mimetype (id),
	FOREIGN KEY (user) REFERENCES user (id),
    PRIMARY KEY (id)
);

# Can get dir name from full file path: SELECT SUBSTR(name, 1, LENGTH(name) - LOCATE('/',REVERSE(name))) FROM file


CREATE TABLE file_backup (
	-- WARNING: Not sure how best to deal with the fact that some remote 'files' have different options, e.g. video format
	-- NOTE: A backup might still be remote too. For instance, web.archive.org.
	file BIGINT UNSIGNED NOT NULL,
	dir BIGINT UNSIGNED NOT NULL,
	name VARBINARY(1024) NOT NULL,
	mimetype INT UNSIGNED NOT NULL,
	user INT UNSIGNED NOT NULL,
	PRIMARY KEY (file, dir),
	FOREIGN KEY (dir) REFERENCES _dir (id),
	FOREIGN KEY (file) REFERENCES _file (id),
	FOREIGN KEY (mimetype) REFERENCES mimetype (id),
	FOREIGN KEY (user) REFERENCES user (id)
);


CREATE TABLE file2 (
	# Stores the user-defined variable tables
	id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
	min BIGINT NOT NULL,
	max BIGINT NOT NULL,
	# NOTE: min and max are only relevant for GUI input. File sizes above 4GB would be impossible to input via Qt5 getInt method, but would be otherwise supported if the int type used is BIGINT or BIGINT UNSIGNED.
	conversion INT UNSIGNED NOT NULL, # 0 if no conversion (i.e. the end data type is integer), otherwise the integer is not input directly by a human but relates to something else like a string
	name VARBINARY(128),
	UNIQUE KEY (name),
	PRIMARY KEY (id)
);
INSERT INTO file2 (min, max, conversion, name) VALUES
(0, 9223372036854775807, 0, "duration")
;


CREATE TABLE _tag (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
	user INT UNSIGNED NOT NULL,
    name VARBINARY(128),
	thumbnail VARBINARY(200) NOT NULL DEFAULT "",
	cover VARBINARY(200) NOT NULL DEFAULT "",
    UNIQUE KEY (name),
	FOREIGN KEY (user) REFERENCES user (id),
    PRIMARY KEY (id)
);
INSERT INTO _tag (id,name) VALUES (0,"!!ROOT TAG!!");
UPDATE _tag SET id=0 WHERE name="!!ROOT TAG!!";
# NOTE: Permissions are AND (each non-zero bit is another required permission)
# NOTE: A permission of 0 allows everyone to see, since the permission mask is applied as if(f.permission & u.permission == f.permission)

CREATE TABLE tag2parent (
    tag_id BIGINT UNSIGNED NOT NULL,
    parent_id BIGINT UNSIGNED NOT NULL,
	user INT UNSIGNED NOT NULL,
	FOREIGN KEY (tag_id) REFERENCES _tag (id),
	FOREIGN KEY (parent_id) REFERENCES _tag (id),
	FOREIGN KEY (user) REFERENCES user (id),
    PRIMARY KEY `tag2parent` (`tag_id`, `parent_id`)
);
CREATE TABLE tag2parent_tree (
	tag BIGINT UNSIGNED NOT NULL,
	parent BIGINT UNSIGNED NOT NULL,
	depth INT UNSIGNED NOT NULL,
	FOREIGN KEY (tag) REFERENCES _tag (id),
	FOREIGN KEY (parent) REFERENCES _tag (id),
    PRIMARY KEY (tag, parent)
);

CREATE TABLE user2hidden_tag (
	user INT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	FOREIGN KEY (user) REFERENCES user (id),
	FOREIGN KEY (tag) REFERENCES _tag (id),
	PRIMARY KEY (user,tag)
);

CREATE TABLE file2tag (
    file_id BIGINT UNSIGNED NOT NULL,
    tag_id BIGINT UNSIGNED NOT NULL,
	user INT UNSIGNED NOT NULL,
	FOREIGN KEY (file_id) REFERENCES file (id),
	FOREIGN KEY (tag_id) REFERENCES _tag (id),
	FOREIGN KEY (user) REFERENCES user (id),
    PRIMARY KEY `file2tag` (file_id, tag_id)
);

CREATE TABLE file2dct_hash (
	file BIGINT UNSIGNED NOT NULL,
	x BIGINT UNSIGNED NOT NULL,
	PRIMARY KEY (file, x)
);

CREATE TABLE file2audio_hash (
	file BIGINT UNSIGNED NOT NULL,
	x BIGINT UNSIGNED NOT NULL,
	PRIMARY KEY (file, x)
);

CREATE TABLE file2sha256 (
	file BIGINT UNSIGNED NOT NULL,
	x BINARY(32) NOT NULL,
	PRIMARY KEY (file)
);

CREATE TABLE file2size (
	file BIGINT UNSIGNED NOT NULL,
	x BIGINT UNSIGNED NOT NULL,
	PRIMARY KEY (file)
);

CREATE TABLE file2duration (
	file BIGINT UNSIGNED NOT NULL,
	x BIGINT UNSIGNED NOT NULL,
	PRIMARY KEY (file)
);

CREATE TABLE box (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    file_id BIGINT UNSIGNED NOT NULL,
    frame_n BIGINT UNSIGNED NOT NULL,
    x DOUBLE NOT NULL,  # As proportion of (image|frame) width
    y DOUBLE NOT NULL,
    w DOUBLE NOT NULL,
    h DOUBLE NOT NULL,
    PRIMARY KEY (id)
);

CREATE TABLE box2tag (
	box_id BIGINT UNSIGNED NOT NULL,
	tag_id BIGINT UNSIGNED NOT NULL,  # ID of box tag
	PRIMARY KEY (`box_id`, `tag_id`)
);


CREATE TABLE method (
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
("python_script");

CREATE TABLE era (
	# Defined by two 'framestamps' - analogous to timestamps
	id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
	file_id BIGINT UNSIGNED NOT NULL,
	start BIGINT UNSIGNED NOT NULL,
	end BIGINT UNSIGNED NOT NULL,
	start_method_id BIGINT UNSIGNED NOT NULL DEFAULT 0,
	end_method_id BIGINT UNSIGNED NOT NULL DEFAULT 0,
	s VARCHAR(20000),
	UNIQUE KEY (file_id, start, end),
	PRIMARY KEY (id)
);

CREATE TABLE era2tag (
	era_id BIGINT UNSIGNED NOT NULL,
	tag_id BIGINT UNSIGNED NOT NULL,
	PRIMARY KEY (era_id, tag_id)
);



# The following tables populate the tags of relationships, not boxes.
# For instance, a man typing on a keyboard would be represented by (a box tagged "man") related to (a box tagged "keyboard") via (a relationship tagged "typing").
# relationtag2tag generates extra relationship tags based on the master and slave box tags. For instance, if the keyboard is a "mechanical" keyboard, it could generate another tag ("man typing on mechanical keyboard") for the relationship.

CREATE TABLE relation (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    master_id BIGINT UNSIGNED NOT NULL,  # ID of master box
    slave_id BIGINT UNSIGNED NOT NULL,
    PRIMARY KEY (id)
);

CREATE TABLE relation2tag (
    relation_id BIGINT UNSIGNED NOT NULL,
    tag_id BIGINT UNSIGNED NOT NULL,  # ID of box tag
    PRIMARY KEY (`relation_id`, `tag_id`)
);



# For instance, suppose there is (a box tagged "human" and "male") which is related to (a box tagged "arm") via (the "body part" tag). We would like the arm to inherit (the "human" and "male" tags).
# The following tables describe the rules. Each rule is of the form (ID,  a = list of master tag IDs,  b = list of slave tag IDs,  c = list of resulting master tags,  d =list of resulting slave tags).
# If a and b hold for a given relation (AND rather than OR - i.e. each tag ID in a must be present in the master box), then all tags of c are added to the master box, and all tags of d are added to the slave box.
# NOTE: 0 is a special 'tag ID'. No tags have an ID of 0. Instead, it means the same 'tag ID' as the triggering tags - effectively propagating the tags to the related box. For instance, (a "body part" box) related to (a "human" box) via a ("body part" relation) (where "human" is the master and "body part" the slave) would propagate the "human" tag to the "body part" box, if the 'res_slave' tag ID was 0. Another use of this may be in a relation designating two things as "identical".

CREATE TABLE relation_add_box_tags__rules (
	id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
	name VARBINARY(128) NOT NULL,
	req_relation_operator INT UNSIGNED NOT NULL DEFAULT 0,
	req_master_operator INT UNSIGNED NOT NULL DEFAULT 0,
	req_slave_operator INT UNSIGNED NOT NULL DEFAULT 0,
	compiled_init VARBINARY(1024), # e.g. 'CALL descendant_tags_id_from_ids("foobar", "5,17");'
	compiled_tbls VARBINARY(1024), # e.g. 'foobar'
	compiled_fltr VARBINARY(4096), # e.g. ' AND r2t.tag_id=foobar.node'
	compiled_hvng VARBINARY(4096),
	UNIQUE KEY (name),
	PRIMARY KEY (id)
);

CREATE TABLE relation_add_box_tags__req_relation_tags (
	rule BIGINT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	descendants_too BOOLEAN NOT NULL DEFAULT FALSE,
	PRIMARY KEY (rule, tag)
);

CREATE TABLE relation_add_box_tags__req_master_tags (
	rule BIGINT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	descendants_too BOOLEAN NOT NULL DEFAULT FALSE,
	PRIMARY KEY (rule, tag)
);

CREATE TABLE relation_add_box_tags__req_slave_tags (
	rule BIGINT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	descendants_too BOOLEAN NOT NULL DEFAULT FALSE,
	PRIMARY KEY (rule, tag)
);

CREATE TABLE relation_add_box_tags__res_relation_tags (
	rule BIGINT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	descendants_too BOOLEAN NOT NULL DEFAULT FALSE,
	PRIMARY KEY (rule, tag)
);

CREATE TABLE relation_add_box_tags__res_master_tags (
	rule BIGINT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	descendants_too BOOLEAN NOT NULL DEFAULT FALSE,
	PRIMARY KEY (rule, tag)
);

CREATE TABLE relation_add_box_tags__res_slave_tags (
	rule BIGINT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	descendants_too BOOLEAN NOT NULL DEFAULT FALSE,
	PRIMARY KEY (rule, tag)
);

CREATE TABLE operators (
	id INT UNSIGNED NOT NULL,
	string VARBINARY(16) NOT NULL,
	UNIQUE KEY (string),
	PRIMARY KEY (id)
);
INSERT INTO operators (id, string) VALUES (0,"AND"), (1,"OR"), (2,"XOR"), (3,"NOT");

CREATE TABLE file2md5 (
	# To use KDE thumbnails
	file BIGINT UNSIGNED NOT NULL PRIMARY KEY,
	x BINARY(16) NOT NULL,
	FOREIGN KEY (file) REFERENCES _file (id)
);

CREATE TABLE file2qt5md5 (
	# To use KDE thumbnails
	file BIGINT UNSIGNED NOT NULL PRIMARY KEY,
	x BINARY(16) NOT NULL,
	FOREIGN KEY (file) REFERENCES _file (id)
);

CREATE TABLE file2thumbnail (
	file BIGINT UNSIGNED NOT NULL PRIMARY KEY,
	x VARBINARY(1024) NOT NULL,
	FOREIGN KEY (file) REFERENCES _file (id)
);
-- INSERT INTO file2thumbnail (file,x) SELECT f.id, CONCAT('https://i.ytimg.com/vi/', f.name, '/hqdefault.jpg') FROM _file f JOIN _dir d ON d.id=f.dir WHERE d.name='https://www.youtube.com/watch?v=';



CREATE TABLE settings (
	name VARBINARY(128) NOT NULL,
	filename_regexp VARCHAR(1024),
	files_from VARCHAR(1024) NOT NULL,
	start_from VARCHAR(1024) NOT NULL,
	skip_tagged BOOLEAN NOT NULL,
	skip_trans BOOLEAN NOT NULL,
	skip_grey BOOLEAN NOT NULL,
	stpetersburger INT UNSIGNED NOT NULL,
	files_from_which INT UNSIGNED NOT NULL,
	start_from_which INT UNSIGNED NOT NULL,
	file_sz_min BIGINT UNSIGNED NOT NULL,
	file_sz_max BIGINT UNSIGNED NOT NULL,
	w_max INT UNSIGNED NOT NULL,
	w_min INT UNSIGNED NOT NULL,
	h_max INT UNSIGNED NOT NULL,
	h_min INT UNSIGNED NOT NULL,
	PRIMARY KEY (name)
);

# Row-level security for modern MySQL/MariaDB
# Based on: https://mariadb.com/resources/blog/protect-your-data-row-level-security-in-mariadb-10-0/
CREATE TABLE permission (
	id BIGINT UNSIGNED NOT NULL PRIMARY KEY,
	name VARCHAR(32) NOT NULL UNIQUE KEY
);
CREATE TABLE user2permissions (
	user VARBINARY(32) NOT NULL PRIMARY KEY,
	permissions BIGINT UNSIGNED NOT NULL
);
CREATE SQL SECURITY DEFINER
VIEW tag
AS
	SELECT *
	FROM _tag
	WHERE id NOT IN (
		SELECT t2pt.tag
		FROM user u
		JOIN user2hidden_tag u2ht ON u2ht.user=u.id
		JOIN tag2parent_tree t2pt ON t2pt.parent=u2ht.tag
		WHERE u.name=SESSION_USER()
	)
;
CREATE SQL SECURITY DEFINER
VIEW file
AS
	SELECT *
	FROM _file
	WHERE id NOT IN (
		SELECT f2t.file_id
		FROM user u
		JOIN user2hidden_tag u2ht ON u2ht.user=u.id
		JOIN tag2parent_tree t2pt ON t2pt.parent=u2ht.tag
		JOIN file2tag f2t ON f2t.tag_id=t2pt.tag
		WHERE u.name=SESSION_USER()
	)
;
CREATE SQL SECURITY DEFINER
VIEW dir
AS
	SELECT d.*
	FROM _dir d
	JOIN user2permissions u2p ON u2p.user=SESSION_USER()
	WHERE u2p.permissions & d.permissions = d.permissions
;
CREATE SQL SECURITY DEFINER
VIEW device
AS
	SELECT d.*
	FROM _device d
	JOIN user2permissions u2p ON u2p.user=SESSION_USER()
	WHERE u2p.permissions & d.permissions = d.permissions
;



CREATE TABLE external_db (
	id INT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
	name VARBINARY(32) NOT NULL UNIQUE KEY
);

CREATE TABLE file2post (
	-- This links posts in EXTERNAL databases to files in this database
	-- An external database cannot be assumed to have at most one post for a given file. For instance, Reddit posts have duplicates.
	-- A given post may have multiple files.
	file BIGINT UNSIGNED NOT NULL,
	post BIGINT UNSIGNED NOT NULL,
	db INT UNSIGNED NOT NULL,
	PRIMARY KEY (file, post, db),
	FOREIGN KEY (file) REFERENCES _file (id),
	FOREIGN KEY (db) REFERENCES external_db (id)
);

ALTER TABLE file2post ADD INDEX (file);
-- Add an index for performance reasons - almost all queries we do will be joining on file ID, not post or db IDs.


)====="
