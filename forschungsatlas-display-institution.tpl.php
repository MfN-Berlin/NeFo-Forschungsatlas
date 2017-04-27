<?php
/**
 * @file
 * Default theme implementation to display an institution.
 */
?>

<div id="forschungsatlas-display-institution">
  <div id="forschungsatlas-institution">

<?php
// Abbreviation
if (!empty($variables['forschungsatlas_institution']->abbrev)) {
  print '<div id="forschungsatlas-institution-abbrev">
      <div class="forschungsatlas-institution-h">'. t('Abbreviation') .':</div>
      <div class="forschungsatlas-institution-c">'. $variables['forschungsatlas_institution']->abbrev .'</div>
  </div>';  //forschungsatlas-institution-abbrev
}
// Address
if (!empty($variables['forschungsatlas_institution']->address)) {
  print '<div id="forschungsatlas-institution-address">';
  print '<div class="forschungsatlas-institution-h">'. t('Address') .':</div>';
  print '<div class="forschungsatlas-institution-c">'.
      check_plain($variables['forschungsatlas_institution']->address['street']) .'<br />'.
      check_plain($variables['forschungsatlas_institution']->address['postalcode']) .' '.
      check_plain($variables['forschungsatlas_institution']->address['city']) .'<br />'.
      check_plain($variables['forschungsatlas_institution']->address['federalstate']) .
    '</div>'; //forschungsatlas-institution-c
  // URL
  if (!empty($variables['forschungsatlas_institution']->url)) {
    print '<div id="forschungsatlas-institution-url">'.
        '<div class="forschungsatlas-institution-h">'. t('URL') .':</div>'.
        l(
            check_url($variables['forschungsatlas_institution']->url),
            check_url($variables['forschungsatlas_institution']->url),
            array(
                'attributes' => array(
                    'target'=>'blank',
                    //'class' => 'forschungsatlas-institution-l-ext',
                )
            )
        ) .
    '</div>'; //forschungsatlas-institution-url
  }
  print '</div>';  //forschungsatlas-institution-address
}
// Organization Type
if (!empty($variables['forschungsatlas_institution']->orgtype)) {
  print '<div id="forschungsatlas-institution-orgtype">
      <div class="forschungsatlas-institution-h">'. t('Organization Type') .':</div>
      <div class="forschungsatlas-institution-c">'. $variables['forschungsatlas_institution']->orgtype .'</div>
  </div>';  //forschungsatlas-institution-orgtype
}
// Belonging
if (!empty($variables['forschungsatlas_institution']->belonging)) {
  print '<div id="forschungsatlas-institution-belonging">
      <div class="forschungsatlas-institution-h">'. t('Belongs to') .':</div>
      <ul class="forschungsatlas-institution-ul">';
  foreach ($variables['forschungsatlas_institution']->belonging AS $key => $value) {
    print '<li class="forschungsatlas-institution-li">'. $value .'</li>';
  }
  print '</ul>
  </div>';  //forschungsatlas-institution-belonging
}
// Subordinate institutions
if (!empty($variables['forschungsatlas_institution']->subdivision)) {
  print '<div id="forschungsatlas-institution-subdivision">
      <div class="forschungsatlas-institution-h">'. t('Subordinate institutions of') .
        ' <em>'. check_plain($variables['forschungsatlas_institution']->name) .'</em>:</div>';
  print _forschungsatlas_build_subordinate_list($variables['forschungsatlas_institution']->subdivision);
  print '</div>';  //forschungsatlas-institution-belonging
}


?>

  </div>

  <div id="forschungsatlas-categories">

