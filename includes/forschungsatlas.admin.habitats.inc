<?php
/**
 * @file
 * nefo.admin.habitats.inc
 */

/**
 * Table sort builder with pagination
 */
function forschungsatlas_admin_habitats() {
  $header = array();
  $header[] = array('data' => t('Name'), 'field' => 'name', 'sort' => 'asc');
  $header[] = array('data' => t('Parent'), 'field' => 'parent');
  $header[] = array('data' => t('Operation'));
  $query = db_select('forschungsatlas__habitats', 'h')
    ->extend('TableSort')
    ->fields('h', array('hid', 'name', 'parent'));
  $table_sort = $query
    ->orderByHeader($header);
  $pager = $table_sort->extend('PagerDefault')
    ->limit(FORSCHUNGSATLAS_PAGER);
  $result = $pager->execute();
  $rows = array();
  foreach ($result as $data) {
    $row = array();
    $row['data']['name'] = check_plain($data->name);
    $row['data']['parent'] = check_plain(forschungsatlas_get_habitat_name($data->parent));
    $operations = array();
    $operations['edit'] = array(
      'title' => t('edit'),
      'href' => FORSCHUNGSATLAS_CONFIG_PATH. '/categories/habitats/'. $data->hid .'/edit',
    );
    $operations['delete'] = array(
      'title' => t('delete'),
      'href' => FORSCHUNGSATLAS_CONFIG_PATH. '/categories/habitats/' . $data->hid . '/delete',
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
    '#attributes' => array('id' => 'forschungsatlas-table-habitats'),
  );
  $build['forschungsatlas_pager'] = array(
    '#markup' => theme('pager'),
  );

  return $build;;
}

/**
 * Page callback: Returns a form to edit a habitat.
 *
 * @param $habitat
 *   An array containing the habitat.
 *
 * @return
 *   A form to edit the habitat.
 *
 * @see forschungsatlas_menu()
 */
function forschungsatlas_admin_habitat_edit($habitat = array()) {
  drupal_set_title($habitat['name']);

  return drupal_get_form('forschungsatlas_admin_habitat_form', $habitat);
}

/**
 * Form constructor for the habitat form.
 *
 * @see forschungsatlas_admin_habitat_form_validate()
 * @see forschungsatlas_admin_habitat_form_submit()
 */
function forschungsatlas_admin_habitat_form($form, &$form_state, $habitat=array(
    'hid' => '', 'name' => '', 'parent' => '')) {

  $hid = intval($habitat['hid']);
  $parent = intval($habitat['parent']);
  $parents = forschungsatlas_admin_get_parent_habitats($hid);
  // Option for no parent
  $parents[0] = t('- None -');
  $form['habitat'] = array(
    '#type' => 'fieldset',
    '#title' => t('Habitat'),
  );
  $form['habitat']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#size' => 20,
    '#default_value' => $habitat['name'],
    '#required' => TRUE,
  );
  $form['habitat']['parent'] = array(
    '#type' => 'select',
    '#title' => t('Parent habitat'),
    '#options' => $parents,
    '#default_value' => $parent,
    '#element_validate' => array('element_validate_integer'),
  );
  $form['hid'] = array(
    '#type' => 'hidden',
    '#value' => $hid,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Validate function for forschungsatlas_admin_habitat_form().
 *
 * @see forschungsatlas_admin_habitat_form()
 * @see forschungsatlas_admin_habitat_form_submit()
 */
function forschungsatlas_admin_habitat_form_validate($form, &$form_state) {
  // Parent habitats should actually exist
  $parent = intval($form_state['values']['parent']);
  if ($parent && !forschungsatlas_get_habitat_name($parent)) {
    watchdog('security', 'Possible SQL injection attempt! An illegal choice has been detected: PARENT.', array(), WATCHDOG_ALERT);
    form_set_error('parent', t('The server was unable to process the request due to an internal error.'));
  }
  // Duplicate
  $hid = intval($form_state['values']['hid']);
  if (_forschungsatlas_findDuplicateHabitat($form_state['values']['name'], $hid) > 0) {
    form_set_error('name', t('A habitat with this name already exists.'));
  }
}

/**
 * Submit function for forschungsatlas_admin_habitat_form().
 *
 * @see forschungsatlas_admin_habitat_form()
 * @see forschungsatlas_admin_habitat_form_validate()
 */
function forschungsatlas_admin_habitat_form_submit($form, &$form_state) {
  global $user;

  $table = 'forschungsatlas__habitats';
  $record = new stdClass();
  $record->name = forschungsatlas_clean_name($form_state['values']['name']);
  $record->parent = $form_state['values']['parent'];
  $record->uid = $user->uid;
  $record->changed = REQUEST_TIME;
  $hid = intval($form_state['values']['hid']);
  if (!$hid) { // Insert
    $record->created = REQUEST_TIME;
    drupal_write_record($table, $record);
    $output = t('The habitat <em>@name</em> has been registered.', array('@name' => $record->name));
  }
  else { // Update
    $record->hid = $hid;
    $result = drupal_write_record($table, $record, 'hid');
    $output = t('The settings for the habitat <em>@name</em> have been updated.', array('@name' => $record->name));
  }
  watchdog('Nefo', $output);
  drupal_set_message($output);
  $form_state['redirect'] = FORSCHUNGSATLAS_CONFIG_PATH. '/categories/habitats';
}


/**
 * Form constructor for the habitat deletion confirmation form.
 *
 * @see forschungsatlas_admin_habitat_delete_confirm_submit()
 */
function forschungsatlas_admin_habitat_delete_confirm($form, &$form_state, $habitat) {
  $form_state['NEFO']['hid'] = $habitat['hid'];
  $form_state['NEFO']['name'] = $habitat['name'];
  $form_state['NEFO']['parent'] = $habitat['parent'];
  $form['hid'] = array('#type' => 'value', '#value' => $habitat['hid']);
  if ($habitat['parent']) {
    $output = t('Are you sure you want to delete this habitat: <em>@name</em>?', array('@name' => $habitat['name']));
  }
  else {
    $output = t('Delete this parent habitat with all its children: <em>@name</em>?', array('@name' => $habitat['name']));
  }
  return confirm_form($form,
      $output,
      FORSCHUNGSATLAS_CONFIG_PATH. '/categories/habitats',
      t('This action cannot be undone.'),
      t('Delete'),
      t('Cancel')
  );
}

/**
 * Execute habitat deletion.
 *
 * @see forschungsatlas_admin_habitat_delete_confirm()
 */
function forschungsatlas_admin_habitat_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $hid = intval($form_state['NEFO']['hid']);
    $parent = intval($form_state['NEFO']['parent']);
    _forschungsatlas_delete_habitat($hid, $parent);
    $output = t('The habitat <em>@name</em> has been deleted.', array('@name' => $form_state['NEFO']['name']));
    watchdog('NEFO', $output);
    drupal_set_message($output);
  }

  $form_state['redirect'] = FORSCHUNGSATLAS_CONFIG_PATH. '/categories/habitats';
}

