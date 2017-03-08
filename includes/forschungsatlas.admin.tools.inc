<?php
/**
 * @file
 * nefo.admin.tools.inc
 */

/**
 * WKT
 */

/**
 * Table sort builder with pagination
 */
function forschungsatlas_admin_wkt() {
  $destination = drupal_get_destination();

  drupal_set_title(t('Institutions with missing WKT value'));

  $header = array();
  $header[] = array('data' => t('Institution'), 'field' => 'name', 'sort' => 'asc');
  $header[] = array('data' => t('Street'), 'field' => 'street');
  $header[] = array('data' => t('Postal Code'), 'field' => 'postalcode');
  $header[] = array('data' => t('City'), 'field' => 'city');
  $header[] = array('data' => t('Federal State'), 'field' => 'federalstate');
  $header[] = array('data' => t('Operation'));

  $query = db_select('forschungsatlas__institutions', 'i')->extend('PagerDefault')->extend('TableSort');
  $query->leftJoin('forschungsatlas__cities', 'c', 'i.cid = c.cid');
  $query->leftJoin('forschungsatlas__federal_states', 'fs', 'i.fsid = fs.fsid');
  $query->leftJoin('forschungsatlas__institution_geolocation', 'g', 'i.iid = g.iid');
  $query->fields('i', array('iid', 'name', 'street', 'postalcode'));
  $query->addField('c', 'name', 'city');
  $query->addField('fs', 'name', 'federalstate');
  $query->condition('g.geolocation_wkt', NULL,'is')
    ->limit(FORSCHUNGSATLAS_PAGER)
    ->orderByHeader($header);
  $result = $query->execute();

  $rows = array();
  foreach ($result as $data) {
    $row = array();
    $row['data']['name'] = check_plain($data->name);
    $row['data']['street'] = check_plain($data->street);
    $row['data']['postalcode'] = check_plain($data->postalcode);
    $row['data']['city'] = check_plain($data->city);
    $row['data']['federalstate'] = $data->federalstate;
    $operations = array();
    $operations['edit'] = array(
        'title' => t('edit'),
		'href' => FORSCHUNGSATLAS_CONFIG_PATH. '/institutions/institution/'. $data->iid .'/edit',
		'query' => $destination,
    );
    $row['data']['operations'] = array(
		'data' => array(
		'#theme' => 'links',
		'#links' => $operations,
		'#attributes' => array('class' => array('links', 'inline', 'nowrap')),
		),
    );
	$rows[] = $row;
  } // foreach()
  $build['forschungsatlas_table'] = array(
    '#theme' => 'table',
	'#header' => $header,
	'#rows' => $rows,
	'#empty' => t('There are no institutions with empty WKT.'),
    '#attributes' => array('id' => 'forschungsatlas-table-wkt'),
  );
  $build['forschungsatlas_pager'] = array(
    '#markup' => theme('pager'),
  );

  return $build;
}

/**
 * Build the geolocation values of institutions with empty WKT
 *
 * @param $form
 * @param $form_state
 */
