<?php
/**
 * @file
 * forschungsatlas.admin.topic.inc
 */

/**
 * Table sort builder with pagination
 */
function forschungsatlas_admin_topics() {
  $header = array();
  $header[] = array('data' => t('Name'), 'field' => 'name', 'sort' => 'asc');
  $header[] = array('data' => t('Parent'), 'field' => 'parent');
  $header[] = array('data' => t('Operation'));
  $query = db_select('forschungsatlas__topics', 't')
    ->extend('TableSort')
    ->fields('t', array('tid', 'name', 'parent'));
  $table_sort = $query
    ->orderByHeader($header);
  $pager = $table_sort->extend('PagerDefault')
    ->limit(FORSCHUNGSATLAS_PAGER);
  $result = $pager->execute();
  $rows = array();
  foreach ($result as $data) {
    $row = array();
    $row['data']['name'] = check_plain($data->name);
    $row['data']['parent'] = check_plain(forschungsatlas_get_topic_name($data->parent));
    $operations = array();
    $operations['edit'] = array(
      'title' => t('edit'),
      'href' => FORSCHUNGSATLAS_CONFIG_PATH. '/categories/topics/'. $data->tid .'/edit',
    );
    $operations['delete'] = array(
      'title' => t('delete'),
      'href' => FORSCHUNGSATLAS_CONFIG_PATH. '/categories/topics/' . $data->tid . '/delete',
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
    '#attributes' => array('id' => 'forschungsatlas-table-topics'),
  );
  $build['forschungsatlas_pager'] = array(
    '#markup' => theme('pager'),
  );

  return $build;;
}

/**
 * Form constructor for the topic form.
 *
 * @see forschungsatlas_admin_topic_form_validate()
 * @see forschungsatlas_admin_topic_form_submit()
 */
function forschungsatlas_admin_topic_form($form, &$form_state, $topic=array(
    'tid' => '', 'name' => '', 'parent' => '')){

  $tid = intval($topic['tid']);
  $parent = intval($topic['parent']);
  $parents = forschungsatlas_admin_get_parent_topics($tid);
  // Option for no parent
  $parents[0] = t('- None -');
  $form['topic'] = array(
    '#type' => 'fieldset',
    '#title' => t('Topic'),
  );
  $form['topic']['name'] = array(
      '#type' => 'textfield',
      '#title' => t('Name'),
      '#size' => 20,
      '#default_value' => $topic['name'],
      '#required' => TRUE,
  );
  $form['topic']['parent'] = array(
    '#type' => 'select',
    '#title' => t('Parent topic'),
    '#options' => $parents,
    '#default_value' => $parent,
    '#element_validate' => array('element_validate_integer'),
  );
  $form['tid'] = array(
    '#type' => 'hidden',
    '#value' => $tid,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Page callback: Returns a form to edit a topic.
 *
 * @param $topic
 *   An array containing the topic.
 *
 * @return
 *   A form to edit the topic.
 *
 * @see forschungsatlas_menu()
 */
function forschungsatlas_admin_topic_edit($topic = array()){
  drupal_set_title($topic['name']);

  return drupal_get_form('forschungsatlas_admin_topic_form', $topic);
}

/**
 * Submit function for forschungsatlas_admin_topic_form().
 *
 * @see forschungsatlas_admin_topic_form()
 * @see forschungsatlas_admin_topic_form_validate()
 */
function forschungsatlas_admin_topic_form_submit($form, &$form_state) {
  $table = 'forschungsatlas__topics';
  $record = new stdClass();
  $record->name = forschungsatlas_clean_name($form_state['values']['name']);
  $record->parent = $form_state['values']['parent'];
  $tid = intval($form_state['values']['tid']);
  if (!$tid) { // Insert
    $record->tid = '';
    drupal_write_record($table, $record);
    $output = t('The topic <em>@name</em> has been registered.', array('@name' => $record->name));
  }
  else { // Update
    $record->tid = $tid;
    $result = drupal_write_record($table, $record, 'tid');
    $output = t('The settings for the topic <em>@name</em> have been updated.', array('@name' => $record->name));
  }
  if ($record->tid) {
    $form_state['values']['tid'] = $record->tid;
    $form_state['tid'] = $record->tid;
  }
  watchdog('NEFO', $output);
  drupal_set_message($output);
  $form_state['redirect'] = FORSCHUNGSATLAS_CONFIG_PATH. '/categories/topics';
}

/**
 * Validate function for forschungsatlas_admin_topic_form().
 *
 * @see forschungsatlas_admin_topic_form()
 * @see forschungsatlas_admin_topic_form_submit()
 */
function forschungsatlas_admin_topic_form_validate($form, &$form_state) {
  // Parent topic should actually exist
  $parent = intval($form_state['values']['parent']);
  if ($parent && !forschungsatlas_get_topic_name($parent)) {
    watchdog('security', 'Possible SQL injection attempt! An illegal choice has been detected: PARENT.', array(), WATCHDOG_ALERT);
    form_set_error('parent', t('The server was unable to process the request due to an internal error.'));
  }
  // Duplicate
  $tid = intval($form_state['values']['tid']);
  if (_forschungsatlas_findDuplicateTopic($form_state['values']['name'], $tid) > 0) {
    form_set_error('name', t('An topic with this name already exists.'));
  }
}

/**
 * Form constructor for the topic deletion confirmation form.
 *
 * @see forschungsatlas_admin_topic_delete_confirm_submit()
 */
function forschungsatlas_admin_topic_delete_confirm($form, &$form_state, $topic) {
  $form_state['NEFO']['tid'] = $topic['tid'];
  $form_state['NEFO']['name'] = $topic['name'];
  $form_state['NEFO']['parent'] = $topic['parent'];
  $form['tid'] = array('#type' => 'value', '#value' => $topic['tid']);
  if ($topic['parent']) {
    $output = t('Are you sure you want to delete this topic: <em>@name</em>?', array('@name' => $topic['name']));
  }
  else {
    $output = t('Delete this parent topic with all its children: <em>@name</em>?', array('@name' => $topic['name']));
  }
  return confirm_form($form,
      $output,
      FORSCHUNGSATLAS_CONFIG_PATH. '/categories/topics',
      t('This action cannot be undone.'),
      t('Delete'),
      t('Cancel')
  );
}

/**
 * Execute topic deletion.
 *
 * @see forschungsatlas_admin_topic_delete_confirm()
 */
function forschungsatlas_admin_topic_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $tid = intval($form_state['NEFO']['tid']);
    $parent = intval($form_state['NEFO']['parent']);
    _forschungsatlas_delete_topic($tid, $parent);
    $output = t('The topic <em>@name</em> has been deleted.', array('@name' => $form_state['NEFO']['name']));
    watchdog('NEFO', $output);
    drupal_set_message($output);
  }

  $form_state['redirect'] = FORSCHUNGSATLAS_CONFIG_PATH. '/categories/topics';
}

/**
 * Delete a topic
 * @param integer $tid
 *  The topic ID
 * @param integer $parent
 *  The parent ID
 */
function _forschungsatlas_delete_topic($tid, $parent) {
  $or = db_or()->condition('tid', $tid);
  if (!$parent) {
    $or->condition('parent', $tid);
  }
  $result = db_select('forschungsatlas__topics', 't')
    ->fields('t', array('tid'))
    ->condition($or)
    ->execute();
  $ids = array();
  foreach ($result as $data) {
    $ids[] = $data->tid;
  }
  db_delete('forschungsatlas__institution_topic')
    ->condition('tid', $ids, 'IN')
    ->execute();
  db_delete('forschungsatlas__topics')
    ->condition('tid', $ids, 'IN')
    ->execute();
}

/**
 * Validations
 */
function forschungsatlas_admin_get_parent_topics($tid){
  $result = db_query('SELECT tid, name FROM {forschungsatlas__topics}
      WHERE tid!=:tid AND parent=0 ORDER BY name', array(':tid' => $tid));
  $rows = array();
  foreach ($result as $row) {
    $rows[$row->tid] = $row->name;
  }

  return $rows;
}

function _forschungsatlas_findDuplicateTopic($name, $tid) {
  return forschungsatlas_find_duplicate_name('forschungsatlas__topics', 'tid', $name, $tid);
}


