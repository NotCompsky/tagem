CALL descendant_tags_id_from_ids("_r1", "86");
CALL descendant_tags_id_from_ids("_m1", "90");
#CALL descendant_tags_id_from_ids("_m2", "355");
CALL descendant_tags_id_from_ids("_s1", "26");

# NOTE: For OR, we can just put csv into these procedure calls.

SELECT
	r.id
FROM
	relation r,
	relation2tag r2t,
	instance2tag i2t_m,
	instance2tag i2t_s,
	_r1,
	_m1,
	_m2,
	_s1
WHERE r.id=r2t.relation_id
  AND i2t_m.instance_id=r.master_id
  AND i2t_s.instance_id=r.slave_id
  AND r2t.tag_id IN (SELECT node FROM _r1) # SHOULD BE UNNECESSARY for AND! But speeds it up drastically.
  AND i2t_m.tag_id IN (SELECT node FROM _m1) # SHOULD BE UNNECESSARY!
  #AND i2t_m.tag_id IN (SELECT node FROM _m2) # SHOULD BE UNNECESSARY!
  AND i2t_s.tag_id IN (SELECT node FROM _s1) # SHOULD BE UNNECESSARY!
GROUP BY r.id
HAVING COUNT(_r1.node=r2t.tag_id) != 0
   AND COUNT(_m1.node=i2t_m.tag_id) != 0
   #AND COUNT(_m2.node=i2t_m.tag_id)
   AND COUNT(_s1.node=i2t_s.tag_id) != 0
;











# Useful commands




# View the mean score of each tag
SELECT t.name, COUNT(f2s.x), SUM(f2s.x) / COUNT(f2s.x) AS `n`
FROM tag t, file2tag f2t, file2Score f2s
WHERE t.id=f2t.tag_id
  AND f2t.file_id=f2s.file_id
GROUP BY t.id
ORDER BY n DESC
;

# View the mean score of each tag, including its descendants
CALL descendant_tags_id_rooted();
SELECT t.name, COUNT(f2s.x) as `c`, SUM(f2s.x) / COUNT(f2s.x) AS `n`
FROM tag t, root2tag r2t, file2tag f2t, file2Score f2s
WHERE t.id=r2t.node
  AND r2t.root=f2t.tag_id
  AND f2t.file_id=f2s.file
GROUP BY t.id
HAVING `c` > 1
ORDER BY n DESC
;






# Insert new relationtag2tag using names in place of IDs

INSERT INTO relationtag2tag
SELECT t.id, m.id, s.id, r.id
FROM tag t, tag m, tag s, tag r
WHERE t.name IN ('Typing')
  AND m.name IN ('Programmer')
  AND s.name IN ('Keyboard')
  AND r.name = 'Programming'
;

# View previous relationtag2tag

SELECT t1.name as tag, t2.name as master, t3.name as slave, t4.name as result
FROM relationtag2tag rt2t, tag t1, tag t2, tag t3, tag t4
WHERE t1.id=rt2t.tag AND t2.id=rt2t.master AND t3.id=rt2t.slave AND t4.id=rt2t.result
;


# Add into relationtag2tag

INSERT INTO relationtag2tag
(tag, master, slave, result)
SELECT t1.id, t2.id, t3.id, t4.id
FROM tag t1, tag t2, tag t3, tag t4
WHERE t1.name='foo' AND t2.name='bar' AND t3.name='ree' AND t4.name='gee'
;

# View files involving this relation (Derived from: Table of relation ID to all tags and descendant tags)
SELECT DISTINCT f.name
FROM file f, instance i_master, relation r, relation2tag r2t, tag2root t2R, tag t
WHERE t2R.root=r2t.tag_id AND t.id=t2R.node AND r.id=r2t.relation_id AND i_master.id=r.master_id AND f.id=i_master.file_id
  AND t.name='foobar'
;



# Relate relations to their resulting tags

## Populate root2tag and two copies, since MySQL does not allow a temporary table to be referenced more than once in a single query
CALL descendant_tags_id_rooted(); # Populate root2tag
DROP TABLE IF EXISTS root2tag_a, root2tag_b;
CREATE TEMPORARY TABLE root2tag_a LIKE root2tag;
CREATE TEMPORARY TABLE root2tag_b LIKE root2tag;
INSERT INTO root2tag_a SELECT * FROM root2tag;
INSERT INTO root2tag_b SELECT * FROM root2tag;


