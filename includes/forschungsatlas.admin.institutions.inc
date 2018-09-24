<?php
/**
 * @file
 * forschungsatlas.admin.institutions.inc
 */

/**
 * Table sort builder with pagination
 */
function forschungsatlas_admin_institutions() {
  $header = array();
  $header[] = array('data' => t('Name'), 'field' => 'name', 'sort' => 'asc');
  $header[] = array('data' => t('City'), 'field' => 'city');
  $header[] = array('data' => t('Federal State'), 'field' => 'federalstate');
  $header[] = array('data' => t('Organization Type'), 'field' => 'orgtype');
  $header[] = array('data' => t('Author'));
  $header[] = array('data' => t('Status'), 'field' => 'status');
  $header[] = array('data' => t('Updated'), 'field' => 'changed');
  $header[] = array('data' => t('Operations'));

  $destination = drupal_get_destination();
  $query = db_select('forschungsatlas__institutions', 'i')->extend('PagerDefault')->extend('TableSort');
  $query->leftJoin('forschungsatlas__cities', 'c', 'i.cid = c.cid');
  $query->leftJoin('forschungsatlas__federal_states', 'fs', 'i.fsid = fs.fsid');
  $query->leftJoin('forschungsatlas__organization_type', 'ot', 'i.otid = ot.otid');
  $query->fields('i', array('iid', 'name', 'status', 'uid', 'changed'));
  $query->addField('c', 'name', 'city');
  $query->addField('fs', 'name', 'federalstate');
  $query->addField('ot', 'name', 'orgtype');
  $query->limit(FORSCHUNGSATLAS_PAGER)
    ->orderByHeader($header);
  $result = $query->execute();

  $rows = array();
  foreach ($result as $data) {
    $row = array();
    $row['data']['name'] = array(
        'data' => array(
            '#type' => 'link',
            '#title' => check_plain($data->name),
            '#href' => FORSCHUNGSATLAS_PUBLIC_PATH . '/institutions/' . $data->iid,
        )
    );
    $row['data']['city'] = check_plain($data->city);
    $row['data']['federalstate'] = check_plain($data->federalstate);
    $row['data']['orgtype'] = check_plain($data->orgtype);
    $row['data']['author'] = theme('username', array('account' => user_load($data->uid)));
    $row['data']['status'] = $data->status ? t('published') : t('not published');
    $row['data']['changed'] = format_date($data->changed, 'short');
    $operations = array();
    $operations['edit'] = array(
      'title' => t('edit'),
      'href' => FORSCHUNGSATLAS_CONFIG_PATH. '/institutions/institution/' . $data->iid . '/edit',
      'query' => $destination,
    );
    $operations['delete'] = array(
      'title' => t('delete'),
      'href' => FORSCHUNGSATLAS_CONFIG_PATH. '/institutions/institution/' . $data->iid . '/delete',
      'query' => $destination,
    );
    $row['data']['operations'] = array(
      'data' => array(
        '#theme' => 'links',
        '#links' => $operations,
        '#attributes' => array('class' => array('links', 'inline', 'nowrap')),
      )
    );

    $rows[] = $row;
  }

  $build['forschungsatlas_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No items available.'),
    '#attributes' => array('id' => 'forschungsatlas-table-institutions'),
  );
  $build['forschungsatlas_pager'] = array(
    '#markup' => theme('pager'),
  );

  return $build;
}

/**
 * Form constructor for the institution form.
 *
 * @see forschungsatlas_admin_institution_form_validate()
 * @see forschungsatlas_admin_institution_form_submit()
 */
