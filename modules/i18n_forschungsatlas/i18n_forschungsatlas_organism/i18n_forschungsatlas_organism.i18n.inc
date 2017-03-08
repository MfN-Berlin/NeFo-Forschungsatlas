<?php
/**
 * @file
 * Internationalization (i18n) hooks
 */

/**
 * Implements hook_i18n_string_info().
 */
function i18n_forschungsatlas_organism_i18n_string_info() {
  $groups['forschungsatlas_organism'] = array(
    'title' => t('Forschungsatlas Organism'),
    'description' => t('Configurable Forschungsatlas Organism.'),
    'format' => FALSE,
    'list' => TRUE,
  );

  return $groups;
}

/**
 * Implements hook_i18n_object_info().
 */
function i18n_forschungsatlas_organism_i18n_object_info() {
  $info['forschungsatlas_organism'] = array(
    'title' => t('Organism Category'),
    'key' => 'oid',
    'load callback' => 'forschungsatlas_organism_load',
    'placeholders' => array(
      '%forschungsatlas_organism' => 'oid',
    ),
    'edit path' => FORSCHUNGSATLAS_CONFIG_PATH .'/categories/organisms/%forschungsatlas_organism/edit',
    'translate tab' => FORSCHUNGSATLAS_CONFIG_PATH .'/categories/organisms/%forschungsatlas_organism/translate',
    'string translation' => array(
      'textgroup' => 'forschungsatlas_organism',
      'type' => 'organism',
      'table' => 'forschungsatlas__organisms',
      'properties' => array(
        'name' => t('Name'),
      ),
      'translate path' => FORSCHUNGSATLAS_CONFIG_PATH .'/categories/organisms/%forschungsatlas_organism/translate/%i18n_language',
    )
  );

  return $info;
}

/**
 * Implements hook_i18n_string_list().
 */
function i18n_forschungsatlas_organism_i18n_string_list($group) {
  if ($group == 'forschungsatlas_organism' || $group == 'all') {
    $strings = array();
    $query = db_select('forschungsatlas__organisms', 't')->fields('t');
    $result = $query->execute()->fetchAll();
    foreach ($result as $organism) {
      $strings['forschungsatlas_organism']['organism'][$organism->oid]['name'] = $organism->name;
    }

    return $strings;
  }
}