<?php
// Research Network
if (!empty($variables['forschungsatlas_institution']->network)) {
  print '<div id="forschungsatlas-institution-network">
      <div class="forschungsatlas-institution-h">'. t('Research Network') .':</div>
      <div class="forschungsatlas-institution-c">
        <ul>';
  foreach ($variables['forschungsatlas_institution']->network AS $key => $value) {
    print '<li>'. check_plain($value) .'</li>';
  }
  print '</ul>
      </div>
  </div>'; //forschungsatlas-institution-network
}
// Organisms
if (!empty($variables['forschungsatlas_institution']->organisms)) {
  print '<div id="forschungsatlas-institution-organisms">
      <div class="forschungsatlas-institution-h">'. t('Organisms') .':</div>
      <div class="forschungsatlas-institution-c">
        <ul>';
  foreach ($variables['forschungsatlas_institution']->organisms AS $key => $value) {
    print '<li>'. check_plain($value) .'</li>';
  }
  print '</ul>
      </div>
  </div>'; //forschungsatlas-institution-organisms
}
// Habitats
if (!empty($variables['forschungsatlas_institution']->habitats)) {
  print '<div id="forschungsatlas-institution-habitats">
      <div class="forschungsatlas-institution-h">'. t('Habitats') .':</div>
      <div class="forschungsatlas-institution-c">
        <ul>';
  foreach ($variables['forschungsatlas_institution']->habitats AS $key => $value) {
    print '<li>'. check_plain($value) .'</li>';
  }
  print '</ul>
      </div>
  </div>'; //forschungsatlas-institution-habitats
}
// Research Topics
if (!empty($variables['forschungsatlas_institution']->topics)) {
  print '<div id="forschungsatlas-institution-topics">
      <div class="forschungsatlas-institution-h">'. t('Research Disciplines') .':</div>
      <div class="forschungsatlas-institution-c">
        <ul>';
  foreach ($variables['forschungsatlas_institution']->topics AS $key => $value) {
    print '<li>'. check_plain($value) .'</li>';
  }
  print '</ul>
      </div>
  </div>'; //forschungsatlas-institution-topics
}
// Interdisciplinary Subjects
if (!empty($variables['forschungsatlas_institution']->intsubjects)) {
  print '<div id="forschungsatlas-institution-intsubjects">
      <div class="forschungsatlas-institution-h">'. t('Interdisciplinary Subjects') .':</div>
      <div class="forschungsatlas-institution-c">
        <ul>';
  foreach ($variables['forschungsatlas_institution']->intsubjects AS $key => $value) {
    print '<li>'. check_plain($value) .'</li>';
  }
  print '</ul>
      </div>
  </div>'; //forschungsatlas-institution-intsubjects
}
// Methodologies
if (!empty($variables['forschungsatlas_institution']->methodologies)) {
  print '<div id="forschungsatlas-institution-methodologies">
      <div class="forschungsatlas-institution-h">'. t('Methodologies') .':</div>
      <div class="forschungsatlas-institution-c">
        <ul>';
  foreach ($variables['forschungsatlas_institution']->methodologies AS $key => $value) {
    print '<li>'. check_plain($value) .'</li>';
  }
  print '</ul>
      </div>
  </div>'; //forschungsatlas-institution-methodologies
}
?>
  </div>
  <div id="forschungsatlas-comment">

<?php
// Comment (description)
if (!empty($variables['forschungsatlas_institution']->description)) {
  print '<div id="forschungsatlas-institution-description">
      <div class="forschungsatlas-institution-h">'. t('Comment') .':</div>
      <div class="forschungsatlas-institution-c">'. check_plain($variables['forschungsatlas_institution']->description) .'</div>
  </div>';  //forschungsatlas-institution-description
}
?>

  </div>
  <div id="forschungsatlas-subdivisions">

<?php

/**
 * Build a HTML list of subdivisions.
 * @param $list
 *  Array of subdivisions.
 *
 * @return string $html_list
 *  The HTML code.
 */
function _forschungsatlas_build_subordinate_list($list) {
  $html_list = '';

  foreach ($list AS $value) {
    if (!is_array($value)) {
      if (!empty($value)) {
        $value_instiuttion = '';
        $value_url = '';
        list($iid, $name, $url) = explode(FORSCHUNGSATLAS_DELIMITER_ID, $value);

        $value_instiuttion = l(
          $name,
          FORSCHUNGSATLAS_PUBLIC_PATH . '/institutions/'. $iid,
          array(
            'attributes' => array(
              'class' => 'forschungsatlas-subdivision-l',
            )
          )
        );
        if (!empty($url)) {
            $value_url = check_url($url);
            $value_url = l(t('Link to website'), $value_url, array(
                'attributes' => array(
                    'target'=>'blank',
                    'class' => 'forschungsatlas-institution-l-ext',
                )
              )
            );
        } // if()
        $html_list .= '<li class="forschungsatlas-subdivision-li">'. $value_instiuttion .' '. $value_url;
      }
    }
    else {
      $html_list .= '<ul class="forschungsatlas-subdivision-ul">'. _forschungsatlas_build_subordinate_list($value) .'</li>';
      $html_list .= '</ul>';
    }
  } // foreach()

  return $html_list;
}


?>

  </div>
</div>

