R"=====(

CREATE TABLE file (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    name VARBINARY(1024),
    language_id BIGINT UNSIGNED,
    score INT,
    added_on DATETIME DEFAULT CURRENT_TIMESTAMP,
    note VARBINARY(30000),
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
    x DOUBLE NOT NULL,  # As proportion of (image|frame)'s width
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

CREATE TABLE relationtag2tag (
    # Decides on the tags a relation automatically gives rise to, given the tags of the master and slave object instances
    tag BIGINT UNSIGNED NOT NULL,  # ID of the relation's tag (not relation table's ID)
    master BIGINT UNSIGNED NOT NULL,  # Tag ID of one of the master's tags
    slave BIGINT UNSIGNED NOT NULL,
    result BIGINT UNSIGNED NOT NULL,  # Resulting Tag ID
    PRIMARY KEY `relation2mst` (tag, master, slave, result)
);
)====="
