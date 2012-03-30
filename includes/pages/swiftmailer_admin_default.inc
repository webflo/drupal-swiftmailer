<?php
/**
 * @file
 * An administration page which allows for configuration of the Swift Mailer
 * library location.
 */

/**
 * Page builder.
 */
function swiftmailer_admin_default_page() {
  return drupal_get_form('swiftmailer_admin_default_form');
}

/**
 * Form builder; the default form.
 *
 * @see swiftmailer_admin_default_form_validate()
 * @see swiftmailer_admin_default_form_submit()
 */
function swiftmailer_admin_default_form($form, &$form_state) {

  // Include helper functions.
  module_load_include('inc', 'swiftmailer', '/includes/helpers/utilities');

  $form['#tree'] = TRUE;

  $form['description'] = array(
    '#markup' => '<p>' . t('The Swift Mailer module replaces the default mail system that is shipped
      with Drupal and installs itself as the primary mail system. This allows
      you to choose how e-mails should be sent. Furthermore, with the Swift
      Mailer module you can easily add attachments to e-mails. To read more
      about how this module works, please have a look at the !documentation.',
      array('!documentation' => l(t('Swift Mailer documentation'), 'http://swiftmailer.org/docs/introduction.html'))) . '</p>',
  );

  $form['library'] = array(
    '#type' => 'fieldset',
    '#title' => t('Library location'),
    '#description' => '<p>' . t('The Swift Mailer library is required for this module
      to work. You are advised to keep your libraries in the
      <em>sites/all/libraries</em> directory. The Swift Mailer
      library can be downloaded from the !website.',
      array('!website' => l(t('Swift Mailer website'), 'http://swiftmailer.org/'))) . '</p>',
  );

  $class = '';
  $path = variable_get(SWIFTMAILER_VARIABLE_PATH, SWIFTMAILER_VARIABLE_PATH_DEFAULT);

  if (!swiftmailer_validate_library($path)) {

    // Attempt to automatically locate the Swift Mailer library.
    $alternatives = file_scan_directory('sites/all/libraries', '/Swift.php/');

    if (empty($alternatives)) {

      $class = 'error';

      $form['library']['message'] = array(
        '#markup' => '<p>' . t('The Swift Mailer library could not be found in the path provided below.') . '</p>',
      );

    }
    else {

      $path = reset($alternatives)->uri;
      $path = preg_replace('/\/lib\/classes\/Swift.php/', '', $path);
      drupal_set_message(t("The SwiftMailer library was found in '!path'.", array('!path' => $path)), 'status');
      variable_set(SWIFTMAILER_VARIABLE_PATH, $path);

    }

  }

  $form['library']['path'] = array(
    '#type' => 'textfield',
    '#title' => t('Library path'),
    '#description' => t('The path to the Swift Mailer directory (e.g. sites/all/libraries/swiftmailer)'),
    '#required' => TRUE,
    '#default_value' => $path,
    '#attributes' => array('class' => array($class)),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Form validation handler for swiftmailer_admin_default_form().
 */
function swiftmailer_admin_default_form_validate($form, &$form_state) {

  // Include helper functions.
  module_load_include('inc', 'swiftmailer', '/includes/helpers/utilities');

  if (!swiftmailer_validate_library($form_state['values']['library']['path'])) {
    form_set_error('library][path', t('The provided path does not contain the Swift Mailer library.'));
  }

}

/**
 * Form submission handler for swiftmailer_admin_default_form().
 */
function swiftmailer_admin_default_form_submit($form, &$form_state) {

  // Include helper functions.
  module_load_include('inc', 'swiftmailer', '/includes/helpers/utilities');

  if (isset($form_state['values']['library']['path'])) {
    variable_set(SWIFTMAILER_VARIABLE_PATH, swiftmailer_validate_library($form_state['values']['library']['path']));
  }

}