SELECT r.id, R2t.node, t1.name AS 'Result', t2.name AS 'Relation', t3.name AS 'master', t4.name AS 'slave'
FROM relation2tag r2t, relation r, root2tag R2t, instance2tag i2t_master, instance2tag i2t_slave, root2tag_a R2t_a, root2tag_b R2t_b, relationtag2tag rt2t, tag t1, tag t2, tag t3, tag t4
WHERE r.id=r2t.relation_id
  AND R2t.root=r2t.tag_id
  AND i2t_master.instance_id=r.master_id
  AND i2t_slave.instance_id =r.slave_id
  AND R2t_a.root=i2t_master.tag_id
  AND R2t_b.root=i2t_slave.tag_id
  AND rt2t.tag=R2t.node AND rt2t.master=R2t_a.node AND rt2t.slave=R2t_b.node
  AND t1.id=rt2t.result
  AND t2.id=r2t.tag_id
  AND t3.id=i2t_master.tag_id
  AND t4.id=i2t_slave.tag_id
;


SELECT r.id,  CONCAT(tD.name) AS 'Result: Relation',  CONCAT(tE.name) AS 'Result: Master',  CONCAT(tF.name) AS 'Result: Slave'
FROM
	relation r,
	relation2tag r2t,
	instance i,
	instance2tag i2m_master,
	instance2tag i2m_slave,
	relation_add_instance_tags__rules rules
LEFT JOIN relation_add_instance_tags__req_relation_tags A ON A.rule=rules.id
LEFT JOIN relation_add_instance_tags__req_master_tags   B ON B.rule=rules.id
LEFT JOIN relation_add_instance_tags__req_slave_tags    C ON C.rule=rules.id
LEFT JOIN relation_add_instance_tags__res_relation_tags D ON D.rule=rules.id
LEFT JOIN tag tD ON tD.id=D.tag
LEFT JOIN relation_add_instance_tags__res_master_tags   E ON E.rule=rules.id
LEFT JOIN tag tE ON tE.id=E.tag
LEFT JOIN relation_add_instance_tags__res_slave_tags    F ON F.rule=rules.id
LEFT JOIN tag tF ON tF.id=F.tag
WHERE r.id=r2t.relation_id
  AND i2m_master.instance_id=r.master_id
  AND i2m_slave.instance_id=r.slave_id
  AND (
	(A.tag IS NULL) OR
	(r2t.tag_id IN (
		SELECT X.tag
		FROM relation_add_instance_tags__req_relation_tags X
		WHERE X.rule=r.id
	))
  )
  AND (
	(B.tag IS NULL) OR
	(i2m_master.tag_id=B.tag)
  )
  AND (
	(C.tag IS NULL) OR
	(i2m_slave.tag_id=C.tag)
  )
GROUP BY r.id
;

















# Table of relation tag names to master and slave tag names

CALL descendant_tags_id_rooted(); # Populate root2tag
CALL descendant_tags_id_from("action_tags", "'Action'");

SELECT relation_id, A.node, B.name as 'relation', E.name as 'master', F.name as 'slave'
FROM relation2tag
JOIN root2tag A ON A.node=tag_id # Get child tags
JOIN action_tags actt ON actt.node=A.root # Otherwise get nonsense results if we have a non-action parent for a relation tag
JOIN tag B ON B.id=A.node
JOIN relation r ON r.id=relation_id
JOIN instance2tag C ON C.instance_id=master_id
JOIN instance2tag D ON D.instance_id=slave_id
JOIN tag E ON E.id=C.tag_id
JOIN tag F ON F.id=D.tag_id
;








# Simplify relations by rewriting those ("Person") -"Arm"-> ("Arm") relations to ("Person") -"Body Part"> ("Arm")

CALL descendant_tags_id_rooted(); # Populate root2tag
CALL descendant_tags_id_from("action_tags", "'Body Part'");

SELECT relation_id, A.node, B.name as 'relation', E.name as 'master', F.name as 'slave'
FROM relation2tag
JOIN root2tag A ON A.node=tag_id # Get child tags
JOIN action_tags actt ON actt.node=A.root # Otherwise get nonsense results if we have a non-action parent for a relation tag
JOIN tag B ON B.id=A.node
JOIN relation r ON r.id=relation_id
JOIN instance2tag C ON C.instance_id=master_id
JOIN instance2tag D ON D.instance_id=slave_id
JOIN tag E ON E.id=C.tag_id
JOIN tag F ON F.id=D.tag_id
WHERE B.id!=(SELECT id FROM tag WHERE name="Body Part")
;












