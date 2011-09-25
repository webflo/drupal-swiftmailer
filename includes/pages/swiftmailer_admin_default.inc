<?php
/**
 * @file
 * An administration page which allows for configuration of the SwiftMailer
 * library location.
 */

function swiftmailer_admin_default_page() {
    
    // Load dependencies.
    module_load_include('inc', 'swiftmailer', '/includes/helpers/utilities');

    // Return form.
    return drupal_get_form('swiftmailer_admin_default_form');

}

/**
 * Form builder; the default form.
 *
 * @see swiftmailer_admin_default_form_validate()
 * @see swiftmailer_admin_default_form_submit()
 */
function swiftmailer_admin_default_form($form, &$form_state) {

    $form['#tree'] = TRUE;

    $form['description'] = array(
        '#markup' => t('<p>The SwiftMailer module replaces the default mail system that is shipped
          with Drupal and installs itself as the primary mail system. This allows
          you to choose how e-mails should be sent. Furthermore, with the Swift
          Mailer module you can easily add attachments to e-mails. To read more
          about how this module works, please have a look at the !documentation.</p>'),
        );

    $form['library'] = array(
        '#type' => 'fieldset',
        '#title' => t('Library location'),
        '#description' => t('<p>The SwiftMailer library is required for this module
          to work. You are advised to keep your libraries in the
          <em>sites/all/libraries</em> directory. The SwiftMailer
          library can be downloaded from the !website.</p>',
          array('!website' => l(t('SwiftMailer website'), 'http://swiftmailer.org/'))),
        );

    $class = '';
    if (!swiftmailer_validate_library(variable_get(SWIFTMAILER_VARIABLE_PATH, SWIFTMAILER_VARIABLE_PATH_DEFAULT))) {
        $class = 'error';

        $form['library']['message'] = array(
            '#markup' => t('<p>The SwiftMailer library could not be found in the path provided below.</p>'),
            );
    }

    $form['library']['path'] = array(
        '#type' => 'textfield',
        '#title' => t('Library path'),
        '#description' => t('The path to the SwiftMailer directory (e.g. sites/all/libraries/swiftmailer)'),
        '#required' => TRUE,
        '#default_value' => check_plain(variable_get(SWIFTMAILER_VARIABLE_PATH, SWIFTMAILER_VARIABLE_PATH_DEFAULT)),
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

    if (!swiftmailer_validate_library($form_state['values']['library']['path'])) {
        form_set_error('library][path', t('The provided path does not contain the SwiftMailer library.'));
    }

}

/**
 * Form submission handler for swiftmailer_admin_default_form().
 */
function swiftmailer_admin_default_form_submit($form, &$form_state) {

    if (isset($form_state['values']['library']['path'])) {
        variable_set(SWIFTMAILER_VARIABLE_PATH, swiftmailer_validate_library($form_state['values']['library']['path']));
    }

}