function forschungsatlas_admin_wkt_rebuild($form, &$form_state) {
  $result = db_query('SELECT i.iid AS iid, i.name AS name, i.street AS street, i.postalcode AS postalcode,
      c.name AS city, fs.name AS federalstate, g.geolocation_wkt AS wkt
      FROM {forschungsatlas__institutions} i
      LEFT JOIN {forschungsatlas__cities} c ON i.cid = c.cid
      LEFT JOIN {forschungsatlas__federal_states} fs ON i.fsid = fs.fsid
      LEFT JOIN {forschungsatlas__institution_geolocation} g ON i.iid = g.iid
      WHERE g.geolocation_wkt IS NULL ORDER BY iid');
  $num_rows = 0;
  foreach ($result as $row) {
    $num_rows++;
    // Use geocoder in live system
    $geolocation = forschungsatlas_generate_wkt($row->street, $row->postalcode, $row->city, $row->federalstate);
    if (!empty($geolocation)) {
        $geolocation['iid'] = $row->iid;
        forschungsatlas_write_wkt($geolocation);
    }
    else {
        drupal_set_message(t('The WKT of <em>@name</em> could not be generated. Please verify the address.', array('@name' => $row->name)), 'error');
    }
  } // foreach()
  if ($num_rows > 0) {
    drupal_set_message(t('The list has been updated.'));
  }
  else {
    drupal_set_message(t('There is nothing to be updated.'));
  }
  drupal_goto(FORSCHUNGSATLAS_CONFIG_PATH. '/tools/wkt');
}


/**
 * URL: Broken links to Institutions
 */

/**
 * Table sort builder with pagination
 */
function forschungsatlas_admin_url() {
  global $pager_total_items;
  $destination = drupal_get_destination();

  drupal_set_title(t('Institutions with broken links'));

  $header = array();
  $header[] = array('data' => t('Institution'), 'field' => 'name', 'sort' => 'asc');
  $header[] = array('data' => t('City'), 'field' => 'city');
  $header[] = array('data' => t('Federal State'), 'field' => 'federalstate');
  $header[] = array('data' => t('URL'), 'field' => 'url');
  $header[] = array('data' => t('Updated'), 'field' => 'changed');
  $header[] = array('data' => t('Operation'));

  $query = db_select('forschungsatlas__tools_broken_links', 'urls')->extend('PagerDefault')->extend('TableSort');
  $query->join('forschungsatlas__institutions', 'i', 'urls.iid=i.iid');
  $query->join('forschungsatlas__cities', 'c', 'i.cid = c.cid');
  $query->join('forschungsatlas__federal_states', 'fs', 'i.fsid = fs.fsid');
  $query->fields('i', array('iid', 'name', 'url', 'changed'));
  $query->addField('c', 'name', 'city');
  $query->addField('fs', 'name', 'federalstate');
  $query->limit(FORSCHUNGSATLAS_PAGER)
    ->orderByHeader($header);
  $result = $query->execute();

  $rows = array();
  foreach ($result as $data) {
    $row = array();
    $row['data']['name'] = check_plain($data->name);
    $row['data']['city'] = check_plain($data->city);
    $row['data']['federalstate'] = $data->federalstate;
    $url = check_url($data->url);
    $row['data']['url'] = l($url, $url, array(
        'attributes' => array(
            'target'=>'blank',
            'class' => 'forschungsatlas-institution-l-ext',
        )
      )
    );
    $row['data']['changed'] = format_date($data->changed, 'short');
    $operations = array();
    $operations['edit'] = array(
        'title' => t('edit'),
        'href' => FORSCHUNGSATLAS_CONFIG_PATH. '/institutions/institution/'. $data->iid .'/edit',
        'query' => $destination,
    );
    $row['data']['operations'] = array(
        'data' => array(
            '#theme' => 'links',
            '#links' => $operations,
            '#attributes' => array('class' => array('links', 'inline', 'nowrap')),
        ),
    );
    $rows[] = $row;
  } // foreach()
  $build['forschungsatlas_table'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#empty' => t('There are no institutions with broken links.'),
      '#attributes' => array('id' => 'forschungsatlas-table-url'),
      '#caption' => (!empty($pager_total_items[0]) ? format_plural($pager_total_items[0], '1 result', '@count results') : ''),
  );
  $build['forschungsatlas_pager'] = array(
      '#markup' => theme('pager'),
  );

  return $build;
}

/**
 * Build the list of institutions with broken links
 *
 * @param $form
 * @param $form_state
 */
function forschungsatlas_admin_url_rebuild($form, &$form_state) {
  $num_rows = forschungsatlas_find_broken_links();
  if ($num_rows > 0) {
    drupal_set_message(t('The list has been updated.'));
  }
  else {
    drupal_set_message(t('There is nothing to be updated.'));
  }
  drupal_goto(FORSCHUNGSATLAS_CONFIG_PATH. '/tools/url');
}




