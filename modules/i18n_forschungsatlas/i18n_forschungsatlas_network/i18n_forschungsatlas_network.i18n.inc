<?php
/**
 * @file
 * Internationalization (i18n) hooks
 */

/**
 * Implements hook_i18n_string_info().
 */
function i18n_forschungsatlas_network_i18n_string_info() {
  $groups['forschungsatlas_network'] = array(
    'title' => t('Forschungsatlas Research Network'),
    'description' => t('Configurable Forschungsatlas Research Network.'),
    'format' => FALSE,
    'list' => TRUE,
  );

  return $groups;
}

/**
 * Implements hook_i18n_object_info().
 */
function i18n_forschungsatlas_network_i18n_object_info() {
  $info['forschungsatlas_network'] = array(
    'title' => t('Research Network Category'),
    'key' => 'rnid',
    'load callback' => 'forschungsatlas_network_load',
    'placeholders' => array(
      '%forschungsatlas_network' => 'rnid',
    ),
    'edit path' => FORSCHUNGSATLAS_CONFIG_PATH .'/categories/networks/%forschungsatlas_network/edit',
    'translate tab' => FORSCHUNGSATLAS_CONFIG_PATH .'/categories/networks/%forschungsatlas_network/translate',
    'string translation' => array(
      'textgroup' => 'forschungsatlas_network',
      'type' => 'researchnetwork',
      'table' => 'forschungsatlas__research_network',
      'properties' => array(
        'name' => t('Name'),
      ),
      'translate path' => FORSCHUNGSATLAS_CONFIG_PATH .'/categories/networks/%forschungsatlas_network/translate/%i18n_language',
    )
  );

  return $info;
}

/**
 * Implements hook_i18n_string_list().
 */
function i18n_forschungsatlas_network_i18n_string_list($group) {
  if ($group == 'forschungsatlas_network' || $group == 'all') {
    $strings = array();
    $query = db_select('forschungsatlas__research_network', 't')->fields('t');
    $result = $query->execute()->fetchAll();
    foreach ($result as $network) {
      $strings['forschungsatlas_network']['researchnetwork'][$network->rnid]['name'] = $network->name;
    }

    return $strings;
  }
}
