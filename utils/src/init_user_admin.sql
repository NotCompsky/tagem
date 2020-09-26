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

// NOTE: This account is necessary for 'post__ytdl_era'

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
	add_tags=TRUE,
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