SELECT
	r.name,
	o1.string,
	GROUP_CONCAT(DISTINCT tA.name) AS 'Req. Relation',
	o2.string,
	GROUP_CONCAT(DISTINCT tB.name) AS 'Req. Master',
	o3.string,
	GROUP_CONCAT(DISTINCT tC.name) AS 'Req. Slave',
	GROUP_CONCAT(DISTINCT tD.name) AS 'Res. Relation',
	GROUP_CONCAT(DISTINCT tE.name) AS 'Res. Master',
	GROUP_CONCAT(DISTINCT tD.name) AS 'Res. Slave'
FROM operators o1, operators o2, operators o3, relation_add_instance_tags__rules r
LEFT JOIN relation_add_instance_tags__req_relation_tags A ON A.rule=r.id
LEFT JOIN tag tA ON tA.id=A.tag
LEFT JOIN relation_add_instance_tags__req_master_tags   B ON B.rule=r.id
LEFT JOIN tag tB ON tB.id=B.tag
LEFT JOIN relation_add_instance_tags__req_slave_tags    C ON C.rule=r.id
LEFT JOIN tag tC ON tC.id=C.tag
LEFT JOIN relation_add_instance_tags__res_relation_tags D ON D.rule=r.id
LEFT JOIN tag tD ON tD.id=D.tag
LEFT JOIN relation_add_instance_tags__res_master_tags   E ON E.rule=r.id
LEFT JOIN tag tE ON tE.id=E.tag
LEFT JOIN relation_add_instance_tags__res_slave_tags    F ON F.rule=r.id
LEFT JOIN tag tF ON tF.id=F.tag
WHERE o1.id=r.req_relation_operator
  AND o2.id=r.req_master_operator
  AND o3.id=r.req_slave_operator
GROUP BY r.id
;


SELECT
	r.req_relation_operator,
	GROUP_CONCAT(DISTINCT A.tag) AS 'Req. Relation',
	r.req_master_operator,
	GROUP_CONCAT(DISTINCT B.tag) AS 'Req. Master',
	r.req_slave_operator,
	GROUP_CONCAT(DISTINCT C.tag) AS 'Req. Slave',
	GROUP_CONCAT(DISTINCT D.tag) AS 'Res. Relation',
	GROUP_CONCAT(DISTINCT E.tag) AS 'Res. Master',
	GROUP_CONCAT(DISTINCT D.tag) AS 'Res. Slave'
FROM operators o1, operators o2, operators o3, relation_add_instance_tags__rules r
LEFT JOIN relation_add_instance_tags__req_relation_tags A ON A.rule=r.id
LEFT JOIN relation_add_instance_tags__req_master_tags   B ON B.rule=r.id
LEFT JOIN relation_add_instance_tags__req_slave_tags    C ON C.rule=r.id
LEFT JOIN relation_add_instance_tags__res_relation_tags D ON D.rule=r.id
LEFT JOIN relation_add_instance_tags__res_master_tags   E ON E.rule=r.id
LEFT JOIN relation_add_instance_tags__res_slave_tags    F ON F.rule=r.id
WHERE r.id=5
;












# Table of relation ID to all tags and descendant tags
SELECT t.name, r2t.relation_id
FROM relation2tag r2t, tag2root t2R, tag t
WHERE t2R.root=r2t.tag_id AND t.id=t2R.node
;


# Display heirarchy of tags

SELECT B.child, t.name as 'parent'
FROM tag t
RIGHT JOIN (
    SELECT t.name as 'child', A.parent_id
    FROM tag t
    JOIN (
        SELECT tag_id, parent_id
        FROM tag2parent
    ) A ON A.tag_id = t.id
) B ON B.parent_id = t.id
;








# Collect all the direct descendends of tag of ID `N`, with descendant level <= 1 (i.e. itself and direct children)

DROP PROCEDURE IF EXISTS descendant_tags_id__max_depth_1;

delimiter $$

