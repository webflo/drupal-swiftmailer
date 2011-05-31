<?php
function swift_mailer_page_admin_config_swift_mailer_test() {
    return drupal_get_form('swift_mailer_page_admin_config_swift_mailer_form_test');
}

function swift_mailer_page_admin_config_swift_mailer_form_test($form, &$form_state) {

    $form['#tree'] = true;

    $form['description'] = array(
        '#markup' => t('<p>This page allows you to send a test e-mail to a recipient of your choice.</p>'),
        );

    if (swift_mailer_validate_library(variable_get(SWIFT_MAILER_VARIABLE_PATH, SWIFT_MAILER_VARIABLE_PATH_DEFAULT))) {

        $form['test'] = array(
            '#type' => 'fieldset',
            '#title' => t('Recipient'),
            '#description' => t('<p>You can send a test e-mail to a recipient of your choice. The e-mail will be sent using the default values as provided by the Swift Mailer module or as configured by you.</p>'),
            );

        global $user;

        $form['test']['recipient'] = array(
            '#title' => t('E-mail'),
            '#type' => 'textfield',
            '#default_value' => $user->mail,
            );

        $form['submit'] = array(
            '#type' => 'submit',
            '#value' => 'Send',
            );

    } else {

        $form['message'] = array(
            '#markup' => t('<p>You need to configure the location of the Swift Mailer library. Please visit the !page
                            and configure the library to enable the configuration options on this page.</p>',
                           array('!page' => l('library configuration page', 'admin/config/swift_mailer'))),
            );

    }
    
    return $form;
}

function swift_mailer_page_admin_config_swift_mailer_form_test_submit($form, &$form_state) {

    if (isset($form_state['values']['test']['recipient'])) {
        drupal_mail('swift_mailer', 'test', $form_state['values']['test']['recipient'], language_default());
        drupal_set_message('An e-mail has been sent to '. $form_state['values']['test']['recipient']);        
    }

}