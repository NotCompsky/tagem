CREATE TRIGGER honour_the_ancestors 
AFTER INSERT ON dir
FOR EACH ROW
	INSERT INTO dir2parent_tree
	(id,parent,depth)
	SELECT new.id, d2pt.parent, d2pt.depth+1
	FROM dir2parent_tree d2pt
	WHERE d2pt.id=new.parent
	UNION
	SELECT new.id, new.id, 0
;