CREATE PROCEDURE descendant_tags_id__max_depth_1(tbl VARBINARY(1024),  str VARBINARY(1024))
BEGIN
	set @query = concat("DROP TABLE IF EXISTS ", tbl, ";");
	PREPARE stmt FROM @query;
	EXECUTE stmt;

	set @query = concat("CREATE TEMPORARY TABLE ", tbl, " (node BIGINT UNSIGNED PRIMARY KEY,  root BIGINT UNSIGNED NOT NULL);");
	PREPARE stmt FROM @query;
	EXECUTE stmt;

	set @query = concat("INSERT INTO ", tbl, " (node, root) SELECT id, id FROM tag WHERE name IN (", str, ");");
	PREPARE stmt FROM @query;
	EXECUTE stmt;

	DROP TABLE IF EXISTS _tmp;

	set @query = concat("INSERT INTO ", tbl, " (node, root) SELECT t2p.tag_id, t2p.parent_id FROM tag2parent t2p, tag t WHERE t2p.parent_id=t.id AND t.name IN (", str, ");");
	PREPARE stmt FROM @query;
	EXECUTE stmt;
END $$

delimiter ;

## eg
# CALL descendant_tags_id__max_depth_1('ttttt', "'Human'");
# SELECT t.name, T.name FROM ttttt a, tag t, tag T WHERE a.node=t.id AND a.root=T.id;






# Print all tags (including itself) descended from tag of ID `N`

DROP PROCEDURE IF EXISTS descendant_tags_id;

delimiter $$

CREATE PROCEDURE descendant_tags_id(seed INT UNSIGNED)
BEGIN
  -- Temporary storage
  DROP TABLE IF EXISTS _result;
  CREATE TEMPORARY TABLE _result (node INT UNSIGNED PRIMARY KEY);

  -- Seeding
  INSERT INTO _result VALUES (seed);

  -- Iteration
  DROP TABLE IF EXISTS _tmp;
  CREATE TEMPORARY TABLE _tmp LIKE _result;
  REPEAT
    TRUNCATE TABLE _tmp;
    INSERT IGNORE INTO _tmp SELECT tag_id AS node
      FROM _result JOIN tag2parent ON node = parent_id;

    INSERT IGNORE INTO _result SELECT node FROM _tmp;
  UNTIL ROW_COUNT() = 0
  END REPEAT;
  DROP TABLE _tmp;
  SELECT * FROM _result;
END $$

delimiter ;

## Usage: `CALL descendant_tags_id(N)`
## Slightly modified query from "Mats Kindahl" from "https://stackoverflow.com/questions/7631048/connect-by-prior-equivalent-for-mysql"




# The above, but using tag names rather than IDs

## Procedure defined same as above, but replacing seeding with:
  INSERT INTO _result (SELECT id FROM tag WHERE name = seed);
### and replacing argument with (seed VARBINARY(128))

DROP PROCEDURE IF EXISTS descendant_tags_name;

delimiter $$

CREATE PROCEDURE descendant_tags_name(seed VARBINARY(128))
BEGIN
  -- Temporary storage
  DROP TABLE IF EXISTS _result;
  CREATE TEMPORARY TABLE _result (node INT UNSIGNED PRIMARY KEY);

  -- Seeding
  INSERT INTO _result (SELECT id FROM tag WHERE name = seed);

  -- Iteration
  DROP TABLE IF EXISTS _tmp;
  CREATE TEMPORARY TABLE _tmp LIKE _result;
  REPEAT
    TRUNCATE TABLE _tmp;
    INSERT IGNORE INTO _tmp SELECT tag_id AS node
      FROM _result JOIN tag2parent ON node = parent_id;

    INSERT IGNORE INTO _result SELECT node FROM _tmp;
  UNTIL ROW_COUNT() = 0
  END REPEAT;
  DROP TABLE _tmp;
  SELECT * FROM _result;
END $$

delimiter ;

## Usage:
CALL descendant_tags_name("TAG_NAME");
SELECT name
FROM tag
JOIN _result ON node = id
;


# Find all files tagged with TAG_NAME or one of its descendant tag

CALL descendant_tags_id((SELECT id FROM tag WHERE name='TAG_NAME'));
SELECT *
FROM file
JOIN (
    SELECT file_id
    FROM file2tag
    WHERE tag_id IN (SELECT node FROM _result)
) A ON A.file_id = id
;



# Find all files tagged with (TAG1 or TAG2) or one of their descendant tags

## Procedures

DROP PROCEDURE IF EXISTS descendant_tags_id_init;
DROP PROCEDURE IF EXISTS descendant_tags_id_preseeded;
DROP PROCEDURE IF EXISTS descendant_tags_id_from;
DROP PROCEDURE IF EXISTS descendant_tags_id_from_ids;

