<?php
/**
 * @file
 * Internationalization (i18n) hooks
 */

/**
 * Implements hook_i18n_string_info().
 */
function i18n_forschungsatlas_intersubject_i18n_string_info() {
  $groups['forschungsatlas_intersubject'] = array(
    'title' => t('Forschungsatlas Interdisciplinary Subject'),
    'description' => t('Configurable Forschungsatlas Interdisciplinary Subject.'),
    'format' => FALSE,
    'list' => TRUE,
  );

  return $groups;
}

/**
 * Implements hook_i18n_object_info().
 */
function i18n_forschungsatlas_intersubject_i18n_object_info() {
  $info['forschungsatlas_intersubject'] = array(
    'title' => t('Interdisciplinary Subject Category'),
    'key' => 'isid',
    'load callback' => 'forschungsatlas_intersubject_load',
    'placeholders' => array(
      '%forschungsatlas_intersubject' => 'isid',
    ),
    'edit path' => FORSCHUNGSATLAS_CONFIG_PATH .'/categories/intersubjects/%forschungsatlas_intersubject/edit',
    'translate tab' => FORSCHUNGSATLAS_CONFIG_PATH .'/categories/intersubjects/%forschungsatlas_intersubject/translate',
    'string translation' => array(
      'textgroup' => 'forschungsatlas_intersubject',
      'type' => 'interdisciplinarysubject',
      'table' => 'forschungsatlas__interdisciplinary_subject',
      'properties' => array(
        'name' => t('Name'),
      ),
      'translate path' => FORSCHUNGSATLAS_CONFIG_PATH .'/categories/intersubjects/%forschungsatlas_intersubject/translate/%i18n_language',
    )
  );

  return $info;
}

/**
 * Implements hook_i18n_string_list().
 */
function i18n_forschungsatlas_intersubject_i18n_string_list($group) {
  if ($group == 'forschungsatlas_intersubject' || $group == 'all') {
    $strings = array();
    $query = db_select('forschungsatlas__interdisciplinary_subject', 't')->fields('t');
    $result = $query->execute()->fetchAll();
    foreach ($result as $intersubject) {
      $strings['forschungsatlas_intersubject']['interdisciplinarysubject'][$intersubject->isid]['name'] = $intersubject->name;
    }

    return $strings;
  }
}
