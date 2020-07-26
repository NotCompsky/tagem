-- NOTE: This account is entirely optional.


INSERT INTO user
(id,name)
VALUES
(2,"Admin")
ON DUPLICATE KEY UPDATE id=id;


UPDATE user
SET
	exec_safe_sql_cmds=TRUE,
	exec_unsafe_sql_cmds=TRUE,
	exec_safe_tasks=TRUE,
	exec_unsafe_tasks=TRUE
WHERE name="Admin"
;