delimiter $$

CREATE PROCEDURE descendant_tags_id_init(tbl VARBINARY(1024))
BEGIN
    set @query = concat("DROP TABLE IF EXISTS ", tbl, ";");
    PREPARE stmt FROM @query;
    EXECUTE stmt;
    
    set @query = concat("CREATE TEMPORARY TABLE ", tbl, " (node BIGINT UNSIGNED PRIMARY KEY);");
    PREPARE stmt FROM @query;
    EXECUTE stmt;
END $$

CREATE PROCEDURE descendant_tags_id_preseeded(tbl VARBINARY(1024))
BEGIN
    set @query_b = concat("INSERT IGNORE INTO _tmp SELECT tag_id AS node FROM ", tbl, " JOIN tag2parent ON node = parent_id;");
    set @query_d = concat("INSERT IGNORE INTO ", tbl, " SELECT node FROM _tmp;");
    PREPARE stmt_b FROM @query_b;
    PREPARE stmt_d FROM @query_d;
    REPEAT
        TRUNCATE TABLE _tmp;
        EXECUTE stmt_b;
        EXECUTE stmt_d;
    UNTIL ROW_COUNT() = 0
    END REPEAT;
END $$

CREATE PROCEDURE descendant_tags_id_from(tbl VARBINARY(1024),  str VARBINARY(1024))
BEGIN
    CALL descendant_tags_id_init(tbl);
    set @query = concat("INSERT INTO ", tbl, " (node) SELECT id FROM tag WHERE name IN (", str, ");");
    PREPARE stmt FROM @query;
    EXECUTE stmt;
    
    
    DROP TABLE IF EXISTS _tmp;
    
    set @query = concat("CREATE TEMPORARY TABLE _tmp LIKE ", tbl, ";");
    PREPARE stmt FROM @query;
    EXECUTE stmt;
    
    
    CALL descendant_tags_id_preseeded(tbl);
    #set @query = concat("SELECT * FROM ", tbl, ";");
    #PREPARE stmt FROM @query;
    #EXECUTE stmt;
END $$

CREATE PROCEDURE descendant_tags_id_from_ids(tbl VARBINARY(1024),  str VARBINARY(1024))
BEGIN
    CALL descendant_tags_id_init(tbl);
    set @query = concat("INSERT INTO ", tbl, " (node) VALUES (", str, ");");
    PREPARE stmt FROM @query;
    EXECUTE stmt;
    
    
    DROP TABLE IF EXISTS _tmp;
    
    set @query = concat("CREATE TEMPORARY TABLE _tmp LIKE ", tbl, ";");
    PREPARE stmt FROM @query;
    EXECUTE stmt;
    
    
    CALL descendant_tags_id_preseeded(tbl);
    #set @query = concat("SELECT * FROM ", tbl, ";");
    #PREPARE stmt FROM @query;
    #EXECUTE stmt;
END $$

delimiter ;

## Usage

CALL descendant_tags_id_from("_tmp_arg",  "'tagname', 'tagname2'");




# Create full tag2root table listing all descendant tags for each tag

## Procedures

DROP PROCEDURE IF EXISTS descendant_tags_id_rooted;

delimiter $$

CREATE PROCEDURE descendant_tags_id_rooted()
BEGIN
    DROP TABLE IF EXISTS tag2root;
    CREATE TEMPORARY TABLE tag2root (node BIGINT UNSIGNED NOT NULL,  root BIGINT UNSIGNED NOT NULL,  PRIMARY KEY `tag2root` (node, root));
    INSERT INTO tag2root (node, root) SELECT id, id FROM tag;
    
    DROP TABLE IF EXISTS _tmp;
    CREATE TEMPORARY TABLE _tmp LIKE tag2root;
    
    REPEAT
        TRUNCATE TABLE _tmp;
        INSERT IGNORE INTO _tmp SELECT tag_id, root FROM tag2root JOIN tag2parent ON node = parent_id;
        INSERT IGNORE INTO tag2root SELECT node, root FROM _tmp;
    UNTIL ROW_COUNT() = 0
    END REPEAT;
    
    DROP TABLE IF EXISTS root2tag;
    CREATE TEMPORARY TABLE root2tag (node BIGINT UNSIGNED NOT NULL,  root BIGINT UNSIGNED NOT NULL,  PRIMARY KEY `root2tag` (root, node));
    INSERT INTO root2tag (node, root) SELECT root, node FROM tag2root;
