<?php
/**
 * @file
 * Internationalization (i18n) hooks
 */

/**
 * Implements hook_i18n_string_info().
 */
function i18n_forschungsatlas_orgtype_i18n_string_info() {
  $groups['forschungsatlas_orgtype'] = array(
    'title' => t('Forschungsatlas Organization Type'),
    'description' => t('Configurable Forschungsatlas Organization Type.'),
    'format' => FALSE,
    'list' => TRUE,
  );

  return $groups;
}

/**
 * Implements hook_i18n_object_info().
 */
function i18n_forschungsatlas_orgtype_i18n_object_info() {
  $info['forschungsatlas_orgtype'] = array(
    'title' => t('Organization Type Category'),
    'key' => 'otid',
    'load callback' => 'forschungsatlas_orgtype_load',
    'placeholders' => array(
      '%forschungsatlas_orgtype' => 'otid',
    ),
    'edit path' => FORSCHUNGSATLAS_CONFIG_PATH .'/institutions/organizationtypes/%forschungsatlas_orgtype/edit',
    'translate tab' => FORSCHUNGSATLAS_CONFIG_PATH .'/institutions/organizationtypes/%forschungsatlas_orgtype/translate',
    'string translation' => array(
      'textgroup' => 'forschungsatlas_orgtype',
      'type' => 'orgtype',
      'table' => 'forschungsatlas__organization_type',
      'properties' => array(
        'name' => t('Name'),
      ),
      'translate path' => FORSCHUNGSATLAS_CONFIG_PATH .'/institutions/organizationtypes/%forschungsatlas_orgtype/translate/%i18n_language',
    )
  );

  return $info;
}

/**
 * Implements hook_i18n_string_list().
 */
function i18n_forschungsatlas_orgtype_i18n_string_list($group) {
  if ($group == 'forschungsatlas_orgtype' || $group == 'all') {
    $strings = array();
    $query = db_select('forschungsatlas__organization_type', 't')->fields('t');
    $result = $query->execute()->fetchAll();
    foreach ($result as $orgtype) {
      $strings['forschungsatlas_orgtype']['orgtype'][$orgtype->otid]['name'] = $orgtype->name;
    }

    return $strings;
  }
}


