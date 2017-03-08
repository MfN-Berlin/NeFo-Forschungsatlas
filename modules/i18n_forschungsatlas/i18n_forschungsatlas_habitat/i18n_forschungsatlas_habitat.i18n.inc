<?php
/**
 * @file
 * Internationalization (i18n) hooks
 */

/**
 * Implements hook_i18n_string_info().
 */
function i18n_forschungsatlas_habitat_i18n_string_info() {
  $groups['forschungsatlas_habitat'] = array(
    'title' => t('Forschungsatlas Habitat'),
    'description' => t('Configurable Forschungsatlas Habitat.'),
    'format' => FALSE,
    'list' => TRUE,
  );

  return $groups;
}

/**
 * Implements hook_i18n_object_info().
 */
function i18n_forschungsatlas_habitat_i18n_object_info() {
  $info['forschungsatlas_habitat'] = array(
    'title' => t('Habitat Category'),
    'key' => 'hid',
    'load callback' => 'forschungsatlas_habitat_load',
    'placeholders' => array(
      '%forschungsatlas_habitat' => 'hid',
    ),
    'edit path' => FORSCHUNGSATLAS_CONFIG_PATH .'/categories/habitats/%forschungsatlas_habitat/edit',
    'translate tab' => FORSCHUNGSATLAS_CONFIG_PATH .'/categories/habitats/%forschungsatlas_habitat/translate',
    'string translation' => array(
      'textgroup' => 'forschungsatlas_habitat',
      'type' => 'habitat',
      'table' => 'forschungsatlas__habitats',
      'properties' => array(
        'name' => t('Name'),
      ),
      'translate path' => FORSCHUNGSATLAS_CONFIG_PATH .'/categories/habitats/%forschungsatlas_habitat/translate/%i18n_language',
    )
  );

  return $info;
}

/**
 * Implements hook_i18n_string_list().
 */
function i18n_forschungsatlas_habitat_i18n_string_list($group) {
  if ($group == 'forschungsatlas_habitat' || $group == 'all') {
    $strings = array();
    $query = db_select('forschungsatlas__habitats', 't')->fields('t');
    $result = $query->execute()->fetchAll();
    foreach ($result as $habitat) {
      $strings['forschungsatlas_habitat']['habitat'][$habitat->hid]['name'] = $habitat->name;
    }

    return $strings;
  }
}