function forschungsatlas_admin_institution_form($form, &$form_state, $institution = array(
        'iid' => '', 'name' => '', 'abbrev' => '', 'street' => '', 'street2' => '', 'postalcode' => '', 'cid' => '',
		'fsid' => '', 'url' => '', 'otid' => '', 'description' => '', 'status' => 1)) {

	$iid = intval($institution['iid']);
	$cid = intval($institution['cid']);
	$fsid = intval($institution['fsid']);
	$otid = intval($institution['otid']);
	// Institution
	$form['institution'] = array(
	    '#type' => 'fieldset',
	    '#title' => t('Institution'),
	);
	$form['institution']['name'] = array(
		'#type' => 'textfield',
	    '#title' => t('Name'),
	    '#size' => 60,
	    '#default_value' => $institution['name'],
	    '#required' => TRUE,
	);
	$form['institution']['abbrev'] = array(
		'#type' => 'textfield',
	    '#title' => t('Abbreviation'),
	    '#size' => 40,
	    '#maxlength' => 50,
	    '#default_value' => $institution['abbrev'],
	    //'#required' => TRUE,
	);
	$form['institution']['belonging'] = array(
		'#type' => 'select',
		'#multiple' => TRUE,
	    '#title' => t('Belongs to'),
	    '#options' => forschungsatlas_admin_get_belonging_names($iid),
	    '#default_value' => forschungsatlas_admin_get_belonging_ids($iid),
	    '#chosen' => TRUE,
	);
	$form['institution']['address'] = array(
	    '#type' => 'fieldset',
	    '#title' => t('Address'),
	);
	$form['institution']['address']['street'] = array(
		'#type' => 'textfield',
	    '#title' => t('Street'),
	    '#size' => 60,
	    '#default_value' => $institution['street'],
	    '#description' => t('Required field for geolocation. Street address only, e.g. "Invalidenstr. 43".'),
	    '#required' => TRUE,
	);
	$form['institution']['address']['street2'] = array(
		'#type' => 'textfield',
	    '#size' => 60,
	    '#default_value' => $institution['street2'],
	    '#description' => t('Optional field. E.g. "Campus II - Gebäude H", "c/o T.rex", etc.'),
	    '#required' => FALSE,
	);
	$form['institution']['address']['postalcode'] = array(
		'#type' => 'textfield',
	    '#title' => t('Postal Code'),
	    '#size' => 20,
	    '#default_value' => $institution['postalcode'],
	    '#required' => TRUE,
	);
	$form['institution']['address']['cid'] = array(
		'#type' => 'select',
	    '#title' => t('City'),
	    '#options' => forschungsatlas_get_cities(),
	    '#empty_option' => t('- Please select -'),
	    '#default_value' => $cid,
	    '#required' => TRUE,
	    '#chosen' => TRUE,
	    '#element_validate' => array('element_validate_integer'),
	);
	$form['institution']['address']['fsid'] = array(
		'#type' => 'select',
	    '#title' => t('Federal State'),
	    '#options' => forschungsatlas_get_federal_states(),
	    '#default_value' => $fsid,
	    '#empty_option' => t('- Please select -'),
	    '#required' => TRUE,
	    '#chosen' => TRUE,
	    '#element_validate' => array('element_validate_integer'),
	);

	$form['institution']['url'] = array(
		'#type' => 'textfield',
	    '#title' => t('URL'),
	    '#size' => 60,
        '#maxlength' => 500,
	    '#default_value' => $institution['url'],
	);
	$form['institution']['otid'] = array(
		'#type' => 'select',
	    '#title' => t('Organization Type'),
	    '#options' => forschungsatlas_get_organization_types(),
	    '#empty_option' => t('- Please select -'),
	    '#default_value' => $otid,
	    '#required' => TRUE,
	    '#chosen' => TRUE,
	    '#element_validate' => array('element_validate_integer'),
	);
	$form['institution']['description'] = array(
	    '#type' => 'textarea',
	    '#title' => t('Comment'),
	    '#size' => 20,
	    '#default_value' => $institution['description'],
	    '#rows' => 2,
	    '#cols' => 20,
	);
	// Categories
	$form['categories'] = array(
		'#type' => 'fieldset',
		'#title' => t('Categories')
	);
	$form['categories']['network'] = array(
		'#type' => 'select',
		'#multiple' => TRUE,
	    '#title' => t('Research Network'),
	    '#options' => _forschungsatlas_admin_getNetworkNames(),
	    '#default_value' => _forschungsatlas_admin_getNetworkIds($iid),
	    '#chosen' => TRUE,
	);
	$form['categories']['organisms'] = array(
		'#type' => 'select',
		'#multiple' => TRUE,
		'#title' => t('Organisms'),
		'#options' => _forschungsatlas_admin_getOrganismNames(),
		'#default_value' => _forschungsatlas_admin_getOrganismIds($iid),
	    '#chosen' => TRUE,
	);
	$form['categories']['habitats'] = array(
		'#type' => 'select',
		'#multiple' => TRUE,
		'#title' => t('Habitats'),
		'#options' => _forschungsatlas_admin_getHabitatNames(),
		'#default_value' => _forschungsatlas_admin_getHabitatIds($iid),
	    '#chosen' => TRUE,
	);
	$form['categories']['topics'] = array(
		'#type' => 'select',
		'#multiple' => TRUE,
		'#title' => t('Research Disciplines'),
		'#options' => _forschungsatlas_admin_getTopicNames(),
		'#default_value' => _forschungsatlas_admin_getTopicIds($iid),
	    '#chosen' => TRUE,
	);
	$form['categories']['intsubjects'] = array(
		'#type' => 'select',
		'#multiple' => TRUE,
		'#title' => t('Interdisciplinary Subjects'),
		'#options' => _forschungsatlas_admin_getIntSubjectNames(),
		'#default_value' => _forschungsatlas_admin_getIntSubjectIds($iid),
	    '#chosen' => TRUE,
	);
	$form['categories']['methodologies'] = array(
		'#type' => 'select',
		'#multiple' => TRUE,
		'#title' => t('Methodologies'),
		'#options' => _forschungsatlas_admin_getMethodologyNames(),
		'#default_value' => _forschungsatlas_admin_getMethodologyIds($iid),
	    '#chosen' => TRUE,
	);
	// Publishing options
	$form['options'] = array(
	    '#type' => 'fieldset',
	    '#title' => t('Publishing options')
	);
	$form['options']['status'] = array(
	    '#type' => 'checkbox',
	    '#title' => t('Published'),
	    '#default_value' => $institution['status'],
	    '#element_validate' => array('element_validate_integer'),
	);
	$form['iid'] = array(
	    '#type' => 'hidden',
	    '#default_value' => $iid,
	);
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Submit'),
	);

	return $form;
}

