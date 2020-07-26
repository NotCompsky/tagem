-- WARNING: This account is advisable. The ID of 0 must be reserved, and foreign keys may refer to this ID.


INSERT INTO user
(id,name)
VALUES
(99,"Invalid")
ON DUPLICATE KEY UPDATE id=id;


UPDATE user SET id=0 WHERE name="Invalid";
