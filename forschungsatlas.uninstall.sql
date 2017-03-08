/**
 * Remove NeFo stored functions and views. 
 * DROP FUNCTION and DROP VIEW privileges are required.
 */

DROP VIEW IF EXISTS forschungsatlas__aux_institutions_view;

DROP FUNCTION IF EXISTS forschungsatlas__getCompositedName;

DROP FUNCTION IF EXISTS forschungsatlas__getAssembledParentName;