/**
 * Page callback: Returns a form to edit an institution.
 *
 * @param $institution
 *   An array containing the institution.
 *
 * @return
 *   A form to edit the institution.
 *
 * @see forschungsatlas_menu()
 */
function forschungsatlas_admin_institution_edit($institution = array()) {
  drupal_set_title($institution['name']);

  return drupal_get_form('forschungsatlas_admin_institution_form', $institution);
}

/**
 * Validate function for forschungsatlas_admin_institution_form().
 *
 * @see forschungsatlas_admin_institution_form()
 * @see forschungsatlas_admin_institution_form_submit()
 */
function forschungsatlas_admin_institution_form_validate($form, &$form_state) {
  // Validate City
  $result = forschungsatlas_get_city_name($form_state['values']['cid']);
  if (!$result) {
    watchdog('security', 'Possible SQL injection attempt! An illegal choice has been detected: CID.', array(), WATCHDOG_ALERT);
    form_set_error('cid', t('The server was unable to process the request due to an internal error.'));
  }
  $form_state['NEFO']['cityname'] = $result;
  // Validate Federal State
  $result = forschungsatlas_get_federal_state_values($form_state['values']['fsid']);
  if (!$result) {
    watchdog('security', 'Possible SQL injection attempt! An illegal choice has been detected: FSID.', array(), WATCHDOG_ALERT);
    form_set_error('fsid', t('The server was unable to process the request due to an internal error.'));
  }
  $form_state['NEFO']['fsname'] = $result['name'];
  // Validate Postal Code
  if (!forschungsatlas_validate_postalcode($form_state['values']['postalcode'])) {
    form_set_error('postalcode' , t('The postal code must be a 5-digit German zip code.'));
  }
  // Validate URL
  if (!empty($form_state['values']['url']) && !forschungsatlas_validate_url($form_state['values']['url'])) {
    form_set_error('url' , t('The URL is not valid.'));
  }
  // Validate Organization Type
  if (!forschungsatlas_get_orgtype_name($form_state['values']['otid'])) {
    watchdog('security', 'Possible SQL injection attempt! An illegal choice has been detected: OTID.', array(), WATCHDOG_ALERT);
    form_set_error('otid', t('The server was unable to process the request due to an internal error.'));
  }
}

/**
 * Validate function for forschungsatlas_admin_institution_form().
 *
 * @see forschungsatlas_admin_institution_form()
 * @see forschungsatlas_admin_institution_form_validate()
 */