END $$

delimiter ;

## Usage

CALL descendant_tags_id_rooted(); # Populate

SELECT node FROM tag2root WHERE root IN (SELECT id FROM tag WHERE name='TAG');






DROP PROCEDURE IF EXISTS ancestor_tags_id_from;

delimiter $$

CREATE PROCEDURE ancestor_tags_id_rooted_from_id(tbl VARBINARY(1024),  tag_id INT)
BEGIN
    CALL ancestor_tags_id_rooted_init(tbl);
    set @query = concat("INSERT INTO ", tbl, " (node, depth) VALUES (", tag_id, ", 0);");
    PREPARE stmt FROM @query;
    EXECUTE stmt;
    
    
    DROP TABLE IF EXISTS _tmp;
    
    set @query = concat("CREATE TEMPORARY TABLE _tmp LIKE ", tbl, ";");
    PREPARE stmt FROM @query;
    EXECUTE stmt;
    
    
    CALL ancestor_tags_id_rooted_preseeded(tbl);
END $$

delimiter ;








# Get table of (file path, instance coords)  given tag names
# TODO: frame_n

SELECT name, tag_id, x, y, w, h
FROM file
JOIN (
    SELECT file_id, tag_id, x, y, w, h
    FROM instance
    JOIN (
        SELECT instance_id, tag_id
        FROM instance2tag
        WHERE tag_id IN (
            SELECT id
            FROM tag
            WHERE name IN ("tagname")
        )
    ) A ON A.instance_id = id
) B ON B.file_id = id
;

# The above, but including descendant tags (counted as separate tags)

CALL descendant_tags_id_from("tmptable", "'tagname'");

SELECT name, tag_id, x, y, w, h
FROM file
JOIN (
    SELECT file_id, tag_id, x, y, w, h
    FROM instance
    JOIN (
        SELECT instance_id, tag_id
        FROM instance2tag
        JOIN tmptable tt ON tt.node = tag_id
    ) A ON A.instance_id = id
) B ON B.file_id = id
;


# The above, but descendant tags counted as their heirarchical root

## Procedures

DROP PROCEDURE IF EXISTS descendant_tags_id_rooted_init;
DROP PROCEDURE IF EXISTS descendant_tags_id_rooted_preseeded;
DROP PROCEDURE IF EXISTS descendant_tags_id_rooted_from;

delimiter $$

CREATE PROCEDURE descendant_tags_id_rooted_init(tbl VARBINARY(1024))
BEGIN
    set @query = concat("DROP TABLE IF EXISTS ", tbl, ";");
    PREPARE stmt FROM @query;
    EXECUTE stmt;
    
    set @query = concat("CREATE TEMPORARY TABLE ", tbl, " (node BIGINT UNSIGNED PRIMARY KEY,  root BIGINT UNSIGNED NOT NULL);");
    PREPARE stmt FROM @query;
    EXECUTE stmt;
END $$

CREATE PROCEDURE descendant_tags_id_rooted_preseeded(tbl VARBINARY(1024))
BEGIN
    set @query_b = concat("INSERT IGNORE INTO _tmp SELECT tag_id, root FROM ", tbl, " JOIN tag2parent ON node = parent_id;");
    set @query_d = concat("INSERT IGNORE INTO ", tbl, " SELECT node, root FROM _tmp;");
    PREPARE stmt_b FROM @query_b;
    PREPARE stmt_d FROM @query_d;
    REPEAT
        TRUNCATE TABLE _tmp;
        EXECUTE stmt_b;
        EXECUTE stmt_d;
    UNTIL ROW_COUNT() = 0
    END REPEAT;
END $$

CREATE PROCEDURE descendant_tags_id_rooted_from(tbl VARBINARY(1024),  str VARBINARY(1024))
BEGIN
    CALL descendant_tags_id_rooted_init(tbl);
    set @query = concat("INSERT INTO ", tbl, " (node, root) SELECT id, id FROM tag WHERE name IN (", str, ");");
    PREPARE stmt FROM @query;
    EXECUTE stmt;
    
    
    DROP TABLE IF EXISTS _tmp;
    
    set @query = concat("CREATE TEMPORARY TABLE _tmp LIKE ", tbl, ";");
    PREPARE stmt FROM @query;
    EXECUTE stmt;
    
    
    CALL descendant_tags_id_rooted_preseeded(tbl);
END $$

delimiter ;


