<?php
/**
 * @file
 * Internationalization (i18n) hooks
 */

/**
 * Implements hook_i18n_string_info().
 */
function i18n_forschungsatlas_methodology_i18n_string_info() {
  $groups['forschungsatlas_methodology'] = array(
    'title' => t('Forschungsatlas Methodology'),
    'description' => t('Configurable Forschungsatlas Methodology.'),
    'format' => FALSE,
    'list' => TRUE,
  );

  return $groups;
}

/**
 * Implements hook_i18n_object_info().
 */
function i18n_forschungsatlas_methodology_i18n_object_info() {
  $info['forschungsatlas_methodology'] = array(
    'title' => t('Methodology Category'),
    'key' => 'mid',
    'load callback' => 'forschungsatlas_methodology_load',
    'placeholders' => array(
      '%forschungsatlas_methodology' => 'mid',
    ),
    'edit path' => FORSCHUNGSATLAS_CONFIG_PATH .'/categories/methodologies/%forschungsatlas_methodology/edit',
    'translate tab' => FORSCHUNGSATLAS_CONFIG_PATH .'/categories/methodologies/%forschungsatlas_methodology/translate',
    'string translation' => array(
      'textgroup' => 'forschungsatlas_methodology',
      'type' => 'methodology',
      'table' => 'forschungsatlas__methodology',
      'properties' => array(
        'name' => t('Name'),
      ),
      'translate path' => FORSCHUNGSATLAS_CONFIG_PATH .'/categories/methodologies/%forschungsatlas_methodology/translate/%i18n_language',
    )
  );

  return $info;
}

/**
 * Implements hook_i18n_string_list().
 */
function i18n_forschungsatlas_methodology_i18n_string_list($group) {
  if ($group == 'forschungsatlas_methodology' || $group == 'all') {
    $strings = array();
    $query = db_select('forschungsatlas__methodology', 't')->fields('t');
    $result = $query->execute()->fetchAll();
    foreach ($result as $methodology) {
      $strings['forschungsatlas_methodology']['methodology'][$methodology->mid]['name'] = $methodology->name;
    }

    return $strings;
  }
}


