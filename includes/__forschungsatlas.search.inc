<?php
/**
 * @file
 * forschungsatlas.search.inc
 */

/**
 *  Menu callback. Show the advanced search.
 *
 *  @see forschungsatlas_search_filter_form()
 */
function forschungsatlas_search_institutions($form, &$form_state) {
  global $pager_total_items;

  $session = isset($_SESSION['forschungsatlas_search_filter']) ? $_SESSION['forschungsatlas_search_filter'] : forschungsatlas_search_init_filters();
  // Get values from callback
  $fsids = !empty($form_state['values']['fsids']) ? $form_state['values']['fsids'] : $session['fsids'];
  // Build form
  $form['#attributes']['class'][] = 'forschungsatlas-search-institutions';
  // Institution
  $form['institution'] = array(
      '#type' => 'fieldset',
      '#title' => t('Institution'),
      '#attributes' => array('class' => array('forschungsatlas-search-fieldset-institutions')),
  );
  $form['institution']['fsids'] = array(
    '#type' => 'select',
    '#multiple' => TRUE,
    '#chosen' => TRUE,
    '#title' => t('Federal State'),
    '#options' => forschungsatlas_get_federal_states(),
    '#default_value' => $fsids,
    '#ajax' => array(
      'callback' => 'forschungsatlas_search_filter_form_callback',
      'wrapper' => 'forschungsatlas-search-institutions-form-wrapper',
    ),
  );
  $form['institution']['cids'] = array(
    '#type' => 'select',
    '#multiple' => TRUE,
    '#chosen' => TRUE,
    '#title' => t('City'),
    '#options' => forschungsatlas_search_get_cities($fsids),
    '#default_value' => $session['cids'],
    '#prefix' => '<div id="forschungsatlas-search-institutions-form-wrapper">',
    '#suffix' => '</div>',
  );
  $form['institution']['otids'] = array(
    '#type' => 'select',
    '#multiple' => TRUE,
    '#chosen' => TRUE,
    '#title' => t('Organization Type'),
    '#options' => forschungsatlas_get_organization_types(),
    '#default_value' => $session['otids'],
  );
  $form['institution']['name'] = array(
    '#type' => 'textfield',
	'#title' => t('Name') .' ('. t('text search') .')',
	'#size' => 20,
	'#default_value' => $session['name'],
  );
  // Categories
  $form['categories'] = array(
      '#type' => 'fieldset',
      '#title' => t('Categories'),
      '#attributes' => array('class' => array('forschungsatlas-search-fieldset-categories')),
  );
  $form['categories']['rnids'] = array(
    '#type' => 'select',
    '#multiple' => TRUE,
    '#chosen' => TRUE,
    '#title' => t('Research Network'),
    '#options' => forschungsatlas_search_get_research_network(),
    '#default_value' => $session['rnids'],
  );
  $form['categories']['hids'] = array(
    '#type' => 'select',
    '#multiple' => TRUE,
    '#chosen' => TRUE,
    '#title' => t('Habitats'),
    '#options' => forschungsatlas_search_get_habitats(),
    '#default_value' => $session['hids'],
  );
  $form['categories']['mids'] = array(
    '#type' => 'select',
    '#multiple' => TRUE,
    '#chosen' => TRUE,
    '#title' => t('Methodologies'),
    '#options' => forschungsatlas_search_get_methodologies(),
    '#default_value' => $session['mids'],
  );
  $form['categories']['oids'] = array(
    '#type' => 'select',
    '#multiple' => TRUE,
    '#chosen' => TRUE,
    '#title' => t('Organisms'),
    '#options' => forschungsatlas_search_get_organisms(),
    '#default_value' => $session['oids'],
  );
  $form['categories']['isids'] = array(
    '#type' => 'select',
    '#multiple' => TRUE,
    '#chosen' => TRUE,
    '#title' => t('Interdisciplinary Subjects'),
    '#options' => forschungsatlas_search_get_interdisciplinary_subjects(),
    '#default_value' => $session['isids'],
  );
  $form['categories']['tids'] = array(
    '#type' => 'select',
    '#multiple' => TRUE,
    '#chosen' => TRUE,
    '#title' => t('Research Disciplines'),
    '#options' => forschungsatlas_search_get_research_topics(),
    '#default_value' => $session['tids'],
  );
  // Submit
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Search'),
  );
  if ($session['adjusted']) {
    $form['actions']['reset'] = array('#type' => 'submit', '#value' => t('Reset'));
  }

  // Build the sortable table header.
  $header = array();
  $header[] = array('data' => t('Name'), 'field' => 'ti.compositedname', 'sort' => 'asc');
  $header[] = array('data' => t('City'), 'field' => 'ti.city');
  $header[] = array('data' => t('URL'));
  $rows = array();
  try {
    $query = db_select('forschungsatlas__aux_institutions', 'ti')->extend('PagerDefault')->extend('TableSort');
    forschungsatlas_search_institutions_build_filter_query($query, $session);
    $result = $query->fields('ti', array('iid', 'name', 'city', 'url', 'assembledparentname'))
      ->limit(FORSCHUNGSATLAS_PAGER)
      ->orderByHeader($header)
      ->execute();
  }
  catch (Exception $e) {
    watchdog_exception('NEFO', $e);
    $_SESSION['forschungsatlas_search_filter'] = forschungsatlas_search_init_filters();
    throw $e;
  }

  foreach ($result as $data) {
    $row = array();
    $row['data']['name'] = forschungsatlas_get_parent_name_list($data->iid, $data->name, $data->assembledparentname);
    $row['data']['city'] = check_plain($data->city);
    if (!empty($data->url)) {
      $url = check_url($data->url);
      $row['data']['url'] = l(t('Link'), $url, array(
          'attributes' => array(
              'target'=>'blank',
              'class' => 'forschungsatlas-institution-l-ext',
          )
        )
      );
    }
    else {
      $row['data']['url'] = '';
    }
      $rows[] = $row;
  } // foreach()

  $form['forschungsatlas_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No content available.'),
    '#attributes' => array('id' => 'forschungsatlas-table-result-list'),
    '#caption' => (!empty($pager_total_items[0]) ? format_plural($pager_total_items[0], '1 result', '@count results') : ''),
  );
  $form['forschungsatlas_pager'] = array(
      '#markup' => theme('pager')
  );

  return $form;
}

