SELECT t.name, f.name, t2f.created_on, t.added_on FROM tag t JOIN tag2file t2f ON t.id=t2f.tag_id JOIN file f ON f.id=t2f.file_id;


# Delete tag2file entries that reference a tag that didn't exist at the time of creation

DELETE t2f FROM tag t JOIN tag2file t2f ON t.id=t2f.tag_id JOIN file f ON f.id=t2f.file_id WHERE t2f.created_on < t.added_on;




# List files ordered by score

SELECT name, score FROM file ORDER BY score DESC;


# List most common tags

SELECT t.name, COUNT(f2t.tag_id) AS c FROM tag t JOIN file2tag f2t ON t.id=f2t.tag_id GROUP BY t.id ORDER BY c ASC;




# List all files tagged with 'TAGNAME'

SELECT f.name FROM file f JOIN (SELECT f2t.file_id FROM file2tag f2t JOIN (SELECT t.id FROM tag t WHERE t.name = 'TAGNAME') T on f2t.tag_id = T.id) F on f.id = F.file_id;




# List all files tagged with at least one of TAG1 or TAG2

SELECT f.name FROM file f JOIN (SELECT f2t.file_id FROM file2tag f2t JOIN (SELECT t.id FROM tag t WHERE t.name IN ('TAG1', 'TAG2')) T on f2t.tag_id = T.id) F on f.id = F.file_id;

SELECT f.name
FROM file f
JOIN (
    SELECT f2t.file_id
    FROM file2tag f2t
    JOIN (
        SELECT t.id
        FROM tag t
        WHERE t.name
        IN ('TAG1','TAG2')
    ) T ON f2t.tag_id = T.id
) F on f.id = F.file_id;




# List all files tagged with ((TAG1 or TAG2) and TAG3)

SELECT f.name
FROM file f
JOIN (
    SELECT f2t.file_id
    FROM file2tag f2t
    JOIN (
        SELECT t.id
        FROM tag t
        WHERE t.name
        IN ('TAG1','TAG2')
    ) T ON f2t.tag_id = T.id
) F on f.id = F.file_id
WHERE f.id IN (
    SELECT f2.id
    FROM file f2
    JOIN (
        SELECT f2t2.file_id
        FROM file2tag f2t2
        JOIN (
            SELECT t2.id
            FROM tag t2
            WHERE t2.name
            IN ('TAG3')
        ) T2 ON f2t2.tag_id = T2.id
    ) F2 on f2.id = F2.file_id
)
;

*~4 times faster*:
SELECT f.name
FROM file f
JOIN (
    SELECT file_id
    FROM (
        (
            SELECT DISTINCT f2t.file_id
            FROM file2tag f2t
            JOIN (
                SELECT t.id
                FROM tag t
                WHERE t.name
                IN ('TAG1','TAG2')
            ) T ON f2t.tag_id = T.id
        )
        UNION ALL
        (
            SELECT DISTINCT f2t.file_id
            FROM file2tag f2t
            JOIN (
                SELECT t.id
                FROM tag t
                WHERE t.name
                IN ('TAG3')
            ) T ON f2t.tag_id = T.id
        )
    )
    AS u
    GROUP BY file_id
    HAVING count(*) >= 2
) F ON f.id = F.file_id
;
Note that the `DISTINCT` is necessary iff we have multiple tags in the union (so here we could do away with it for the `sfm` subtable).
    

# List all files tagged with at least two of ((TAG1 or TAG2) and (TAG3) and (TAG4))
SELECT f.name
FROM file f
JOIN (
    SELECT file_id
    FROM (
        (
            SELECT DISTINCT f2t.file_id
            FROM file2tag f2t
            JOIN (
                SELECT t.id
                FROM tag t
                WHERE t.name
                IN ('TAG1','TAG2')
            ) T ON f2t.tag_id = T.id
        )
        UNION ALL
        (
            SELECT DISTINCT f2t.file_id
            FROM file2tag f2t
            JOIN (
                SELECT t.id
                FROM tag t
                WHERE t.name
                IN ('TAG3', 'TAG4', 'TAG5')
            ) T ON f2t.tag_id = T.id
        )
        UNION ALL
        (
            SELECT DISTINCT f2t.file_id
            FROM file2tag f2t
            JOIN (
                SELECT t.id
                FROM tag t
                WHERE t.name
                IN ('TAG6', 'TAG7', 'TAG8')
            ) T ON f2t.tag_id = T.id
        )
    )
    AS u
    GROUP BY file_id
    HAVING count(*) >= 2
) F ON f.id = F.file_id
;
