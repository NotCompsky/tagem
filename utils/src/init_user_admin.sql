// NOTE: This account is entirely optional.

R"=====(

INSERT INTO user
(id,name)
VALUES
(2,"Admin")
ON DUPLICATE KEY UPDATE id=id;


UPDATE user
SET
	exec_qry=TRUE,
	exec_safe_sql_cmds=TRUE,
	exec_unsafe_sql_cmds=TRUE,
	exec_safe_tasks=TRUE,
	exec_unsafe_tasks=TRUE,
	exec_safe_tasks=TRUE,
	edit_tasks=TRUE,
	link_tags=TRUE,
	unlink_tags=TRUE,
	edit_tags=TRUE,
	merge_files=TRUE,
	backup_files=TRUE,
	add_files=TRUE,
	create_files=TRUE,
	edit_names=TRUE,
	add_eras=TRUE,
	record_local_fs=TRUE,
	edit_users=TRUE
WHERE name="Admin"
;

)====="
