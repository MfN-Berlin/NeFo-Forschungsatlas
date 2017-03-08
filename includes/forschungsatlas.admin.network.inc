<?php
/**
 * @file
 * nefo.admin.network.inc
 */

/**
 * Table sort builder with pagination
 */
function forschungsatlas_admin_networks() {
  $header = array();
  $header[] = array('data' => t('Name'), 'field' => 'name', 'sort' => 'asc');
  $header[] = array('data' => t('Parent'), 'field' => 'parent');
  $header[] = array('data' => t('Operation'));
  $query = db_select('forschungsatlas__research_network', 'ot')
    ->extend('TableSort')
    ->fields('ot', array('rnid', 'name', 'parent'));
  $table_sort = $query
    ->orderByHeader($header);
  $pager = $table_sort->extend('PagerDefault')
    ->limit(FORSCHUNGSATLAS_PAGER);
  $result = $pager->execute();
  $rows = array();
  foreach ($result as $data) {
    $row = array();
    $row['data']['name'] = check_plain($data->name);
    $row['data']['parent'] = check_plain(forschungsatlas_get_network_name($data->parent));
    $operations = array();
    $operations['edit'] = array(
      'title' => t('edit'),
      'href' => FORSCHUNGSATLAS_CONFIG_PATH. '/categories/networks/'. $data->rnid .'/edit',
    );
    $operations['delete'] = array(
      'title' => t('delete'),
      'href' => FORSCHUNGSATLAS_CONFIG_PATH. '/categories/networks/' . $data->rnid . '/delete',
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
    '#attributes' => array('id' => 'forschungsatlas-table-networks'),
  );
  $build['forschungsatlas_pager'] = array(
    '#markup' => theme('pager'),
  );

  return $build;;
}

/**
 * Form constructor for the research network form.
 *
 * @see forschungsatlas_admin_network_form_validate()
 * @see forschungsatlas_admin_network_form_submit()
 */
function forschungsatlas_admin_network_form($form, &$form_state, $network=array(
    'rnid' => '', 'name' => '', 'parent' => '')){

  $rnid = intval($network['rnid']);
  $parent = intval($network['parent']);
  $parents = forschungsatlas_admin_get_parent_networks($rnid);
  // Option for no parent
  $parents[0] = t('- None -');
  $form['network'] = array(
    '#type' => 'fieldset',
    '#title' => t('Research Network'),
  );
  $form['network']['name'] = array(
      '#type' => 'textfield',
      '#title' => t('Name'),
      '#size' => 20,
      '#default_value' => $network['name'],
      '#required' => TRUE,
  );
  $form['network']['parent'] = array(
    '#type' => 'select',
    '#title' => t('Parent research network'),
    '#options' => $parents,
    '#default_value' => $parent,
    '#element_validate' => array('element_validate_integer'),
  );
  $form['rnid'] = array(
    '#type' => 'hidden',
    '#value' => $rnid,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Page callback: Returns a form to edit an research network.
 *
 * @param $network
 *   An array containing the research network.
 *
 * @return
 *   A form to edit the research network.
 *
 * @see forschungsatlas_menu()
 */
function forschungsatlas_admin_network_edit($network = array()){
  drupal_set_title($network['name']);

  return drupal_get_form('forschungsatlas_admin_network_form', $network);
}

/**
 * Submit function for forschungsatlas_admin_network_form().
 *
 * @see forschungsatlas_admin_network_form()
 * @see forschungsatlas_admin_network_form_validate()
 */
function forschungsatlas_admin_network_form_submit($form, &$form_state) {
  global $user;

  $table = 'forschungsatlas__research_network';
  $record = new stdClass();
  $record->name = forschungsatlas_clean_name($form_state['values']['name']);
  $record->parent = $form_state['values']['parent'];
  $record->uid = $user->uid;
  $record->changed = REQUEST_TIME;
  $rnid = intval($form_state['values']['rnid']);
  if (!$rnid) { // Insert
    $record->created = REQUEST_TIME;
    drupal_write_record($table, $record);
    $output = t('The research network <em>@name</em> has been registered.', array('@name' => $record->name));
  }
  else { // Update
    $record->rnid = $rnid;
    $result = drupal_write_record($table, $record, 'rnid');
    $output = t('The settings for the research network <em>@name</em> have been updated.', array('@name' => $record->name));
  }
  watchdog('Nefo', $output);
  drupal_set_message($output);
  $form_state['redirect'] = FORSCHUNGSATLAS_CONFIG_PATH. '/categories/networks';
}

/**
 * Validate function for forschungsatlas_admin_network_form().
 *
 * @see forschungsatlas_admin_network_form()
 * @see forschungsatlas_admin_network_form_submit()
 */
function forschungsatlas_admin_network_form_validate($form, &$form_state) {
  // Parent research network should actually exist
  $parent = intval($form_state['values']['parent']);
  if ($parent && !forschungsatlas_get_network_name($parent)) {
    watchdog('security', 'Possible SQL injection attempt! An illegal choice has been detected: PARENT.', array(), WATCHDOG_ALERT);
    form_set_error('parent', t('The server was unable to process the request due to an internal error.'));
  }
  // Duplicate
  $rnid = intval($form_state['values']['rnid']);
  if (_forschungsatlas_findDuplicateNetwork($form_state['values']['name'], $rnid) > 0) {
    form_set_error('name', t('An research network with this name already exists.'));
  }
}


/**
 * Form constructor for the research network deletion confirmation form.
 *
 * @see forschungsatlas_admin_network_delete_confirm_submit()
 */
function forschungsatlas_admin_network_delete_confirm($form, &$form_state, $network) {
  $form_state['NEFO']['rnid'] = $network['rnid'];
  $form_state['NEFO']['name'] = $network['name'];
  $form_state['NEFO']['parent'] = $network['parent'];
  $form['rnid'] = array('#type' => 'value', '#value' => $network['rnid']);
  if ($network['parent']) {
    $output = t('Are you sure you want to delete this research network: <em>@name</em>?', array('@name' => $network['name']));
  }
  else {
    $output = t('Delete this parent research network with all its children: <em>@name</em>?', array('@name' => $network['name']));
  }
  return confirm_form($form,
      $output,
      FORSCHUNGSATLAS_CONFIG_PATH. '/categories/networks',
      t('This action cannot be undone.'),
      t('Delete'),
      t('Cancel')
  );
}

/**
 * Execute research network deletion.
 *
 * @see forschungsatlas_admin_network_delete_confirm()
 */
function forschungsatlas_admin_network_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $rnid = intval($form_state['NEFO']['rnid']);
    $parent = intval($form_state['NEFO']['parent']);
    _forschungsatlas_delete_network($rnid, $parent);
    $output = t('The research network <em>@name</em> has been deleted.', array('@name' => $form_state['NEFO']['name']));
    watchdog('NEFO', $output);
    drupal_set_message($output);
  }

  $form_state['redirect'] = FORSCHUNGSATLAS_CONFIG_PATH. '/categories/networks';
}

/**
 * Delete a research network
 * @param integer $rnid
 *  The research network ID
 * @param integer $parent
 *  The parent ID
 */
function _forschungsatlas_delete_network($rnid, $parent) {
  $or = db_or()->condition('rnid', $rnid);
  if (!$parent) {
    $or->condition('parent', $rnid);
  }
  $result = db_select('forschungsatlas__research_network', 't')
    ->fields('t', array('rnid'))
    ->condition($or)
    ->execute();
  $ids = array();
  foreach ($result as $data) {
    $ids[] = $data->rnid;
  }
  db_delete('forschungsatlas__institution_researchnetwork')
    ->condition('rnid', $ids, 'IN')
    ->execute();
  db_delete('forschungsatlas__research_network')
    ->condition('rnid', $ids, 'IN')
    ->execute();
}


/**
 * Validations
 */
function forschungsatlas_admin_get_parent_networks($rnid){
  $result = db_query('SELECT rnid, name FROM {forschungsatlas__research_network}
      WHERE rnid!=:rnid AND parent=0 ORDER BY name', array(':rnid' => $rnid));
  $rows = array();
  foreach ($result as $row) {
    $rows[$row->rnid] = $row->name;
  }

  return $rows;
}

function _forschungsatlas_findDuplicateNetwork($name, $rnid) {
  return forschungsatlas_find_duplicate_name('forschungsatlas__research_network', 'rnid', $name, $rnid);
}





