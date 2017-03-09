<?php
/**
 * @file
 * forschungsatlas.admin.organism.inc
 */

/**
 * Table sort builder with pagination
 */
function forschungsatlas_admin_organisms() {
  $header = array();
  $header[] = array('data' => t('Name'), 'field' => 'name', 'sort' => 'asc');
  $header[] = array('data' => t('Parent'), 'field' => 'parent');
  $header[] = array('data' => t('Operation'));
  $query = db_select('forschungsatlas__organisms', 'm')
    ->extend('TableSort')
    ->fields('m', array('oid', 'name', 'parent'));
  $table_sort = $query
    ->orderByHeader($header);
  $pager = $table_sort->extend('PagerDefault')
    ->limit(FORSCHUNGSATLAS_PAGER);
  $result = $pager->execute();
  $rows = array();
  foreach ($result as $data) {
    $row = array();
    $row['data']['name'] = check_plain($data->name);
    $row['data']['parent'] = check_plain(forschungsatlas_get_organism_name($data->parent));
    $operations = array();
    $operations['edit'] = array(
      'title' => t('edit'),
      'href' => FORSCHUNGSATLAS_CONFIG_PATH. '/categories/organisms/'. $data->oid .'/edit',
    );
    $operations['delete'] = array(
      'title' => t('delete'),
      'href' => FORSCHUNGSATLAS_CONFIG_PATH. '/categories/organisms/' . $data->oid . '/delete',
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
    '#attributes' => array('id' => 'forschungsatlas-table-organisms'),
  );
  $build['forschungsatlas_pager'] = array(
    '#markup' => theme('pager'),
  );

  return $build;;
}

/**
 * Form constructor for the organism form.
 *
 * @see forschungsatlas_admin_organism_form_validate()
 * @see forschungsatlas_admin_organism_form_submit()
 */
function forschungsatlas_admin_organism_form($form, &$form_state, $organism=array(
    'oid' => '', 'name' => '', 'parent' => '')) {

  $oid = intval($organism['oid']);
  $parent = intval($organism['parent']);
  $parents = forschungsatlas_admin_get_parent_organisms($oid);
  // Option for no parent
  $parents[0] = t('- None -');
  $form['organism'] = array(
    '#type' => 'fieldset',
    '#title' => t('Organism'),
  );
  $form['organism']['name'] = array(
      '#type' => 'textfield',
      '#title' => t('Name'),
      '#size' => 20,
      '#default_value' => $organism['name'],
      '#required' => TRUE,
  );
  $form['organism']['parent'] = array(
    '#type' => 'select',
    '#title' => t('Parent Organism'),
    '#options' => $parents,
    '#default_value' => $parent,
    '#element_validate' => array('element_validate_integer'),
  );
  $form['oid'] = array(
    '#type' => 'hidden',
    '#value' => $oid,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Page callback: Returns a form to edit an organism.
 *
 * @param $organism
 *   An array containing the organism.
 *
 * @return
 *   A form to edit the organism.
 *
 * @see forschungsatlas_menu()
 */
function forschungsatlas_admin_organism_edit($organism = array()){
  drupal_set_title($organism['name']);

  return drupal_get_form('forschungsatlas_admin_organism_form', $organism);
}

/**
 * Submit function for forschungsatlas_admin_organism_form().
 *
 * @see forschungsatlas_admin_organism_form()
 * @see forschungsatlas_admin_organism_form_validate()
 */
function forschungsatlas_admin_organism_form_submit($form, &$form_state) {
  global $user;

  $table = 'forschungsatlas__organisms';
  $record = new stdClass();
  $record->name = forschungsatlas_clean_name($form_state['values']['name']);
  $record->parent = $form_state['values']['parent'];
  $oid = intval($form_state['values']['oid']);
  if (!$oid) { // Insert
    $record->oid = '';
    drupal_write_record($table, $record);
    $output = t('The organism <em>@name</em> has been registered.', array('@name' => $record->name));
  }
  else { // Update
    $record->oid = $oid;
    $result = drupal_write_record($table, $record, 'oid');
    $output = t('The settings for the organism <em>@name</em> have been updated.', array('@name' => $record->name));
  }
  if ($record->oid) {
    $form_state['values']['oid'] = $record->oid;
    $form_state['oid'] = $record->oid;
  }
  watchdog('NEFO', $output);
  drupal_set_message($output);
  $form_state['redirect'] = FORSCHUNGSATLAS_CONFIG_PATH. '/categories/organisms';
}

/**
 * Validate function for forschungsatlas_admin_organism_form().
 *
 * @see forschungsatlas_admin_organism_form()
 * @see forschungsatlas_admin_organism_form_submit()
 */
function forschungsatlas_admin_organism_form_validate($form, &$form_state) {
  // Parent organism should actually exist
  $parent = intval($form_state['values']['parent']);
  if ($parent && !forschungsatlas_get_organism_name($parent)) {
    watchdog('security', 'Possible SQL injection attempt! An illegal choice has been detected: PARENT.', array(), WATCHDOG_ALERT);
    form_set_error('parent', t('The server was unable to process the request due to an internal error.'));
  }
  // Duplicate
  $oid = intval($form_state['values']['oid']);
  if (_forschungsatlas_findDuplicateOrganism($form_state['values']['name'], $oid) > 0) {
    form_set_error('name', t('An organism with this name already exists.'));
  }
}


/**
 * Form constructor for the organism deletion confirmation form.
 *
 * @see forschungsatlas_admin_organism_delete_confirm_submit()
 */
function forschungsatlas_admin_organism_delete_confirm($form, &$form_state, $organism) {
  $form_state['NEFO']['oid'] = $organism['oid'];
  $form_state['NEFO']['name'] = $organism['name'];
  $form_state['NEFO']['parent'] = $organism['parent'];
  $form['oid'] = array('#type' => 'value', '#value' => $organism['oid']);
  if ($organism['parent']) {
    $output = t('Are you sure you want to delete this organism: <em>@name</em>?', array('@name' => $organism['name']));
  }
  else {
    $output = t('Delete this parent organism with all its children: <em>@name</em>?', array('@name' => $organism['name']));
  }
  return confirm_form($form,
      $output,
      FORSCHUNGSATLAS_CONFIG_PATH. '/categories/organisms',
      t('This action cannot be undone.'),
      t('Delete'),
      t('Cancel')
  );
}

/**
 * Execute organism deletion.
 *
 * @see forschungsatlas_admin_organism_delete_confirm()
 */
function forschungsatlas_admin_organism_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $oid = intval($form_state['NEFO']['oid']);
    $parent = intval($form_state['NEFO']['parent']);
    _forschungsatlas_delete_organism($oid, $parent);
    $output = t('The organism <em>@name</em> has been deleted.', array('@name' => $form_state['NEFO']['name']));
    watchdog('NEFO', $output);
    drupal_set_message($output);
  }

  $form_state['redirect'] = FORSCHUNGSATLAS_CONFIG_PATH. '/categories/organisms';
}

/**
 * Delete an organism
 * @param integer $oid
 *  The organism ID
 * @param integer $parent
 *  The parent ID
 */
function _forschungsatlas_delete_organism($oid, $parent) {
  $or = db_or()->condition('oid', $oid);
  if (!$parent) {
    $or->condition('parent', $oid);
  }
  $result = db_select('forschungsatlas__organisms', 't')
    ->fields('t', array('oid'))
    ->condition($or)
    ->execute();
  $ids = array();
  foreach ($result as $data) {
    $ids[] = $data->oid;
  }
  db_delete('forschungsatlas__institution_organism')
    ->condition('oid', $ids, 'IN')
    ->execute();
  db_delete('forschungsatlas__organisms')
    ->condition('oid', $ids, 'IN')
    ->execute();
}


/**
 * Validations
 */
function forschungsatlas_admin_get_parent_organisms($oid){
  $result = db_query('SELECT oid, name FROM {forschungsatlas__organisms}
      WHERE oid!=:oid AND parent=0 ORDER BY name', array(':oid' => $oid));
  $rows = array();
  foreach ($result as $row) {
    $rows[$row->oid] = $row->name;
  }

  return $rows;
}

function _forschungsatlas_findDuplicateOrganism($name, $oid) {
  return forschungsatlas_find_duplicate_name('forschungsatlas__organisms', 'oid', $name, $oid);
}





