/**
 * Create NeFo stored functions and views.
 * CREATE FUNCTION and CREATE VIEW privileges are required.
 */

--
--  STEP 01: Create function forschungsatlas__getAssembledParentName
--
DELIMITER $$
DROP FUNCTION IF EXISTS forschungsatlas__getAssembledParentName $$
CREATE FUNCTION forschungsatlas__getAssembledParentName (parentid VARCHAR(16))
RETURNS VARCHAR(2048) CHARSET utf8
DETERMINISTIC
BEGIN
    DECLARE comp_name VARCHAR(2048);
    DECLARE rname VARCHAR(255);
    DECLARE rparent, rchild VARCHAR(16);
    DECLARE searchparent, num_rows INT;
    SET rname = NULL;
    SET comp_name = NULL;
    IF parentid IS NOT NULL THEN
        SELECT CONCAT(ti.name, 0x1F4e45464f49441F, ti.iid) AS rname INTO rname
            FROM forschungsatlas__institutions AS ti WHERE ti.iid = parentid;
        IF rname IS NOT NULL THEN
            SET comp_name = rname;
        END IF;
        SET searchparent = 1;
        SET rparent = parentid;
        WHILE searchparent > 0 DO
            SET rname = NULL;
            SELECT IFNULL(rc, '') INTO rchild
                FROM (SELECT tii.linkid AS rc
                          FROM forschungsatlas__institution_institution AS tii
                          WHERE tii.iid = rparent AND tii.delta = 0 LIMIT 1) AS A;
            SET num_rows = found_rows();
            IF num_rows > 0 THEN
                IF LENGTH(rchild) = 0 THEN
                    SET searchparent = 0;
                ELSE
                    SET rparent = rchild;
                    SELECT CONCAT(ti.name, 0x1F4e45464f49441F, ti.iid) AS rname INTO rname
                        FROM forschungsatlas__institutions AS ti WHERE ti.iid = rparent;
                    IF rname IS NOT NULL THEN
                        SET comp_name = CONCAT(rname, 0x1F4E45464F4E4C1F, comp_name);
                    END IF;
                END IF;
            ELSE
                SET searchparent = 0;
            END IF;
        END WHILE;
    END IF;
    RETURN comp_name;
END $$
DELIMITER ;

--
--  STEP 02: Create function forschungsatlas__getCompositedName
--
DELIMITER $$
DROP FUNCTION IF EXISTS forschungsatlas__getCompositedName $$
CREATE FUNCTION forschungsatlas__getCompositedName (childid VARCHAR(16), parentid VARCHAR(16))
RETURNS VARCHAR(2048) CHARSET utf8
DETERMINISTIC
BEGIN
    DECLARE comp_name VARCHAR(2048);
    DECLARE rname VARCHAR(255);
    DECLARE rparent, rchild VARCHAR(16);
    DECLARE searchparent, num_rows INT;
    SET rname = NULL;
    SELECT ti.name INTO rname FROM forschungsatlas__institutions AS ti WHERE ti.iid = childid;
    IF rname IS NOT NULL THEN
        SET comp_name = rname;
        SET rname = NULL;
    END IF;
    IF parentid IS NOT NULL THEN
        SELECT ti.name INTO rname
            FROM forschungsatlas__institutions AS ti WHERE ti.iid = parentid;
        IF rname IS NOT NULL THEN
            SET comp_name = CONCAT(rname, ' > ', comp_name);
        END IF;
        SET searchparent = 1;
        SET rparent = parentid;
        WHILE searchparent > 0 DO
            SET rname = NULL;
            SELECT IFNULL(rc, '') INTO rchild
                FROM (SELECT tii.linkid AS rc
                          FROM forschungsatlas__institution_institution AS tii
                          WHERE tii.iid = rparent AND tii.delta = 0 LIMIT 1) AS A;
            SET num_rows = found_rows();
            IF num_rows > 0 THEN
                IF LENGTH(rchild) = 0 THEN
                    SET searchparent = 0;
                ELSE
                    SET rparent = rchild;
                    SELECT ti.name INTO rname
                        FROM forschungsatlas__institutions AS ti WHERE ti.iid = rparent;
                    IF rname IS NOT NULL THEN
                        SET comp_name = CONCAT(rname, ' > ', comp_name);
                    END IF;
                END IF;
            ELSE
                SET searchparent = 0;
            END IF;
        END WHILE;
    END IF;
    RETURN comp_name;
END $$
DELIMITER ;

--
--  STEP 03: Create function forschungsatlas__getFamilyTreeIds
--
DELIMITER $$
DROP FUNCTION IF EXISTS `forschungsatlas__getFamilyTreeIds`$$
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
--  STEP 04: forschungsatlas__aux_institutions_view
--
CREATE OR REPLACE VIEW forschungsatlas__aux_institutions_view AS
    SELECT
        i.iid AS iid,
        i.name AS name,
        i.street AS street,
        i.postalcode AS postalcode,
        i.cid AS cid,
        c.name AS city,
        i.fsid AS fsid,
        fs.name AS federalstate,
        i.url AS url,
        i.otid AS otid,
        ot.name AS orgtype,
        g.geolocation_geo_type AS geotype,
        g.geolocation_lat AS latitude,
        g.geolocation_lon AS longitude,
        g.geolocation_wkt AS wkt,
        FORSCHUNGSATLAS__GETCOMPOSITEDNAME(i.iid, l.linkid) AS compositedname,
        FORSCHUNGSATLAS__GETASSEMBLEDPARENTNAME(l.linkid) AS assembledparentname,
        i.abbrev AS abbrev,
        FORSCHUNGSATLAS__GETFAMILYTREEIDS(i.iid) AS familytreeids,
        CONCAT_WS(' ', i.name, i.abbrev) AS fullname
   FROM
        forschungsatlas__institutions i
            LEFT JOIN
        forschungsatlas__institution_institution AS l ON i.iid = l.iid
           LEFT JOIN
        forschungsatlas__cities c ON i.cid = c.cid
            LEFT JOIN
        forschungsatlas__federal_states fs ON i.fsid = fs.fsid
           LEFT JOIN
        forschungsatlas__organization_type ot ON i.otid = ot.otid
           LEFT JOIN
        forschungsatlas__institution_geolocation g ON i.iid = g.iid
    WHERE
        i.status = 1;




