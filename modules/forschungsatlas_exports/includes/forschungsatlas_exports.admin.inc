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
 * @param unknown $action
 * @return multitype:string The multitype:multitype:The   multitype:unknown
 */
function forschungsatlas_exports_download($action='') {
  drupal_set_title(t('Download export test'));

  // Select action
  $actions = forschungsatlas_exports_actions();
  $request = array(
    'label' => $actions[$action]['label'],
    'table' => $actions[$action]['table'],
    'mapping_column' => $actions[$action]['column'],
    'mapping_table' => $actions[$action]['mapping_table'],
  );

  // Build export data
  $export_data = array();
  $export_data = _forschungsatlas_exports_build_institution_category_export($request);

//
//  KRUMO !!!
//
  krumo($export_data);

  $header = array();
  $header[] = array('data' => t('Action'));

  $rows = array();
  $row = array();
  $row['data']['name'] = $action;
  $rows[] = $row;

  $build['forschungsatlas_exports_table'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#empty' => t('There are no available exports.'),
  );

  return $build;

}


function _forschungsatlas_exports_build_institution_category_export($request=array()) {
  $export_rows = array();
  // Header
  $row_header = _forschungsatlas_exports_build_institution_category_header($request);
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
    $export_row = _forschungsatlas_exports_build_institution_category_row($institution, $request);
    $export_rows[] = $export_row;
  }

  return $export_rows;
}

function _forschungsatlas_exports_build_institution_category_header($request=array()) {
  $categories = _forschungsatlas_exports_get_categories($request);
  $row = array();
  $row[] = 'ID';
  $row[] = 'Name';
  $row[] = 'Kürzel';
  foreach ($categories as $value) {
    $row[] = $value['categname'];
  }

  return $row;
}

function _forschungsatlas_exports_build_institution_category_row($institution=array(), $request=array()) {
  $categories = _forschungsatlas_exports_get_categories($request);
  $row = array();
  $row[] = $institution['iid'];
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


function _forschungsatlas_exports_get_categories($data=array()) {
  $categories = array();
  $query = 'SELECT '. $data['mapping_column'] .' categid, name categname FROM {'. $data['table'] .'} ORDER BY name ASC';
  $result = db_query($query);
  foreach ($result as $record) {
    $category = array();
    $category['categid'] = $record->categid;
    $category['categname'] = $record->categname;
    $categories[] = $category;
  }

  return $categories;
}

