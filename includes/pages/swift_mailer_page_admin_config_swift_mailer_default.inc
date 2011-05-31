<?php
function swift_mailer_page_admin_config_swift_mailer_default() {
    return drupal_get_form('swift_mailer_page_admin_config_swift_mailer_form_default');
}

/**
 * Form builder; the default form.
 *
 * @see swift_mailer_page_admin_config_swift_mailer_form_default_validate()
 * @see swift_mailer_page_admin_config_swift_mailer_form_default_submit()
 */
function swift_mailer_page_admin_config_swift_mailer_form_default($form, &$form_state) {

    $form['#tree'] = true;

    $form['description'] = array(
        '#markup' => t('<p>The Swift Mailer module replaces the default mail system that is shipped
                        with Drupal and installs itself as the primary mail system. This allows
                        you to choose how e-mails should be sent. Furthermore, with the Swift
                        Mailer module you can easily add attachments to e-mails. To read more
                        about how this module works, please have a look at the !documentation.</p>'),
        );

    $form['library'] = array(
        '#type' => 'fieldset',
        '#title' => t('Library location'),
        '#description' => t('<p>The Swift Mailer library is required for this module 
                            to work. You are advised to keep your libraries in the
                            <em>sites/all/libraries</em> directory. The Swift Mailer
                            library can be downloaded from the !website.</p>',
                            array('!website' => l('Swift Mailer website', 'http://swiftmailer.org/'))),
        );

    $class = '';
    if (!swift_mailer_validate_library(variable_get(SWIFT_MAILER_VARIABLE_PATH, SWIFT_MAILER_VARIABLE_PATH_DEFAULT))) {
        $class = 'error';

        $form['library']['message'] = array(
            '#markup' => t('<p>The Swift Mailer library could not be found in the path provided below.</p>'),
            );
    }

    $form['library']['path'] = array(
        '#type' => 'textfield',
        '#title' => t('Library path'),
        '#description' => t('The path to the Swift Mailer directory (e.g. sites/all/libraries/swift_mailer)'),
        '#required' => true,
        '#default_value' => check_plain(variable_get(SWIFT_MAILER_VARIABLE_PATH, SWIFT_MAILER_VARIABLE_PATH_DEFAULT)),
        '#attributes' => array('class' => array($class)),
        );

    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Save'),
        );
    
    return $form;
}

/**
 * Form validation handler for swift_mailer_page_admin_config_swift_mailer_form_default().
 */
function swift_mailer_page_admin_config_swift_mailer_form_default_validate($form, &$form_state) {

    if (!swift_mailer_validate_library($form_state['values']['library']['path'])) {
        form_set_error('library][path', t('The provided path does not contain the Swift Mailer library.'));
    }

}

/**
 * Form submission handler for swift_mailer_page_admin_config_swift_mailer_form_default().
 */
function swift_mailer_page_admin_config_swift_mailer_form_default_submit($form, &$form_state) {

    if (isset($form_state['values']['library']['path'])) {
        variable_set(SWIFT_MAILER_VARIABLE_PATH, swift_mailer_validate_library($form_state['values']['library']['path']));
    }

}