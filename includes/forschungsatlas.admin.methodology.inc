<?php
/**
 * @file
 * nefo.admin.methodology.inc
 */

/**
 * Table sort builder with pagination
 */
function forschungsatlas_admin_methodologies() {
  $header = array();
  $header[] = array('data' => t('Name'), 'field' => 'name', 'sort' => 'asc');
  $header[] = array('data' => t('Parent'), 'field' => 'parent');
  $header[] = array('data' => t('Operation'));
  $query = db_select('forschungsatlas__methodology', 'm')
    ->extend('TableSort')
    ->fields('m', array('mid', 'name', 'parent'));
  $table_sort = $query
    ->orderByHeader($header);
  $pager = $table_sort->extend('PagerDefault')
    ->limit(FORSCHUNGSATLAS_PAGER);
  $result = $pager->execute();
  $rows = array();
  foreach ($result as $data) {
    $row = array();
    $row['data']['name'] = check_plain($data->name);
    $row['data']['parent'] = check_plain(forschungsatlas_get_methodology_name($data->parent));
    $operations = array();
    $operations['edit'] = array(
      'title' => t('edit'),
      'href' => FORSCHUNGSATLAS_CONFIG_PATH. '/categories/methodologies/'. $data->mid .'/edit',
    );
    $operations['delete'] = array(
      'title' => t('delete'),
      'href' => FORSCHUNGSATLAS_CONFIG_PATH. '/categories/methodologies/' . $data->mid . '/delete',
      'query' => drupal_get_destination(),
    );
    $row['data']['operations'] = array(
    'data' => array(
      '#theme' => 'links',
      '#links' => $operations,
      '#attributes' => array('class' => array('links', 'inline', 'nowrap')),
      ),
    );
    $rows[] = $row;
  }
  $build['forschungsatlas_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No items available.'),
    '#attributes' => array('id' => 'forschungsatlas-table-methodologies'),
  );
  $build['forschungsatlas_pager'] = array(
    '#markup' => theme('pager'),
  );

  return $build;;
}

/**
 * Form constructor for the methodology form.
 *
 * @see forschungsatlas_admin_methodology_form_validate()
 * @see forschungsatlas_admin_methodology_form_submit()
 */
