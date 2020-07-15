DROP PROCEDURE IF EXISTS gen_tag2parent_tree;

delimiter $$

CREATE PROCEDURE gen_tag2parent_tree()
BEGIN
    TRUNCATE TABLE tag2parent_tree;
    INSERT INTO tag2parent_tree (id, parent, depth) SELECT id, id, 0 FROM _tag;
    
    DROP TABLE IF EXISTS _tmp;
    CREATE TEMPORARY TABLE _tmp LIKE tag2parent_tree;
    REPEAT
        TRUNCATE TABLE _tmp;
        INSERT INTO _tmp SELECT t2pt.id, t2p.parent, t2pt.depth+1 FROM tag2parent_tree t2pt JOIN tag2parent t2p ON t2p.id=t2pt.parent ON DUPLICATE KEY UPDATE depth=LEAST(_tmp.depth, t2pt.depth+1);
        INSERT INTO tag2parent_tree (id,parent,depth) SELECT t.id, t.parent, t.depth FROM _tmp t ON DUPLICATE KEY UPDATE depth=LEAST(tag2parent_tree.depth, t.depth);
    UNTIL ROW_COUNT() = 0
    END REPEAT;
END $$

delimiter ;





DROP PROCEDURE IF EXISTS fill_dir2parent_tree;
delimiter $$
CREATE PROCEDURE fill_dir2parent_tree()
BEGIN
    DROP TABLE IF EXISTS _tmp;
    CREATE TEMPORARY TABLE _tmp LIKE dir2parent_tree;
    REPEAT
        TRUNCATE TABLE _tmp;
        
        INSERT INTO _tmp
        SELECT t2pt.id, d.parent, t2pt.depth+1
        FROM dir2parent_tree t2pt
        JOIN _dir d ON d.id=t2pt.parent
        WHERE d.parent IS NOT NULL
        ON DUPLICATE KEY UPDATE depth=LEAST(_tmp.depth, t2pt.depth+1);
        
        INSERT INTO dir2parent_tree
        (id,parent,depth)
        SELECT t.id, t.parent, t.depth
        FROM _tmp t
        ON DUPLICATE KEY UPDATE depth=LEAST(dir2parent_tree.depth, t.depth);
    UNTIL ROW_COUNT() = 0
    END REPEAT;
END $$
delimiter ;




DROP PROCEDURE IF EXISTS gen_dir2parent_tree;

delimiter $$

CREATE PROCEDURE gen_dir2parent_tree()
BEGIN
    TRUNCATE TABLE dir2parent_tree;
    INSERT INTO dir2parent_tree (id, parent, depth) SELECT id, id, 0 FROM _dir;
    CALL fill_dir2parent_tree();
END $$

delimiter ;
