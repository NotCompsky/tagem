R"=====(

CREATE TABLE file (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    name VARBINARY(1024),
    language_id BIGINT UNSIGNED,
    score INT,
    added_on DATETIME DEFAULT CURRENT_TIMESTAMP,
    note VARBINARY(30000),
    UNIQUE KEY (name),
    PRIMARY KEY (id)
);

CREATE TABLE tag (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    name VARBINARY(128),
    UNIQUE KEY (name),
    PRIMARY KEY (id)
);

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

CREATE TABLE instance (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    name VARBINARY(128),
    file_id BIGINT UNSIGNED NOT NULL,
    frame_n BIGINT UNSIGNED NOT NULL,
    x DOUBLE NOT NULL,  # As proportion of (image|frame) width
    y DOUBLE NOT NULL,
    w DOUBLE NOT NULL,
    h DOUBLE NOT NULL,
    PRIMARY KEY (id)
);

CREATE TABLE instance2tag (
    instance_id BIGINT UNSIGNED NOT NULL,
    tag_id BIGINT UNSIGNED NOT NULL,  # ID of instance tag
    PRIMARY KEY `instance2tag` (`instance_id`, `tag_id`)
);



CREATE TABLE framestamp (
	id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
	file_id BIGINT UNSIGNED NOT NULL,
	frame_id BIGINT UNSIGNED NOT NULL,
	PRIMARY KEY (id)
);

CREATE TABLE framestamp2tag (
	framestamp_id BIGINT UNSIGNED NOT NULL,
	tag_id BIGINT UNSIGNED NOT NULL,
	PRIMARY KEY (`framestamp_id`, `tag_id`)
);



# The following tables populate the tags of relationships, not instances.
# For instance, a man typing on a keyboard would be represented by (an instance tagged "man") related to (an instance tagged "keyboard") via (a relationship tagged "typing").
# relationtag2tag generates extra relationship tags based on the master and slave instance tags. For instance, if the keyboard is a "mechanical" keyboard, it could generate another tag ("man typing on mechanical keyboard") for the relationship.

CREATE TABLE relation (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    master_id BIGINT UNSIGNED NOT NULL,  # ID of master instance
    slave_id BIGINT UNSIGNED NOT NULL,
    PRIMARY KEY (id)
);

CREATE TABLE relation2tag (
    relation_id BIGINT UNSIGNED NOT NULL,
    tag_id BIGINT UNSIGNED NOT NULL,  # ID of instance tag
    PRIMARY KEY `instance2tag` (`relation_id`, `tag_id`)
);



# For instance, suppose there is (an instance tagged "human" and "male") which is related to (an instance tagged "arm") via (the "body part" tag). We would like the arm to inherit (the "human" and "male" tags).
# The following tables describe the rules. Each rule is of the form (ID,  a = list of master tag IDs,  b = list of slave tag IDs,  c = list of resulting master tags,  d =list of resulting slave tags).
# If a and b hold for a given relation (AND rather than OR - i.e. each tag ID in a must be present in the master instance), then all tags of c are added to the master instance, and all tags of d are added to the slave instance.
# NOTE: 0 is a special 'tag ID'. No tags have an ID of 0. Instead, it means the same 'tag ID' as the triggering tags - effectively propagating the tags to the related instance. For instance, (a "body part" instance) related to (a "human" instance) via a ("body part" relation) (where "human" is the master and "body part" the slave) would propagate the "human" tag to the "body part" instance, if the 'res_slave' tag ID was 0. Another use of this may be in a relation designating two things as "identical".

CREATE TABLE relation_add_instance_tags__rules (
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

CREATE TABLE relation_add_instance_tags__req_relation_tags (
	rule BIGINT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	descendants_too BOOLEAN NOT NULL DEFAULT FALSE,
	PRIMARY KEY (rule, tag)
);

CREATE TABLE relation_add_instance_tags__req_master_tags (
	rule BIGINT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	descendants_too BOOLEAN NOT NULL DEFAULT FALSE,
	PRIMARY KEY (rule, tag)
);

CREATE TABLE relation_add_instance_tags__req_slave_tags (
	rule BIGINT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	descendants_too BOOLEAN NOT NULL DEFAULT FALSE,
	PRIMARY KEY (rule, tag)
);

CREATE TABLE relation_add_instance_tags__res_relation_tags (
	rule BIGINT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	descendants_too BOOLEAN NOT NULL DEFAULT FALSE,
	PRIMARY KEY (rule, tag)
);

CREATE TABLE relation_add_instance_tags__res_master_tags (
	rule BIGINT UNSIGNED NOT NULL,
	tag BIGINT UNSIGNED NOT NULL,
	descendants_too BOOLEAN NOT NULL DEFAULT FALSE,
	PRIMARY KEY (rule, tag)
);

CREATE TABLE relation_add_instance_tags__res_slave_tags (
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

)====="
