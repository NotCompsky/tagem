-- WARNING: The Guest account must exist. It is the default account for non-logged-in users. It's ID must be 1 - however, its name is arbitrary.

INSERT INTO user
(id,name)
VALUES
(1,"Guest")
ON DUPLICATE KEY UPDATE id=id;


INSERT INTO user2shown_file2
(user,file2)
SELECT id, 1
FROM user
ON DUPLICATE KEY UPDATE user=user;
