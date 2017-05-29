/**
 * Create NeFo stored function and views 
 * CREATE FUNCTION and CREATE VIEW privileges are required.
 */

--
--  STEP 01: Create function forschungsatlas__exports_getFamilyTreeIds
--
DELIMITER $$
DROP FUNCTION IF EXISTS `forschungsatlas__tools_getFamilyTreeIds`$$
CREATE FUNCTION `forschungsatlas__getFamilyTreeIds`(parentid VARCHAR(16)) RETURNS varchar(1024) CHARSET utf8
    DETERMINISTIC
BEGIN
    DECLARE list_csv_ids VARCHAR(1024);
    DECLARE rparent VARCHAR(1024);
    DECLARE queue, subqueue, childrenqueue VARCHAR(1024);
    DECLARE qlength, pos INT;

    SET list_csv_ids = NULL;
    
    IF parentid IS NOT NULL THEN
        SET list_csv_ids = '';
        SET queue = parentid;
        SET qlength = 1;

        WHILE qlength > 0 DO
            SET rparent = queue;
            IF qlength = 1 THEN
                SET queue = '';
            ELSE
                SET pos = LOCATE(',', queue) + 1;
                SET subqueue = SUBSTR(queue, pos);
                SET queue = subqueue;
            END IF;
            SET qlength = qlength - 1;

            SELECT IFNULL(qc, '') INTO childrenqueue FROM (SELECT GROUP_CONCAT(iid) qc  FROM forschungsatlas__institution_institution WHERE linkid = rparent) A;
                IF LENGTH(childrenqueue) = 0 THEN
                        IF LENGTH(queue) = 0 THEN
                            SET qlength = 0;
                        END IF;
                ELSE
                        IF LENGTH(list_csv_ids) = 0 THEN
                            SET list_csv_ids = childrenqueue;
                        ELSE
                            SET list_csv_ids = CONCAT(list_csv_ids, ',', childrenqueue);
                        END IF;
                        IF LENGTH(queue) = 0 THEN
                            SET queue = childrenqueue;
                        ELSE
                            SET queue = CONCAT(queue, ',', childrenqueue);
                        END IF;
                        SET qlength = LENGTH(queue) - LENGTH(REPLACE(queue, ',', '')) + 1;
                END IF;
        END WHILE;
        IF LENGTH(list_csv_ids) = 0 THEN
            SET list_csv_ids = parentid;
        ELSE
            SET list_csv_ids = CONCAT(parentid, ',', list_csv_ids);
        END IF;    
    END IF;
    
    RETURN list_csv_ids;
END$$
DELIMITER ;

--
--  STEP 02: forschungsatlas__exports_institutions_view
--
CREATE OR REPLACE VIEW forschungsatlas__exports_institutions_view AS
    SELECT 
        i.iid AS iid,
        i.name AS name,
        IFNULL(i.abbrev, '') AS abbrev,
        l.linkid AS linkid,
        FORSCHUNGSATLAS__GETFAMILYTREEIDS(i.iid) AS familytreeids
   FROM
        forschungsatlas__institutions i
            LEFT JOIN
        forschungsatlas__institution_institution AS l ON i.iid = l.iid
    WHERE
        (i.status = 1) AND (linkid IS NULL)
    ORDER BY name;