function forschungsatlas_admin_institution_form_submit($form, &$form_state) {
  global $user;

  $values = $form_state['values'];
  $insert = empty($values['iid']);
  $institution = new stdClass();
  $institution->iid = $insert ? '' : $values['iid'];
  $institution->name = forschungsatlas_clean_name($values['name']);
  $institution->abbrev = forschungsatlas_clean_name($values['abbrev']);
  $institution->street = forschungsatlas_clean_name($values['street']);
  $institution->street2 = forschungsatlas_clean_name($values['street2']);
  $institution->postalcode = forschungsatlas_clean_name($values['postalcode']);
  $institution->cid = $values['cid'];
  $institution->fsid = $values['fsid'];
  $institution->url = forschungsatlas_clean_name($values['url']);
  $institution->otid = $values['otid'];
  $institution->description = $values['description'];
  $institution->uid = $user->uid;
  $institution->status = $values['status'] ? FORSCHUNGSATLAS_PUBLISHED : FORSCHUNGSATLAS_NOT_PUBLISHED;
  $institution->changed = REQUEST_TIME;
  $mapping = new stdClass();
  $mapping->cityname = $form_state['NEFO']['cityname'];
  $mapping->fsname = $form_state['NEFO']['fsname'];
  $mapping->belonging = $form_state['values']['belonging'];
  // Categories
  $mapping->network = $form_state['values']['network'];
  $mapping->organisms	= $form_state['values']['organisms'];
  $mapping->habitats = $form_state['values']['habitats'];
  $mapping->topics = $form_state['values']['topics'];
  $mapping->intsubjects = $form_state['values']['intsubjects'];
  $mapping->methodologies	= $form_state['values']['methodologies'];
  // Save the institution
  forschungsatlas_save_institution($institution, $mapping);

  if ($insert) {
    $output = t('The institution <em>@name</em> has been created.', array('@name' => $institution->name));
    watchdog('NEFO', $output);
    drupal_set_message($output);
  }
  else {
    $output = t('The institution <em>@name</em> has been updated.', array('@name' => $institution->name));
    watchdog('NEFO', $output);
    drupal_set_message($output);
  }
  if ($institution->iid) {
    $form_state['values']['iid'] = $institution->iid;
    $form_state['iid'] = $institution->iid;
    $form_state['redirect'] = FORSCHUNGSATLAS_PUBLIC_PATH .'/institutions/'. $institution->iid ;
  }
  else {
    drupal_set_message(t('The institution could not be saved.'), 'error');
    $form_state['rebuild'] = TRUE;
  }
}

/**
 * Form constructor for the institution deletion confirmation form.
 *
 * @see forschungsatlas_admin_institution_delete_confirm_submit()
 */
function forschungsatlas_admin_institution_delete_confirm($form, &$form_state, $institution) {
  $form_state['NEFO']['iid'] = $institution['iid'];
  $form_state['NEFO']['name'] = $institution['name'];
  // Always provide entity id in the same form key as in the entity edit form.
  $form['iid'] = array('#type' => 'value', '#value' => $institution['iid']);
  return confirm_form($form,
    t('Are you sure you want to delete this institution: <em>@name</em>?', array('@name' => $institution['name'])),
    FORSCHUNGSATLAS_PUBLIC_PATH .'/institutions/'. $institution['iid'],
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Execute institution deletion.
 *
 * @see forschungsatlas_admin_institution_delete_confirm()
 */
function forschungsatlas_admin_institution_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $iid = intval($form_state['NEFO']['iid']);
    forschungsatlas_delete_institution($iid);
    cache_clear_all();
    $output = t('The institution <em>@name</em> has been deleted.', array('@name' => $form_state['NEFO']['name']));
    watchdog('NEFO', $output);
    drupal_set_message($output);
  }

  $form_state['redirect'] = '<front>';
}

/**
 * Build a selectable options for a form element.
 *
 * @param integer $iid
 *  The IID to be excluded from the list.
 *
 * @return
 *  An array in the form array(iid => name).
 */
