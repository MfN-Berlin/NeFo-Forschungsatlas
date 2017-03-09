<?php
/**
 * @file
 * forschungsatlas.admin.intersubjects.inc
 */

/**
 * Table sort builder with pagination
 */
function forschungsatlas_admin_intersubjects() {
  $header = array();
  $header[] = array('data' => t('Name'), 'field' => 'name', 'sort' => 'asc');
  $header[] = array('data' => t('Parent'), 'field' => 'parent');
  $header[] = array('data' => t('Operation'));
  $query = db_select('forschungsatlas__interdisciplinary_subject', 'inters')
    ->extend('TableSort')
    ->fields('inters', array('isid', 'name', 'parent'));
  $table_sort = $query
    ->orderByHeader($header);
  $pager = $table_sort->extend('PagerDefault')
    ->limit(FORSCHUNGSATLAS_PAGER);
  $result = $pager->execute();
  $rows = array();
  foreach ($result as $data) {
    $row = array();
    $row['data']['name'] = check_plain($data->name);
    $row['data']['parent'] = check_plain(forschungsatlas_get_intsubject_name($data->parent));
    $operations = array();
    $operations['edit'] = array(
      'title' => t('edit'),
      'href' => FORSCHUNGSATLAS_CONFIG_PATH. '/categories/intersubjects/'. $data->isid .'/edit',
    );
    $operations['delete'] = array(
      'title' => t('delete'),
      'href' => FORSCHUNGSATLAS_CONFIG_PATH. '/categories/intersubjects/' . $data->isid . '/delete',
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
    '#attributes' => array('id' => 'forschungsatlas-table-intersubjects'),
  );
  $build['forschungsatlas_pager'] = array(
    '#markup' => theme('pager'),
  );

  return $build;;
}

/**
 * Form constructor for the interdisciplinary subject form.
 *
 * @see forschungsatlas_admin_intersubject_form_validate()
 * @see forschungsatlas_admin_intersubject_form_submit()
 */
function forschungsatlas_admin_intersubject_form($form, &$form_state, $intersubject=array(
    'isid' => '', 'name' => '', 'parent' => '')) {

  $isid = intval($intersubject['isid']);
  $parent = intval($intersubject['parent']);
  $parents = forschungsatlas_admin_get_parent_intsubjects($isid);
  // Option for no parent
  $parents[0] = t('- None -');
  $form['intersubject'] = array(
    '#type' => 'fieldset',
    '#title' => t('Interdisciplinary subject'),
  );
  $form['intersubject']['name'] = array(
      '#type' => 'textfield',
      '#title' => t('Name'),
      '#size' => 20,
      '#default_value' => $intersubject['name'],
      '#required' => TRUE,
  );
  $form['intersubject']['parent'] = array(
    '#type' => 'select',
    '#title' => t('Parent interdisciplinary subject'),
    '#options' => $parents,
    '#default_value' => $parent,
    '#element_validate' => array('element_validate_integer'),
  );
  $form['isid'] = array(
    '#type' => 'hidden',
    '#value' => $isid,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Page callback: Returns a form to edit an interdisciplinary subject.
 *
 * @param $intersubject
 *   An array containing the interdisciplinary subject.
 *
 * @return
 *   A form to edit the interdisciplinary subject.
 *
 * @see forschungsatlas_menu()
 */
function forschungsatlas_admin_intersubject_edit($intersubject = array()){
  drupal_set_title($intersubject['name']);

  return drupal_get_form('forschungsatlas_admin_intersubject_form', $intersubject);
}

/**
 * Submit function for forschungsatlas_admin_intersubject_form().
 *
 * @see forschungsatlas_admin_intersubject_form()
 * @see forschungsatlas_admin_intersubject_form_validate()
 */
function forschungsatlas_admin_intersubject_form_submit($form, &$form_state) {
  $table = 'forschungsatlas__interdisciplinary_subject';
  $record = new stdClass();
  $record->name = forschungsatlas_clean_name($form_state['values']['name']);
  $record->parent = $form_state['values']['parent'];
  $isid = intval($form_state['values']['isid']);
  if (!$isid) { // Insert
    $record->isid = '';
    drupal_write_record($table, $record);
    $output = t('The interdisciplinary subject <em>@name</em> has been registered.', array('@name' => $record->name));
  }
  else { // Update
    $record->isid = $isid;
    $result = drupal_write_record($table, $record, 'isid');
    $output = t('The settings for the interdisciplinary subject <em>@name</em> have been updated.', array('@name' => $record->name));
  }
  if ($record->isid) {
    $form_state['values']['isid'] = $record->isid;
    $form_state['isid'] = $record->isid;
  }
  watchdog('NEFO', $output);
  drupal_set_message($output);
  $form_state['redirect'] = FORSCHUNGSATLAS_CONFIG_PATH. '/categories/intersubjects';
}

/**
 * Validate function for forschungsatlas_admin_intersubject_form().
 *
 * @see forschungsatlas_admin_intersubject_form()
 * @see forschungsatlas_admin_intersubject_form_submit()
 */
function forschungsatlas_admin_intersubject_form_validate($form, &$form_state) {
  // Parent intersubjects should actually exist
  $parent = intval($form_state['values']['parent']);
  if ($parent && !forschungsatlas_get_intsubject_name($parent)) {
    watchdog('security', 'Possible SQL injection attempt! An illegal choice has been detected: PARENT.', array(), WATCHDOG_ALERT);
    form_set_error('parent', t('The server was unable to process the request due to an internal error.'));
  }
  // Duplicate
  $isid = intval($form_state['values']['isid']);
  if (_forschungsatlas_findDuplicateInterSubject($form_state['values']['name'], $isid) > 0) {
    form_set_error('name', t('An interdisciplinary subject with this name already exists.'));
  }
}


/**
 * Form constructor for the interdisciplinary subject deletion confirmation form.
 *
 * @see forschungsatlas_admin_intersubject_delete_confirm_submit()
 */
function forschungsatlas_admin_intersubject_delete_confirm($form, &$form_state, $intersubject) {
  $form_state['NEFO']['isid'] = $intersubject['isid'];
  $form_state['NEFO']['name'] = $intersubject['name'];
  $form_state['NEFO']['parent'] = $intersubject['parent'];
  $form['isid'] = array('#type' => 'value', '#value' => $intersubject['isid']);
  if ($intersubject['parent']) {
    $output = t('Are you sure you want to delete this interdisciplinary subject: <em>@name</em>?', array('@name' => $intersubject['name']));
  }
  else {
    $output = t('Delete this parent interdisciplinary subject with all its children: <em>@name</em>?', array('@name' => $intersubject['name']));
  }
  return confirm_form($form,
      $output,
      FORSCHUNGSATLAS_CONFIG_PATH. '/categories/intersubjects',
      t('This action cannot be undone.'),
      t('Delete'),
      t('Cancel')
  );
}

/**
 * Execute interdisciplinary subject deletion.
 *
 * @see forschungsatlas_admin_intersubject_delete_confirm()
 */
function forschungsatlas_admin_intersubject_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $isid = intval($form_state['NEFO']['isid']);
    $parent = intval($form_state['NEFO']['parent']);
    _forschungsatlas_delete_intersubject($isid, $parent);
    $output = t('The interdisciplinary subject <em>@name</em> has been deleted.', array('@name' => $form_state['NEFO']['name']));
    watchdog('NEFO', $output);
    drupal_set_message($output);
  }

  $form_state['redirect'] = FORSCHUNGSATLAS_CONFIG_PATH. '/categories/intersubjects';
}

/**
 * Delete a interdisciplinary subject
 * @param integer $isid
 *  The interdisciplinary subject ID
 * @param integer $parent
 *  The parent ID
 */
function _forschungsatlas_delete_intersubject($isid, $parent) {
  $or = db_or()->condition('isid', $isid);
  if (!$parent) {
    $or->condition('parent', $isid);
  }
  $result = db_select('forschungsatlas__interdisciplinary_subject', 't')
    ->fields('t', array('isid'))
    ->condition($or)
    ->execute();
  $ids = array();
  foreach ($result as $data) {
    $ids[] = $data->isid;
  }
  db_delete('forschungsatlas__institution_intersubject')
    ->condition('isid', $ids, 'IN')
    ->execute();
  db_delete('forschungsatlas__interdisciplinary_subject')
    ->condition('isid', $ids, 'IN')
    ->execute();
}


/**
 * Validations
 */
function forschungsatlas_admin_get_parent_intsubjects($isid){
  $result = db_query('SELECT isid, name FROM {forschungsatlas__interdisciplinary_subject}
      WHERE isid!=:isid AND parent=0 ORDER BY name', array(':isid' => $isid));
  $rows = array();
  foreach ($result as $row) {
    $rows[$row->isid] = $row->name;
  }

  return $rows;
}

function _forschungsatlas_findDuplicateInterSubject($name, $isid) {
  return forschungsatlas_find_duplicate_name('forschungsatlas__interdisciplinary_subject', 'isid', $name, $isid);
}




