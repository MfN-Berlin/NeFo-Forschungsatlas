<?php
/**
 * @file
 * forschungsatlas.institution.inc
 */

/**
 *  Menu callback. Show the institution.
 *
 *  @param $institution
 *  An associative array containing the institution.
 *
 *  @see forschungsatlas_menu()
 */
function forschungsatlas_institution_show($institution=NULL) {
  drupal_set_title($institution['name']);

  $iid = intval($institution['iid']);
  $variables = array('forschungsatlas_institution' => new stdClass());
  // Name
  $variables['forschungsatlas_institution']->name = $institution['name'];
  // Abbreviation
  $variables['forschungsatlas_institution']->abbrev = $institution['abbrev'];
  // Address
  $variables['forschungsatlas_institution']->address = array(
      'street' => $institution['street'],
      'postalcode' => $institution['postalcode'],
      'city' => $institution['city'],
      'federalstate' => $institution['federalstate'],
  );
  // URL
  $variables['forschungsatlas_institution']->url = $institution['url'];
  // Organization Type
  $tnorgtype = _forschungsatlas_build_list_translation(array($institution['otid'] => $institution['orgtype']), 'forschungsatlas_orgtype:orgtype');
  $variables['forschungsatlas_institution']->orgtype = $tnorgtype[$institution['otid']];
  // Belonging
  $variables['forschungsatlas_institution']->belonging = _forschungsatlas_get_belonging_list($iid);
  // Research Network
  $variables['forschungsatlas_institution']->network = _forschungsatlas_get_network_list($iid);
  // Organisms
  $variables['forschungsatlas_institution']->organisms = _forschungsatlas_get_organisms_list($iid);
  // Habitats
  $variables['forschungsatlas_institution']->habitats = _forschungsatlas_get_habitats_list($iid);
  // Research Topics
  $variables['forschungsatlas_institution']->topics = _forschungsatlas_get_topics_list($iid);
  // Interdisciplinary Subjects
  $variables['forschungsatlas_institution']->intsubjects = _forschungsatlas_get_intsubjects_list($iid);
  // Methodologies
  $variables['forschungsatlas_institution']->methodologies = _forschungsatlas_get_methodologies_list($iid);
  // Comment
  $variables['forschungsatlas_institution']->description = $institution['description'];
  // Subordinate institutions
  $variables['forschungsatlas_institution']->subdivision = _forschungsatlas_get_subordinate_list($iid);
  // Last changed
  $variables['forschungsatlas_institution']->changed = format_date($institution['changed'], 'custom', FORSCHUNGSATLAS_DATE_FORMAT);

  return theme('forschungsatlas_display_institution', $variables);
}

/**
 * Build a list of institutions to which the given institution is belonging.
 *
 * @param integer $iid
 *  The institution ID.
 *
 * @return
 *  An array containing the list of institutions.
 */
