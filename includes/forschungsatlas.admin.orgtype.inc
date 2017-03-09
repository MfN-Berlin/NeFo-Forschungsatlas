<?php
/**
 * @file
 * forschungsatlas.admin.orgtype.inc
 */

/**
 * Table sort builder with pagination
 */
function forschungsatlas_admin_orgtypes() {
  $header = array();
  $header[] = array('data' => t('Name'), 'field' => 'name', 'sort' => 'asc');
  $header[] = array('data' => t('Operation'));
  $query = db_select('forschungsatlas__organization_type', 'ot')
    ->extend('TableSort')
    ->fields('ot', array('otid', 'name'));
  $table_sort = $query
    ->orderByHeader($header);
  $pager = $table_sort->extend('PagerDefault')
    ->limit(FORSCHUNGSATLAS_PAGER);
  $result = $pager->execute();
  $rows = array();
  foreach ($result as $data) {
    $row = array();
    $row['data']['name'] = check_plain($data->name);
    $operations = array();
    $operations['edit'] = array(
      'title' => t('edit'),
      'href' => FORSCHUNGSATLAS_CONFIG_PATH. '/institutions/organizationtypes/'. $data->otid .'/edit',
    );
    if (_forschungsatlas_find_orgtype_usage(intval($data->otid)) > 0) {
      $operations['delete'] = array(
          'title' => t('delete'),
      );
    } else {
      $operations['delete'] = array(
          'title' => t('delete'),
          'href' => FORSCHUNGSATLAS_CONFIG_PATH. '/institutions/organizationtypes/' . $data->otid . '/delete',
          'query' => drupal_get_destination(),
      );
    }
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
    '#attributes' => array('id' => 'forschungsatlas-table-orgtypes'),
  );
  $build['forschungsatlas_pager'] = array(
    '#markup' => theme('pager'),
  );

  return $build;;
}

/**
 * Form constructor for the organization type form.
 *
 * @see forschungsatlas_admin_orgtype_form_validate()
 * @see forschungsatlas_admin_orgtype_form_submit()
 */
function forschungsatlas_admin_orgtype_form($form, &$form_state, $orgtype=array(
    'otid' => '', 'name' => '')) {

  $otid = intval($orgtype['otid']);
  $form['orgtype'] = array(
    '#type' => 'fieldset',
    '#title' => t('Organization type'),
  );
  $form['orgtype']['name'] = array(
      '#type' => 'textfield',
      '#title' => t('Name'),
      '#size' => 20,
      '#default_value' => $orgtype['name'],
      '#required' => TRUE,
  );
  $form['otid'] = array(
    '#type' => 'hidden',
    '#value' => $otid,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Page callback: Returns a form to edit an organization type.
 *
 * @param $orgtype
 *   An array containing the organization type.
 *
 * @return
 *   A form to edit the organization type.
 *
 * @see forschungsatlas_menu()
 */
function forschungsatlas_admin_orgtype_edit($orgtype = array()){
  drupal_set_title($orgtype['name']);

  return drupal_get_form('forschungsatlas_admin_orgtype_form', $orgtype);
}

/**
 * Submit function for forschungsatlas_admin_orgtype_form().
 *
 * @see forschungsatlas_admin_orgtype_form()
 * @see forschungsatlas_admin_orgtype_form_validate()
 */
function forschungsatlas_admin_orgtype_form_submit($form, &$form_state) {
  global $user;

  $table = 'forschungsatlas__organization_type';
  $record = new stdClass();
  $record->name = forschungsatlas_clean_name($form_state['values']['name']);
  $otid = intval($form_state['values']['otid']);
  if (!$otid) { // Insert
    $record->otid = '';
    drupal_write_record($table, $record);
    $output = t('The organization type <em>@name</em> has been registered.', array('@name' => $record->name));
  }
  else { // Update
    $record->otid = $otid;
    $result = drupal_write_record($table, $record, 'otid');
    $output = t('The settings for the organization type <em>@name</em> have been updated.', array('@name' => $record->name));
  }
  if ($record->otid) {
    $form_state['values']['otid'] = $record->otid;
    $form_state['otid'] = $record->otid;
  }
  watchdog('NEFO', $output);
  drupal_set_message($output);
  $form_state['redirect'] = FORSCHUNGSATLAS_CONFIG_PATH. '/institutions/organizationtypes';
}

/**
 * Validate function for forschungsatlas_admin_orgtype_form().
 *
 * @see forschungsatlas_admin_orgtype_form()
 * @see forschungsatlas_admin_orgtype_form_submit()
 */
function forschungsatlas_admin_orgtype_form_validate($form, &$form_state) {
  // Duplicate
  $otid = intval($form_state['values']['otid']);
  if (_forschungsatlas_findDuplicateOrgtype($form_state['values']['name'], $otid) > 0) {
    form_set_error('name', t('An organization type with this name already exists.'));
  }
}

/**
 * Form constructor for the organization type deletion confirmation form.
 *
 * @see forschungsatlas_admin_orgtype_delete_confirm_submit()
 */
function forschungsatlas_admin_orgtype_delete_confirm($form, &$form_state, $orgtype) {
  $form_state['NEFO']['otid'] = $orgtype['otid'];
  $form_state['NEFO']['name'] = $orgtype['name'];
  $form['otid'] = array('#type' => 'value', '#value' => $orgtype['otid']);
  $output = t('Are you sure you want to delete this organization type: <em>@name</em>?', array('@name' => $orgtype['name']));
  return confirm_form($form,
      $output,
      FORSCHUNGSATLAS_CONFIG_PATH. '/institutions/organizationtypes',
      t('This action cannot be undone.'),
      t('Delete'),
      t('Cancel')
  );
}


/**
 * Execute organization type deletion.
 *
 * @see forschungsatlas_admin_orgtype_delete_confirm()
 */
function forschungsatlas_admin_orgtype_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $otid = intval($form_state['NEFO']['otid']);
    _forschungsatlas_delete_orgtype($otid);
    $output = t('The organization type <em>@name</em> has been deleted.', array('@name' => $form_state['NEFO']['name']));
    watchdog('NEFO', $output);
    drupal_set_message($output);
  }

  $form_state['redirect'] = FORSCHUNGSATLAS_CONFIG_PATH. '/institutions/organizationtypes';
}

/**
 * Delete an organization type
 * @param integer $otid
 *  The organization type ID
 */
function _forschungsatlas_delete_orgtype($otid) {
  db_delete('forschungsatlas__organization_type')
    ->condition('otid', $otid)
    ->execute();
}

/**
 * Find usage of a given organization type
 *
 * @param integer $otid
 *  The organization type ID
 */
function _forschungsatlas_find_orgtype_usage($otid) {
  $num_rows = db_query('SELECT count(iid) FROM {forschungsatlas__institutions}
      WHERE otid = :otid', array(':otid' => $otid))->fetchField();

  return $num_rows;
}


/**
 * Validations
 */
function _forschungsatlas_findDuplicateOrgtype($name, $otid) {

  return forschungsatlas_find_duplicate_name('forschungsatlas__organization_type', 'otid', $name, $otid);
}


