<?php
/**
 * @file
 * Internationalization (i18n) hooks
 */

/**
 * Implements hook_i18n_string_info().
 */
function i18n_forschungsatlas_topic_i18n_string_info() {
  $groups['forschungsatlas_topic'] = array(
    'title' => t('Forschungsatlas Topic'),
    'description' => t('Configurable Forschungsatlas Topic.'),
    'format' => FALSE,
    'list' => TRUE,
  );

  return $groups;
}

/**
 * Implements hook_i18n_object_info().
 */
function i18n_forschungsatlas_topic_i18n_object_info() {
  $info['forschungsatlas_topic'] = array(
    'title' => t('Topic Category'),
    'key' => 'tid',
    'load callback' => 'forschungsatlas_topic_load',
    'placeholders' => array(
      '%forschungsatlas_topic' => 'tid',
    ),
    'edit path' => FORSCHUNGSATLAS_CONFIG_PATH .'/categories/topics/%forschungsatlas_topic/edit',
    'translate tab' => FORSCHUNGSATLAS_CONFIG_PATH .'/categories/topics/%forschungsatlas_topic/translate',
    'string translation' => array(
      'textgroup' => 'forschungsatlas_topic',
      'type' => 'topic',
      'table' => 'forschungsatlas__topics',
      'properties' => array(
        'name' => t('Name'),
      ),
      'translate path' => FORSCHUNGSATLAS_CONFIG_PATH .'/categories/topics/%forschungsatlas_topic/translate/%i18n_language',
    )
  );

  return $info;
}

/**
 * Implements hook_i18n_string_list().
 */
function i18n_forschungsatlas_topic_i18n_string_list($group) {
  if ($group == 'forschungsatlas_topic' || $group == 'all') {
    $strings = array();
    $query = db_select('forschungsatlas__topics', 't')->fields('t');
    $result = $query->execute()->fetchAll();
    foreach ($result as $topic) {
      $strings['forschungsatlas_topic']['topic'][$topic->tid]['name'] = $topic->name;
    }

    return $strings;
  }
}