/**
 * Selects just the city dropdown to be returned for re-rendering.
 *
 * @return array
 *   Renderable array
 */
function forschungsatlas_search_filter_form_callback($form, $form_state) {
  return $form['institution']['cids'];
}

/**
 * Form submission handler for forschungsatlas_search_filter_form().
 *
 * @see forschungsatlas_search_institutions()
 * @see forschungsatlas_institutions_list()
 */
function forschungsatlas_search_institutions_submit($form, &$form_state) {
  switch ($form_state['values']['op']) {
    case t('Search'):
      $_SESSION['forschungsatlas_search_filter'] = forschungsatlas_search_init_filters();
      // Federal state
      if (!empty($form_state['values']['fsids'])) {
        $fsids = array_intersect_key($form_state['values']['fsids'], forschungsatlas_get_federal_states());
        $_SESSION['forschungsatlas_search_filter']['fsids'] = $fsids;
      }
      // City
      if (!empty($form_state['values']['cids'])) {
        $cids = array_intersect_key($form_state['values']['cids'], forschungsatlas_search_get_cities($_SESSION['forschungsatlas_search_filter']['fsids']));
        $_SESSION['forschungsatlas_search_filter']['cids']= $cids;
      }
      // Organization Type
      if (!empty($form_state['values']['otids'])) {
        $otids = array_intersect_key($form_state['values']['otids'], forschungsatlas_get_organization_types());
        $_SESSION['forschungsatlas_search_filter']['otids'] = $otids;
      }
      // Name
      if (!empty($form_state['values']['name'])) {
        $name = trim(forschungsatlas_clean_name($form_state['values']['name']));
        if (!empty($name)) {
          $_SESSION['forschungsatlas_search_filter']['name'] = $name;
        }
      }
      // Research Network
      if (!empty($form_state['values']['rnids'])) {
        $rnids = array_intersect_key($form_state['values']['rnids'], forschungsatlas_search_get_research_network());
        $_SESSION['forschungsatlas_search_filter']['rnids'] = $rnids;
      }
      // Habitats
      if (!empty($form_state['values']['hids'])) {
        $hids = array_intersect_key($form_state['values']['hids'], forschungsatlas_search_get_habitats());
        $_SESSION['forschungsatlas_search_filter']['hids'] = $hids;
      }
      // Methodologies
      if (!empty($form_state['values']['mids'])) {
        $mids = array_intersect_key($form_state['values']['mids'], forschungsatlas_search_get_methodologies());
        $_SESSION['forschungsatlas_search_filter']['mids'] = $mids;
      }
      // Organisms
      if (!empty($form_state['values']['oids'])) {
        $oids = array_intersect_key($form_state['values']['oids'], forschungsatlas_search_get_organisms());
        $_SESSION['forschungsatlas_search_filter']['oids'] = $oids;
      }
      // Interdisciplinary Subjects
      if (!empty($form_state['values']['isids'])) {
        $isids = array_intersect_key($form_state['values']['isids'], forschungsatlas_search_get_interdisciplinary_subjects());
        $_SESSION['forschungsatlas_search_filter']['isids'] = $isids;
      }
      // Research Topics
      if (!empty($form_state['values']['tids'])) {
        $tids = array_intersect_key($form_state['values']['tids'], forschungsatlas_search_get_research_topics());
        $_SESSION['forschungsatlas_search_filter']['tids'] = $tids;
      }
      if (!empty($_SESSION['forschungsatlas_search_filter']['fsids']) || !empty($_SESSION['forschungsatlas_search_filter']['cids'])
          || !empty($_SESSION['forschungsatlas_search_filter']['otids']) || !empty($_SESSION['forschungsatlas_search_filter']['name'])
          || !empty($_SESSION['forschungsatlas_search_filter']['rnids']) || !empty($_SESSION['forschungsatlas_search_filter']['hids'])
          || !empty($_SESSION['forschungsatlas_search_filter']['mids']) || !empty($_SESSION['forschungsatlas_search_filter']['oids'])
          || !empty($_SESSION['forschungsatlas_search_filter']['isids']) || !empty($_SESSION['forschungsatlas_search_filter']['tids'])) {
        $_SESSION['forschungsatlas_search_filter']['adjusted'] = TRUE;
      }
      $form_state['rebuild'] = TRUE;
      break;
    case t('Reset'):
      $_SESSION['forschungsatlas_search_filter'] = forschungsatlas_search_init_filters();
      break;
  }
}