## Usage

CALL descendant_tags_id_rooted_from("tmptable", "'foo','bar'");

SELECT name, root, x, y, w, h
FROM file
JOIN (
    SELECT DISTINCT file_id, root, x, y, w, h
    FROM instance
    JOIN (
        SELECT instance_id, tt.root
        FROM instance2tag
        JOIN tmptable tt ON tt.node = tag_id
    ) A ON A.instance_id = id
) B ON B.file_id = id
;





# List combined coordinates (i.e. minimum spanning rectangle) in files covering combinations of instances with relations tagged RTAG where the master has the tag ITAG1 and the slave has the tag ITAG2

CALL descendant_tags_id_from("rtag", "'ActionTag'");
CALL descendant_tags_id_from("mtag", "'MasterTag(the doer)'");
CALL descendant_tags_id_from("stag", "'SlaveTag(the acted on object)'");

SELECT name, x, y, w, h
FROM file
JOIN (
    SELECT file_id, MIN(x) AS x, MIN(y) AS y, MAX(x+w) - MIN(x) AS w, MAX(h+y) - MIN(y) AS h
    FROM instance
    JOIN (
        SELECT master_id, slave_id
        FROM relation
        JOIN (
            SELECT relation_id
            FROM relation2tag
            WHERE tag_id IN (SELECT node FROM rtag)
        ) R2T ON R2T.relation_id = id
        WHERE master_id IN (
            SELECT instance_id
            FROM instance2tag
            WHERE tag_id IN (SELECT node FROM mtag)
        )
        AND slave_id IN (
            SELECT instance_id
            FROM instance2tag
            WHERE tag_id IN (SELECT node FROM stag)
        )
    ) A ON A.master_id = id
        OR A.slave_id = id
    GROUP BY file_id, A.slave_id
) B ON B.file_id = id
;















# List all parents of a tag (this heirarchy is a tree structure)

## Procedures

DROP PROCEDURE IF EXISTS ancestor_tags_id_rooted_init;
DROP PROCEDURE IF EXISTS ancestor_tags_id_rooted_preseeded;
DROP PROCEDURE IF EXISTS ancestor_tags_id_rooted_from_id;

delimiter $$

CREATE PROCEDURE ancestor_tags_id_rooted_init(tbl VARBINARY(1024))
BEGIN
    set @query = concat("DROP TABLE IF EXISTS ", tbl, ";");
    PREPARE stmt FROM @query;
    EXECUTE stmt;
    
    set @query = concat("CREATE TEMPORARY TABLE ", tbl, " (node BIGINT UNSIGNED,  parent BIGINT UNSIGNED NOT NULL,  PRIMARY KEY `node2parent` (`node`, `parent`));");
    PREPARE stmt FROM @query;
    EXECUTE stmt;
END $$

CREATE PROCEDURE ancestor_tags_id_rooted_preseeded(tbl VARBINARY(1024))
BEGIN
    set @query_b = concat("INSERT IGNORE INTO _tmp SELECT tag_id, parent_id FROM ", tbl, " JOIN tag2parent ON tag_id = parent;");
    set @query_d = concat("INSERT IGNORE INTO ", tbl, " SELECT node, parent FROM _tmp;");
    PREPARE stmt_b FROM @query_b;
    PREPARE stmt_d FROM @query_d;
    REPEAT
        TRUNCATE TABLE _tmp;
        EXECUTE stmt_b;
        EXECUTE stmt_d;
    UNTIL ROW_COUNT() = 0
    END REPEAT;
END $$

CREATE PROCEDURE ancestor_tags_id_rooted_from_id(tbl VARBINARY(1024),  tag_id INT)
BEGIN
    CALL ancestor_tags_id_rooted_init(tbl);
    set @query = concat("INSERT INTO ", tbl, " SELECT ", tag_id, ", parent_id FROM tag2parent WHERE tag_id=", tag_id, ";");
    PREPARE stmt FROM @query;
    EXECUTE stmt;
    
    
    DROP TABLE IF EXISTS _tmp;
    
    set @query = concat("CREATE TEMPORARY TABLE _tmp LIKE ", tbl, ";");
    PREPARE stmt FROM @query;
    EXECUTE stmt;
    
    
    CALL ancestor_tags_id_rooted_preseeded(tbl);
END $$

delimiter ;


CALL ancestor_tags_id_rooted_from_id("foobar", 90);
SELECT * FROM foobar;
