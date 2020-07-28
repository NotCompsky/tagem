/*
-- Copyright 2020 Adam Gray
-- This file is part of the tagem program.
-- The tagem program is free software: you can redistribute it and/or
-- modify it under the terms of the GNU General Public License as published by the
-- Free Software Foundation version 3 of the License.
-- The tagem program is distributed in the hope that it will be useful, but
-- WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
-- FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
-- This copyright notice should be included in any copy or substantial copy of the tagem source code.
-- The absense of this copyright notices on some other files in this project does not indicate that those files do not also fall under this license, unless they have a different license written at the top of the file.
*/

R"=====(

delimiter $$

CREATE PROCEDURE IF NOT EXISTS gen_tag2parent_tree()
BEGIN
    TRUNCATE TABLE tag2parent_tree;
    INSERT INTO tag2parent_tree (id, parent, depth) SELECT id, id, 0 FROM tag;
    
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





delimiter $$
CREATE PROCEDURE IF NOT EXISTS fill_dir2parent_tree()
BEGIN
    DROP TABLE IF EXISTS _tmp;
    CREATE TEMPORARY TABLE _tmp LIKE dir2parent_tree;
    REPEAT
        TRUNCATE TABLE _tmp;
        
        INSERT INTO _tmp
        SELECT t2pt.id, d.parent, t2pt.depth+1
        FROM dir2parent_tree t2pt
        JOIN dir d ON d.id=t2pt.parent
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





delimiter $$

CREATE PROCEDURE IF NOT EXISTS gen_dir2parent_tree()
BEGIN
    TRUNCATE TABLE dir2parent_tree;
    INSERT INTO dir2parent_tree (id, parent, depth) SELECT id, id, 0 FROM dir;
    CALL fill_dir2parent_tree();
END $$

delimiter ;


CREATE PROCEDURE IF NOT EXISTS tagem_db_initialised()
BEGIN
END
;

)====="