/**
 * Apply filter for institutions based on session.
 *
 * @param $query
 *   A SelectQuery to which the filters should be applied.
 */
function forschungsatlas_search_institutions_build_filter_query(SelectQueryInterface $query, $filter_data=array()) {
  // Federal state
  if (!empty($filter_data['fsids'])) {
    $query->condition('ti.fsid', $filter_data['fsids'], 'IN');
  }
  // City
  if (!empty($filter_data['cids'])) {
    $query->condition('ti.cid', $filter_data['cids'], 'IN');
  }
  // Organization Type
  if (!empty($filter_data['otids'])) {
    $query->condition('ti.otid', $filter_data['otids'], 'IN');
  }
  // Name
  if (!empty($filter_data['name'])) {
    $words = explode(' ', $filter_data['name']);
    $db_or = db_or();
    foreach ($words as $word) {
      $db_or->condition('ti.name', '%' . db_like($word) . '%', 'LIKE');
    }
    $query->condition($db_or);
  }
  // Categories: Build subqueries
  // Research Network
  if (!empty($filter_data['rnids'])) {
    $options = _forschungsatlas_search_get_ids($filter_data['rnids']);
    $iids = _forschungsatlas_search_filter_categories('forschungsatlas__institution_researchnetwork', 'forschungsatlas__research_network', 'rnid', $options);
    if (!empty($iids)) {
      $query->condition('ti.iid', $iids, 'IN');
    }
  }
  // Habitats
  if (!empty($filter_data['hids'])) {
    $options = _forschungsatlas_search_get_ids($filter_data['hids']);
    $iids = _forschungsatlas_search_filter_categories('forschungsatlas__institution_habitat', 'forschungsatlas__habitats', 'hid', $options);
    if (!empty($iids)) {
      $query->condition('ti.iid', $iids, 'IN');
    }
  }
  // Methodologies
  if (!empty($filter_data['mids'])) {
    $options = _forschungsatlas_search_get_ids($filter_data['mids']);
    $iids = _forschungsatlas_search_filter_categories('forschungsatlas__institution_methodology', 'forschungsatlas__methodology', 'mid', $options);
    if (!empty($iids)) {
      $query->condition('ti.iid', $iids, 'IN');
    }
  }
  // Organisms
  if (!empty($filter_data['oids'])) {
    $options = _forschungsatlas_search_get_ids($filter_data['oids']);
    $iids = _forschungsatlas_search_filter_categories('forschungsatlas__institution_organism', 'forschungsatlas__organisms', 'oid', $options);
    if (!empty($iids)) {
      $query->condition('ti.iid', $iids, 'IN');
    }
  }
  // Interdisciplinary Subjects
  if (!empty($filter_data['isids'])) {
    $options = _forschungsatlas_search_get_ids($filter_data['isids']);
    $iids = _forschungsatlas_search_filter_categories('forschungsatlas__institution_intersubject', 'forschungsatlas__interdisciplinary_subject', 'isid', $options);
    if (!empty($iids)) {
      $query->condition('ti.iid', $iids, 'IN');
    }
  }
  // Research Topics
  if (!empty($filter_data['tids'])) {
    $options = _forschungsatlas_search_get_ids($filter_data['tids']);
    $iids = _forschungsatlas_search_filter_categories('forschungsatlas__institution_topic', 'forschungsatlas__topics', 'tid', $options);
    if (!empty($iids)) {
      $query->condition('ti.iid', $iids, 'IN');
    }
  }
}

