R"=====(

CREATE TABLE _file (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    name VARBINARY(1024),
    added_on DATETIME DEFAULT CURRENT_TIMESTAMP,
    permissions BIGINT UNSIGNED NOT NULL DEFAULT 0,
    UNIQUE KEY (name),
    PRIMARY KEY (id)
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
    name VARBINARY(128),
	permissions BIGINT UNSIGNED NOT NULL DEFAULT 0,
    UNIQUE KEY (name),
    PRIMARY KEY (id)
);
# NOTE: Permissions are AND (each non-zero bit is another required permission)
# NOTE: A permission of 0 allows everyone to see, since the permission mask is applied as if(f.permission & u.permission == f.permission)

CREATE TABLE tag2parent (
    tag_id BIGINT UNSIGNED NOT NULL,
    parent_id BIGINT UNSIGNED NOT NULL,
    PRIMARY KEY `tag2parent` (`tag_id`, `parent_id`)
);

CREATE TABLE file2tag (
    file_id BIGINT UNSIGNED NOT NULL,
    tag_id BIGINT UNSIGNED NOT NULL,
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
	SELECT t.*
	FROM _tag t
	JOIN user2permissions u2p ON u2p.user=SESSION_USER()
	WHERE u2p.permissions & t.permissions=t.permissions
;
CREATE SQL SECURITY DEFINER
VIEW file
AS
	SELECT f.*
	FROM _file f
	LEFT JOIN file2tag f2t ON f2t.file_id=f.id
	LEFT JOIN tag t ON t.id=f2t.tag_id
	LEFT JOIN user2permissions u2p ON u2p.user=SESSION_USER()
	WHERE (f2t.tag_id IS NULL) OR (u2p.permissions & t.permissions=t.permissions)
;

)====="