function _forschungsatlas_get_belonging_list($iid) {
  $list = array();
  $result = db_query('SELECT FORSCHUNGSATLAS__GETCOMPOSITEDNAME(i.iid, l.parent) AS compositedname,
        FORSCHUNGSATLAS__GETASSEMBLEDPARENTNAME(l.parent) AS assembledparentname
      FROM {forschungsatlas__institutions} AS i
      LEFT JOIN {forschungsatlas__institution_institution} AS l ON i.iid = l.iid
      WHERE i.iid = :iid ORDER BY compositedname', array(':iid' => $iid));

  foreach ($result as $row) {
    if (!empty($row->assembledparentname)) {
      $list[] = _forschungsatlas_get_belonging_assembled_name($row->assembledparentname);
    }
  }

  return $list;
}

/**
 * Build a list of subordinate institutions.
 *
 * @param integer $iid
 *  The institution ID.
 *
 * @return
 *  An array containing the list of institutions.
 */
function _forschungsatlas_get_subordinate_list($iid) {
  $subordinates = _forschungsatlas_get_subordinates($iid);
  $list = _forschungsatlas_format_subordinates($subordinates);

  return $list;
}

/**
 * Format the subdivision strings for the display.
 * @param $subordinates
 *  Array of subdivisions.
 *
 * @return $list
 *  Array containing the prepared subdivisions.
 */
function _forschungsatlas_format_subordinates($subordinates) {
  $list = array();
  foreach ($subordinates as $inst) {
    $list[] = array(
        'institution' => $inst['child']->iid .
            FORSCHUNGSATLAS_DELIMITER_ID .
            $inst['child']->name.
            FORSCHUNGSATLAS_DELIMITER_ID .
            $inst['child']->url,
        'child' => empty($inst['subchildren']) ? '' : _forschungsatlas_format_subordinates($inst['subchildren']),
    );
  }

  return $list;
}

/**
 * Recursive search all subordinate institutions.
 * @param integer $iid
 *  The parent institution ID.
 *
 * @return $subordinates
 *  An array containing the subordinate institutions.
 */
function _forschungsatlas_get_subordinates($iid) {
  $subordinates = array();
  $children = _forschungsatlas_get_child_institutions($iid);
  foreach ($children AS $child) {
    $subchildren = _forschungsatlas_get_subordinates($child->iid);
    $subordinates[] = array('child' => $child, 'subchildren' => $subchildren);
  }

  return $subordinates;
}

/**
 * Get all children institutions from the database.
 * @param integer $iid
 *  The parent institution ID.
 *
 * @return $result
 *  An associative array, or FALSE if there is no next row.
 */
function _forschungsatlas_get_child_institutions($iid) {
  $result = db_query('SELECT l.iid AS iid, i.name AS name, i.url AS url
      FROM {forschungsatlas__institution_institution} AS l
              JOIN
           {forschungsatlas__institutions} AS i ON l.iid = i.iid
      WHERE l.parent = :iid ORDER BY i.name', array(':iid' => $iid)
  )->fetchAll();

  return $result;
}

/**
 * Build the assembled name of institutions to which the given institution is belonging.
 *
 * @param string $parentname
 *  The parent name
 *
 * @return  string $assembled_name
 *  The assembled parent name
 */
function _forschungsatlas_get_belonging_assembled_name($parentname) {
  $parents = array();
  $list = explode(FORSCHUNGSATLAS_DELIMITER_NEWLINE, $parentname);
  foreach ($list AS $value) {
    list($rname, $riid) = explode(FORSCHUNGSATLAS_DELIMITER_ID, $value);
    $parents[] = l(
            $rname,
            FORSCHUNGSATLAS_PUBLIC_PATH . '/institutions/'. $riid,
            array(
                'attributes' => array(
                    'class' => 'forschungsatlas-institution-l',
                )
           )
    );
  } //foreach()
  $assembled_name = implode(' '. FORSCHUNGSATLAS_SEPARATOR .' ', $parents);

  return $assembled_name;
}

/**
 * Build a list of names associated with an institution.
 *
 * @param interger $iid
 *  The institution ID
 * @param string $mapping_table
 *  Mapping table name
 * @param string $table
 *  Table name
 * @param string $col_name
 *  Column name
 *
 * @return
 *  An array in the form array(id => name).
 */
function _forschungsatlas_get_list($iid, $mapping_table, $table, $col_name) {
  $list = array();
  $result = db_query('SELECT a.'. $col_name .' AS '. $col_name .', b.name AS name
      FROM {'. $mapping_table .'} AS a, {'. $table .'} AS b
      WHERE a.iid=:iid AND a.'. $col_name .' = b.'. $col_name, array(':iid' => $iid));
  foreach ($result as $row) {
    $list[$row->$col_name] = $row->name;
  }

  return $list;
}

/**
 * Build a list of research networks associated with the given institution.
 *
 * @param integer $iid
 *  The institution ID.
 *
 * @return
 *  An array containing the list of research networks.
 */
function _forschungsatlas_get_network_list($iid) {
  $list = _forschungsatlas_get_list($iid, 'forschungsatlas__institution_researchnetwork', 'forschungsatlas__research_network', 'rnid');
  return _forschungsatlas_build_list_translation($list, 'forschungsatlas_network:researchnetwork');
}

/**
 * Build a list of organisms associated with the given institution.
 *
 * @param integer $iid
 *  The institution ID.
 *
 * @return
 *  An array containing the list of organisms.
 */
function _forschungsatlas_get_organisms_list($iid) {
  $list = _forschungsatlas_get_list($iid, 'forschungsatlas__institution_organism', 'forschungsatlas__organisms', 'oid');
  return _forschungsatlas_build_list_translation($list, 'forschungsatlas_organism:organism');
}

/**
 * Build a list of habitats associated with the given institution.
 *
 * @param integer $iid
 *  The institution ID.
 *
 * @return
 *  An array containing the list of habitats.
 */
function _forschungsatlas_get_habitats_list($iid) {
  $list = _forschungsatlas_get_list($iid, 'forschungsatlas__institution_habitat', 'forschungsatlas__habitats', 'hid');
  return _forschungsatlas_build_list_translation($list, 'forschungsatlas_habitat:habitat');
}

/**
 * Build a list of research topics associated with the given institution.
 *
 * @param integer $iid
 *  The institution ID.
 *
 * @return
 *  An array containing the list of research topics.
 */
function _forschungsatlas_get_topics_list($iid) {
  $list = _forschungsatlas_get_list($iid, 'forschungsatlas__institution_topic', 'forschungsatlas__topics', 'tid');
  return _forschungsatlas_build_list_translation($list, 'forschungsatlas_topic:topic');
}

/**
 * Build a list of interdisciplinary subjects associated with the given institution.
 *
 * @param integer $iid
 *  The institution ID.
 *
 * @return
 *  An array containing the list of interdisciplinary subjects.
 */
function _forschungsatlas_get_intsubjects_list($iid) {
  $list = _forschungsatlas_get_list($iid, 'forschungsatlas__institution_intersubject', 'forschungsatlas__interdisciplinary_subject', 'isid');
  return _forschungsatlas_build_list_translation($list, 'forschungsatlas_intersubject:interdisciplinarysubject');
}

/**
 * Build a list of methodologies associated with the given institution.
 *
 * @param integer $iid
 *  The institution ID.
 *
 * @return
 *  An array containing the list of methodologies.
 */
function _forschungsatlas_get_methodologies_list($iid) {
  $list = _forschungsatlas_get_list($iid, 'forschungsatlas__institution_methodology', 'forschungsatlas__methodology', 'mid');
  return _forschungsatlas_build_list_translation($list, 'forschungsatlas_methodology:methodology');
}

/**
 * Translate a list of names associated with an institution.
 *
 * @param array $list
 *  An array in the form array(key => name)
 * @param strin $object
 *  i18n Object
 *
 * @return
 *  An array in the form array(key => name translation).
 */
function _forschungsatlas_build_list_translation($list, $object) {
  $tnlist = array();
  $comp_options = array();

  foreach ($list as $key => $value) {
    if (module_exists('i18n_string') && module_exists('i18n_forschungsatlas')) {
      $tnname = i18n_string("$object:$key:name", $value);
    }
    else {
      $tnname = $value;
    }
    $comp_options[$tnname] = array('key' => $key, 'name' => $tnname);
  }
  ksort($comp_options);

  foreach ($comp_options as $row) {
    $tnlist[$row['key']] = $row['name'];
  }

  return $tnlist;
}