/**
 * Delete a habitat
 * @param integer $hid
 *  The habitat ID
 * @param integer $parent
 *  The parent ID
 */
function _forschungsatlas_delete_habitat($hid, $parent) {
  $or = db_or()->condition('hid', $hid);
  if (!$parent) {
    $or->condition('parent', $hid);
  }
  $result = db_select('forschungsatlas__habitats', 't')
    ->fields('t', array('hid'))
    ->condition($or)
    ->execute();
  $ids = array();
  foreach ($result as $data) {
    $ids[] = $data->hid;
  }
  db_delete('forschungsatlas__institution_habitat')
    ->condition('hid', $ids, 'IN')
    ->execute();
  db_delete('forschungsatlas__habitats')
    ->condition('hid', $ids, 'IN')
    ->execute();
}


/**
 * Validations
 */
function forschungsatlas_admin_get_parent_habitats($hid){
  $result = db_query('SELECT hid, name FROM {forschungsatlas__habitats}
      WHERE hid!=:hid AND parent=0 ORDER BY name', array(':hid' => $hid));
  $rows = array();
  foreach ($result as $row) {
    $rows[$row->hid] = $row->name;
  }

  return $rows;
}

function _forschungsatlas_findDuplicateHabitat($name, $hid) {

  return forschungsatlas_find_duplicate_name('forschungsatlas__habitats', 'hid', $name, $hid);
}