/**
 * Extract ID and parent ID from composite key.
 *
 * @param array $options
 *  Array of composite keys
 *
 * @return array
 *  An array containing the IDs and Parent IDs.
 */
function _forschungsatlas_search_get_ids($options = array()) {
  $ids = array();
  $parents = array();
  foreach ($options AS $compkey) {
    list($id, $parent) = explode('-', $compkey);
    if (!$parent) {
      $parents[] = $id;
    }
    else {
      if (!in_array($parent, $parents)) {
        $ids[] = $id;
      }
    }
  }

  return array('ids' => $ids, 'parents' => $parents);
}


/**
 * Get list of used cities
 *
 * @return array
 *   Dropdown options.
 */
function forschungsatlas_search_get_cities($fsids=array()) {
  $cities = array();
  $where = (empty($fsids)) ? 'WHERE 1' : 'WHERE t1.fsid IN (:fsids)';
  $result = db_query("SELECT t1.cid AS cid, CONCAT(t1.name, ' (', t3.fscode, ')') AS name, t2.fsid AS fsid
    FROM {forschungsatlas__cities} t1
    JOIN {forschungsatlas__institutions} t2 ON t1.cid = t2.cid
    JOIN {forschungsatlas__federal_states} t3 ON t1.fsid = t3.fsid
    $where
    GROUP BY t1.cid ORDER BY name", array(':fsids' => $fsids));
  foreach ($result as $row) {
    $cities[$row->cid] = $row->name;
  }

  return $cities;
}

/**
 * Get research network options
 *
 * @return array
 *   Dropdown options.
 */
function forschungsatlas_search_get_research_network() {
  return _forschungsatlas_search_build_composited_options('forschungsatlas__research_network', 'rnid', 'forschungsatlas_network:researchnetwork');
}

/**
 * Get habitat options
 *
 * @return array
 *   Dropdown options.
 */
function forschungsatlas_search_get_habitats() {
  return _forschungsatlas_search_build_composited_options('forschungsatlas__habitats', 'hid', 'forschungsatlas_habitat:habitat');
}

/**
 * Get methodology options
 *
 * @return array
 *   Dropdown options.
 */
function forschungsatlas_search_get_methodologies() {
  return _forschungsatlas_search_build_composited_options('forschungsatlas__methodology', 'mid', 'forschungsatlas_methodology:methodology');
}

/**
 * Get organism options
 *
 * @return array
 *   Dropdown options.
 */
function forschungsatlas_search_get_organisms() {
  return _forschungsatlas_search_build_composited_options('forschungsatlas__organisms', 'oid', 'forschungsatlas_organism:organism');
}

/**
 * Get interdisciplinary subject options
 *
 * @return array
 *   Dropdown options.
 */
function forschungsatlas_search_get_interdisciplinary_subjects() {
  return _forschungsatlas_search_build_composited_options('forschungsatlas__interdisciplinary_subject', 'isid', 'forschungsatlas_intersubject:interdisciplinarysubject');
}

/**
 * Get topic options
 *
 * @return array
 *   Dropdown options.
 */
function forschungsatlas_search_get_research_topics() {
  return _forschungsatlas_search_build_composited_options('forschungsatlas__topics', 'tid', 'forschungsatlas_topic:topic');
}


/**
 * Initialise filter
 *
 * @return $filters
 *  An array containing the
 */
function forschungsatlas_search_init_filters() {
  return array(
      'fsids' => array(),
      'cids' => array(),
      'otids' => array(),
      'name' => '',
      'rnids' => array(),
      'crids' => array(),
      'hids' => array(),
      'mids' => array(),
      'oids' => array(),
      'isids' => array(),
      'tids' => array(),
      'adjusted' => FALSE,
  );
}

/**
 * Build a selectable options for a form element.
 *
 * @param string $table
 *  Table name
 * @param string $key
 *  Key name
 * @param string $object
 *  i18n Object
 *
 * @return
 *  An array in the form array(CompositedKey => FormattedName).
 */
function _forschungsatlas_search_build_composited_options($table, $key, $object) {
  $options = array();
  $comp_options = array();

  $result = db_query('SELECT t1.'. $key .' AS keyid, t1.name AS name, t1.parent AS parent, t2.name AS parentname
      FROM {'. $table .'} AS t1 LEFT JOIN {'. $table .'} AS t2 ON t1.parent = t2.'. $key);

  foreach ($result as $row) {
    if (module_exists('i18n_string') && module_exists('i18n_nefo')) {
      $tnname = i18n_string("$object:$row->keyid:name", $row->name);
      $tnparentname = (!empty($row->parentname)) ? i18n_string("$object:$row->parent:name", $row->parentname) : '';
    }
    else {
      $tnname = $row->name;
      $tnparentname = $row->parentname;
    }
    $keyid = $row->keyid;
    $parent = $row->parent;
    $compositedkey = $row->keyid .'-'. $row->parent;
    if (empty($tnparentname)) {
      $name = $tnname;
      $compositedname = $tnname;
    }
    else {
      $name = '- '. $tnname;
      $compositedname = $tnparentname .' > '. $tnname;
    }
    $comp_options[$compositedname] = array($key => $keyid, 'name' => $name, 'parent' => $parent, 'compositedkey' => $compositedkey);
  }
  ksort($comp_options);

  foreach ($comp_options as $row) {
    $options[$row['compositedkey']] = $row['name'];
  }

  return $options;
}

/**
 * Build array of iids by category.
 *
 * @param string $table_inst
 *  Table name
 * @param string $table_categ
 *  Table name
 * @param string $key
 *  Key name
 * @param array $options
 *  Array of category ids
 *
 * @return
 *  An array containing the iids.
 */
function _forschungsatlas_search_filter_categories($table_inst, $table_categ, $key, $options) {
  $iids = array();

  $result = db_query('SELECT t1.iid AS iid, t1.'. $key .' AS '. $key .', IF(t2.parent = 0, t2.'. $key .', t2.parent) AS GlobalParent
      FROM {'. $table_inst .'} t1 JOIN {'. $table_categ .'} t2 ON t1.'. $key .' = t2.'. $key);
  foreach ($result as $row) {
    if (in_array($row->$key, $options['ids']) || in_array($row->GlobalParent, $options['parents'])) {
      if (!in_array($row->iid, $iids)) {
        $iids[] = $row->iid;
      }
    }
  } //foreach()

  return $iids;
}