function forschungsatlas_admin_methodology_form($form, &$form_state, $methodology=array(
    'mid' => '', 'name' => '', 'parent' => '')) {

  $mid = intval($methodology['mid']);
  $parent = intval($methodology['parent']);
  $parents = forschungsatlas_admin_get_parent_methodologies($mid);
  // Option for no parent
  $parents[0] = t('- None -');
  $form['methodology'] = array(
    '#type' => 'fieldset',
    '#title' => t('Methodology'),
  );
  $form['methodology']['name'] = array(
      '#type' => 'textfield',
      '#title' => t('Name'),
      '#size' => 20,
      '#default_value' => $methodology['name'],
      '#required' => TRUE,
  );
  $form['methodology']['parent'] = array(
    '#type' => 'select',
    '#title' => t('Parent Methodology'),
    '#options' => $parents,
    '#default_value' => $parent,
    '#element_validate' => array('element_validate_integer'),
  );
  $form['mid'] = array(
    '#type' => 'hidden',
    '#value' => $mid,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Page callback: Returns a form to edit a methodology.
 *
 * @param $methodology
 *   An array containing the methodology.
 *
 * @return
 *   A form to edit the methodology.
 *
 * @see forschungsatlas_menu()
 */
function forschungsatlas_admin_methodology_edit($methodology = array()){
  drupal_set_title($methodology['name']);

  return drupal_get_form('forschungsatlas_admin_methodology_form', $methodology);
}

/**
 * Submit function for forschungsatlas_admin_methodology_form().
 *
 * @see forschungsatlas_admin_methodology_form()
 * @see forschungsatlas_admin_methodology_form_validate()
 */
function forschungsatlas_admin_methodology_form_submit($form, &$form_state) {
  global $user;

  $table = 'forschungsatlas__methodology';
  $record = new stdClass();
  $record->name = forschungsatlas_clean_name($form_state['values']['name']);
  $record->parent = $form_state['values']['parent'];
  $record->uid = $user->uid;
  $record->changed = REQUEST_TIME;
  $mid = intval($form_state['values']['mid']);
  if (!$mid) { // Insert
    $record->created = REQUEST_TIME;
    drupal_write_record($table, $record);
    $output = t('The methodology <em>@name</em> has been registered.', array('@name' => $record->name));
  }
  else { // Update
    $record->mid = $mid;
    $result = drupal_write_record($table, $record, 'mid');
    $output = t('The settings for the methodology <em>@name</em> have been updated.', array('@name' => $record->name));
  }
  watchdog('Nefo', $output);
  drupal_set_message($output);
  $form_state['redirect'] = FORSCHUNGSATLAS_CONFIG_PATH. '/categories/methodologies';
}

/**
 * Validate function for forschungsatlas_admin_methodology_form().
 *
 * @see forschungsatlas_admin_methodology_form()
 * @see forschungsatlas_admin_methodology_form_submit()
 */
function forschungsatlas_admin_methodology_form_validate($form, &$form_state) {
  // Parent methodologies should actually exist
  $parent = intval($form_state['values']['parent']);
  if ($parent && !forschungsatlas_get_methodology_name($parent)) {
    watchdog('security', 'Possible SQL injection attempt! An illegal choice has been detected: PARENT.', array(), WATCHDOG_ALERT);
    form_set_error('parent', t('The server was unable to process the request due to an internal error.'));
  }
  // Duplicate
  $mid = intval($form_state['values']['mid']);
  if (_forschungsatlas_findDuplicateMethodology($form_state['values']['name'], $mid) > 0) {
    form_set_error('name', t('A methodology with this name already exists.'));
  }
}


/**
 * Form constructor for the methodology deletion confirmation form.
 *
 * @see forschungsatlas_admin_methodology_delete_confirm_submit()
 */
function forschungsatlas_admin_methodology_delete_confirm($form, &$form_state, $methodology) {
  $form_state['NEFO']['mid'] = $methodology['mid'];
  $form_state['NEFO']['name'] = $methodology['name'];
  $form_state['NEFO']['parent'] = $methodology['parent'];
  $form['mid'] = array('#type' => 'value', '#value' => $methodology['mid']);
  if ($methodology['parent']) {
    $output = t('Are you sure you want to delete this methodology: <em>@name</em>?', array('@name' => $methodology['name']));
  }
  else {
    $output = t('Delete this parent methodology with all its children: <em>@name</em>?', array('@name' => $methodology['name']));
  }
  return confirm_form($form,
      $output,
      FORSCHUNGSATLAS_CONFIG_PATH. '/categories/methodologies',
      t('This action cannot be undone.'),
      t('Delete'),
      t('Cancel')
  );
}

/**
 * Execute methodology deletion.
 *
 * @see forschungsatlas_admin_methodology_delete_confirm()
 */
function forschungsatlas_admin_methodology_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $mid = intval($form_state['NEFO']['mid']);
    $parent = intval($form_state['NEFO']['parent']);
    _forschungsatlas_delete_methodology($mid, $parent);
    $output = t('The methodology <em>@name</em> has been deleted.', array('@name' => $form_state['NEFO']['name']));
    watchdog('NEFO', $output);
    drupal_set_message($output);
  }

  $form_state['redirect'] = FORSCHUNGSATLAS_CONFIG_PATH. '/categories/methodologies';
}

/**
 * Delete a methodology
 * @param integer $mid
 *  The methodology ID
 * @param integer $parent
 *  The parent ID
 */
function _forschungsatlas_delete_methodology($mid, $parent) {
  $or = db_or()->condition('mid', $mid);
  if (!$parent) {
    $or->condition('parent', $mid);
  }
  $result = db_select('forschungsatlas__methodology', 't')
    ->fields('t', array('mid'))
    ->condition($or)
    ->execute();
  $ids = array();
  foreach ($result as $data) {
    $ids[] = $data->mid;
  }
  db_delete('forschungsatlas__institution_methodology')
    ->condition('mid', $ids, 'IN')
    ->execute();
  db_delete('forschungsatlas__methodology')
    ->condition('mid', $ids, 'IN')
    ->execute();
}



/**
 * Validations
 */
function forschungsatlas_admin_get_parent_methodologies($mid){
  $result = db_query('SELECT mid, name FROM {forschungsatlas__methodology}
      WHERE mid!=:mid AND parent=0 ORDER BY name', array(':mid' => $mid));
  $rows = array();
  foreach ($result as $row) {
    $rows[$row->mid] = $row->name;
  }

  return $rows;
}

function _forschungsatlas_findDuplicateMethodology($name, $mid) {

  return forschungsatlas_find_duplicate_name('forschungsatlas__methodology', 'mid', $name, $mid);
}