function forschungsatlas_admin_get_belonging_names($iid) {
  $names = array();
  $result = db_query('SELECT i.iid AS iid, FORSCHUNGSATLAS__GETCOMPOSITEDNAME(i.iid, l.parent) AS assembled_name
      FROM {forschungsatlas__institutions} AS i
      LEFT JOIN {forschungsatlas__institution_institution} AS l ON i.iid = l.iid
      WHERE i.iid!=:iid
      GROUP BY i.iid ORDER BY assembled_name', array(':iid' => $iid));

  foreach ($result as $row) {
    $names[$row->iid] = $row->assembled_name;
  }

  return $names;
}

/**
 * Build an array of mapped IIDs to the given institution ID.
 *
 * @param integer $iid
 *  The institution ID
 *
 * @return
 *  An array containing the associated IIDs.
 */
function forschungsatlas_admin_get_belonging_ids($iid) {
	return forschungsatlas_get_mapped_ids($iid, 'forschungsatlas__institution_institution', 'parent');
}

// Categories

/**
 * Build a selectable options for a form element.
 */
function _forschungsatlas_admin_getNetworkNames() {
  return _forschungsatlas_admin_build_composited_options('forschungsatlas__research_network', 'rnid');
}

/**
 * Build default options for a selectable form element.
 */
function _forschungsatlas_admin_getNetworkIds($iid) {
	return forschungsatlas_get_mapped_ids($iid, 'forschungsatlas__institution_researchnetwork', 'rnid');
}

/**
 * Build a selectable options for a form element.
 */
function _forschungsatlas_admin_getOrganismNames() {
  return _forschungsatlas_admin_build_composited_options('forschungsatlas__organisms', 'oid');
}

/**
 * Build default options for a selectable form element.
 */
function _forschungsatlas_admin_getOrganismIds($iid) {
	return forschungsatlas_get_mapped_ids($iid, 'forschungsatlas__institution_organism', 'oid');
}

/**
 * Build a selectable options for a form element.
 */
function _forschungsatlas_admin_getHabitatNames() {
  return _forschungsatlas_admin_build_composited_options('forschungsatlas__habitats', 'hid');
}

/**
 * Build default options for a selectable form element.
 */
function _forschungsatlas_admin_getHabitatIds($iid) {
	return forschungsatlas_get_mapped_ids($iid, 'forschungsatlas__institution_habitat', 'hid');
}

/**
 * Build a selectable options for a form element.
 */
function _forschungsatlas_admin_getTopicNames() {
  return _forschungsatlas_admin_build_composited_options('forschungsatlas__topics', 'tid');
}

/**
 * Build default options for a selectable form element.
 */
function _forschungsatlas_admin_getTopicIds($iid) {
	return forschungsatlas_get_mapped_ids($iid, 'forschungsatlas__institution_topic', 'tid');
}

/**
 * Build a selectable options for a form element.
 */
function _forschungsatlas_admin_getIntSubjectNames() {
	return _forschungsatlas_admin_build_composited_options('forschungsatlas__interdisciplinary_subject', 'isid');
}

/**
 * Build default options for a selectable form element.
 */
function _forschungsatlas_admin_getIntSubjectIds($iid) {
	return forschungsatlas_get_mapped_ids($iid, 'forschungsatlas__institution_intersubject', 'isid');
}

/**
 * Build a selectable options for a form element.
 */
function _forschungsatlas_admin_getMethodologyNames() {
  return _forschungsatlas_admin_build_composited_options('forschungsatlas__methodology', 'mid');
}

/**
 * Build default options for a selectable form element.
 */
function _forschungsatlas_admin_getMethodologyIds($iid) {
	return forschungsatlas_get_mapped_ids($iid, 'forschungsatlas__institution_methodology', 'mid');
}

/**
 * Build a selectable options for a form element.
 *
 * @param string $table
 *  Table name
 * @param string $key
 *  Key name
 *
 * @return
 *  An array in the form array(CompositedKey => FormattedName).
 */
function _forschungsatlas_admin_build_composited_options($table, $key) {
  $options = array();
  $comp_options = array();

  $result = db_query('SELECT t1.'. $key .' AS keyid, t1.name AS name, t2.name AS parentname
      FROM {'. $table .'} AS t1 LEFT JOIN {'. $table .'} AS t2 ON t1.parent = t2.'. $key);
  foreach ($result as $row) {
    if (empty($row->parentname)) {
      $name = $row->name;
      $compositedname = $row->name;
    }
    else {
      $name = '- '. $row->name;
      $compositedname = $row->parentname .' > '. $row->name;
    }
    $comp_options[$compositedname] = array('keyid' => $row->keyid, 'name' => $name);
  } // foreach()
  ksort($comp_options);

  foreach ($comp_options as $row) {
    $options[$row['keyid']] = $row['name'];
  }

  return $options;
}


