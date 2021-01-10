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

INSERT INTO protocol (id, name) VALUES
(0, "NONE!"),
(1, "file://"),
(2, "http://"),
(3, "https://"),
(4, "youtube-dl"),
(5, "magnet:?")
ON DUPLICATE KEY UPDATE id=id;


INSERT INTO device (name,user,protocol,embed_pre,embed_post) VALUES
("https://youtube.com/watch?v=",2,(SELECT id FROM protocol WHERE name='youtube-dl'), 'https://www.youtube.com/embed/', '?enablejsapi=1'),
("https://",2,(SELECT id FROM protocol WHERE name="https://"),"",""),
("https://twitter.com/",2,(SELECT id FROM protocol WHERE name='https://'), '<blockquote class="twitter-tweet"><a href="https://twitter.com/AnyUsernameWorksHere/status/', '?ref_src=twsrc%5Etfw">Link</a></blockquote><script async src="https://platform.twitter.com/widgets.js" charset="utf-8"></script>'),
("magnet:?",2,5,"","")
ON DUPLICATE KEY UPDATE name=name;
-- WARNING: The device IDs are assumed in the scripts, so these must be inserted in this order even if they are unused.


INSERT INTO device (name,protocol,user) VALUES
("https://www.google.com/", (SELECT id FROM protocol WHERE name="https://"),2),
("https://stackoverflow.com/", (SELECT id FROM protocol WHERE name="https://"),2),
("https://en.wikipedia.org/", (SELECT id FROM protocol WHERE name="https://"),2),
("https://github.com/", (SELECT id FROM protocol WHERE name="https://"),2)
ON DUPLICATE KEY UPDATE protocol=protocol;


SET FOREIGN_KEY_CHECKS=0;
INSERT INTO dir
(id,parent,device,user,name,full_path)
VALUES
(1,NULL,2,2,"https://","https://"),
(2,1,1,2,"www.youtube.com/","https://www.youtube.com/"),
(3,2,1,2,"watch?v=","https://www.youtube.com/watch?v="),
(4,NULL,4,2,"magnet:?","magnet:?")
ON DUPLICATE KEY UPDATE parent=parent
;
SET FOREIGN_KEY_CHECKS=1;

SET @i := -1;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"!!NONE!!") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"audio/aac") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"image/bmp") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"text/css") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"text/csv") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"image/gif") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"text/html") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"image/jpeg") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"textjavascript") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"application/json") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"audio/mpeg") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"video/mpeg") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"audio/ogg") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"video/ogg") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"audio/opus") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"image/png") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"image/tiff") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"text/plain") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"audio/wav") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"audio/webm") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"video/webm") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"image/webp") ON DUPLICATE KEY UPDATE id=id;
INSERT INTO mimetype (id,name) VALUES (@i:=@i+1,"video/avi") ON DUPLICATE KEY UPDATE id=id;


INSERT INTO ext2mimetype
(name,id)
SELECT REGEXP_REPLACE(SUBSTRING_INDEX(name, '/', -1), '^x-', ''), id
FROM mimetype
WHERE SUBSTRING_INDEX(name, '/', -1) NOT IN ("plain","mpeg","webm","javascript","!!NONE!!","quicktime","x-ms-asf")
  AND name NOT IN ("video/ogg")
UNION
SELECT "mp4", id FROM mimetype WHERE name='video/mp4'
UNION
SELECT "mp3", id FROM mimetype WHERE name='audio/mp3'
UNION
SELECT "jpg", id FROM mimetype WHERE name='image/jpeg'
UNION
SELECT "webm", id FROM mimetype WHERE name='video/webm'
ON DUPLICATE KEY UPDATE id=VALUES(id)
;


INSERT INTO file2
(id,min, max, conversion, name)
VALUES
(1,0, 100, 0, "Score")
ON DUPLICATE KEY UPDATE id=id;


INSERT INTO tag (id,name,user)
VALUES
(1,"!!PART OF FILE!!",2),
(0,"!!ROOT TAG!!",2)
ON DUPLICATE KEY UPDATE id=id;
UPDATE tag SET id=0 WHERE name="!!ROOT TAG!!";


INSERT IGNORE INTO method (name) VALUES
("next_subtitle"),
("wipe_subtitle"),
("skip"),
("menu"),
("python_script")
ON DUPLICATE KEY UPDATE name=name;


INSERT INTO operators
(id, string)
VALUES
(0,"AND"),
(1,"OR"),
(2,"XOR"),
(3,"NOT")
ON DUPLICATE KEY UPDATE id=id;

)====="
