<?php
/**
 * @file
 * forschungsatlas_exports.admin.inc
 */

/**
 * Table builder
 */
function forschungsatlas_exports_list() {
  $destination = drupal_get_destination();

  drupal_set_title(t('Database exports'));
  $header = array();
  $header[] = array('data' => t('Category'));
  $header[] = array('data' => t('Operation'));

  $rows = array();
  $result = forschungsatlas_exports_actions();
  foreach ($result as $key => $value) {
    $row = array();
    $row['data']['name'] = check_plain($value['label']);
    $operations = array();
    $operations['export'] = array(
        'title' => t('export'),
        'href' => FORSCHUNGSATLAS_CONFIG_PATH. '/exports/'. $key .'/download',
        'query' => $destination,
    );
    $row['data']['operations'] = array(
        'data' => array(
            '#theme' => 'links',
            '#links' => $operations,
            '#attributes' => array('class' => array('links', 'inline', 'nowrap')),
        ),
    );
    $rows[] = $row;
  } // foreach()
  $build['forschungsatlas_exports_table'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#empty' => t('There are no available exports.'),
  );

  return $build;
}

/**
 * Create the export file
 */
function forschungsatlas_exports_download($action='') {
  // Select action
  $actions = forschungsatlas_exports_actions();
  $request = array(
    'label' => $actions[$action]['label'],
    'table' => $actions[$action]['table'],
    'mapping_column' => $actions[$action]['column'],
    'mapping_table' => $actions[$action]['mapping_table'],
  );
  // File name from label
  $filename = _forschungsatlas_exports_sanitize_filename($request['label']);
  $filename .= '.csv';
  // Send header
  drupal_add_http_header('Content-Type', 'text/csv');
  drupal_add_http_header('Content-Disposition', 'attachment;filename='. $filename);
  drupal_add_http_header('Cache-Control', 'no-store, no-cache, must-revalidate, post-check=0, pre-check=0', FALSE);
  // Export array
  $export_data = array();
  $categories = _forschungsatlas_exports_get_categories($request);
  $export_data = _forschungsatlas_exports_build_institution_category_export($request, $categories);
  // Output into a csv file
  $fp = fopen('php://output', 'w');
  foreach($export_data as $line){
    fputcsv($fp, $line);
  }
  fclose($fp);
  drupal_exit();
}

/**
 * Build export array
 */
function _forschungsatlas_exports_build_institution_category_export($request=array(), $categories=array()) {
  $export_rows = array();
  // Header
  $row_header = _forschungsatlas_exports_build_institution_category_header($request, $categories);
  $export_rows[] = $row_header;
  // Get list of parent institutions
  $result = db_query('SELECT iid, name, abbrev, familytreeids FROM {forschungsatlas__exports_institutions_view}');
  foreach ($result as $record) {
    $institution = array(
      'iid' => $record->iid,
      'name' => $record->name,
      'abbrev' => $record->abbrev,
      'familytreeids' => $record->familytreeids,
    );
    // Build row
    $export_row = _forschungsatlas_exports_build_institution_category_row($institution, $request, $categories);
    $export_rows[] = $export_row;
  }

  return $export_rows;
}

/**
 * Build the header of the csv file
 */
function _forschungsatlas_exports_build_institution_category_header($request=array(), $categories=array()) {
  $row = array();
  //$row[] = 'ID';
  $row[] = 'Name';
  $row[] = 'Kürzel';
  foreach ($categories as $value) {
    $row[] = $value['categname'];
  }

  return $row;
}

/**
 * Build a line of the export csv file
 */
function _forschungsatlas_exports_build_institution_category_row($institution=array(), $request=array(), $categories=array()) {
  $row = array();
  //$row[] = $institution['iid'];
  $row[] = $institution['name'];
  $row[] = $institution['abbrev'];
  foreach ($categories as $value) {
    $category = array(
      'categid' => $value['categid'],
      'mapping_column' => $request['mapping_column'],
      'mapping_table' => $request['mapping_table'],
    );
    $usage = _forschungsatlas_exports_find_category_usage($institution, $category);
    $row[] = $usage;
  }

  return $row;
}

/**
 * Check if the parent institution is mapped to the category
 */
function _forschungsatlas_exports_find_category_usage($institution=array(), $category=array()) {
  $query = 'SELECT iid, '. $category['mapping_column'] .' FROM {'. $category['mapping_table'] .'}
      WHERE (iid IN ('. $institution['familytreeids'] .')) AND
           ('. $category['mapping_column']  .' = '. $category['categid'] .')';

  $rows_num = db_query($query)->rowCount();
  if ($rows_num > 0) {
    return 1;
  }

  return 0;
}

/**
 * Get list of categories
 */
function _forschungsatlas_exports_get_categories($request=array()) {
  $categories = array();
  $query = 'SELECT '. $request['mapping_column'] .' categid, name categname FROM {'. $request['table'] .'} ORDER BY name ASC';
  $result = db_query($query);
  foreach ($result as $record) {
    $category = array();
    $category['categid'] = $record->categid;
    $category['categname'] = $record->categname;
    $categories[] = $category;
  }

  return $categories;
}

/**
 * Sanitize string
 */
function _forschungsatlas_exports_sanitize_filename($string) {
  return strtolower(trim(preg_replace('~[^0-9a-z]+~i', '-', html_entity_decode(preg_replace('~&([a-z]{1,2})(?:acute|cedil|circ|grave|lig|orn|ring|slash|th|tilde|uml);~i', '$1', htmlentities($string, ENT_QUOTES, 'UTF-8')), ENT_QUOTES, 'UTF-8')), '-'));
}

