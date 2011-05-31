<?php
function swift_mailer_page_admin_config_swift_mailer_messages() {
    return drupal_get_form('swift_mailer_page_admin_config_swift_mailer_form_messages');
}

/**
 * Form builder; the messages form.
 *
 * @see swift_mailer_page_admin_config_swift_mailer_form_messages_submit()
 */
function swift_mailer_page_admin_config_swift_mailer_form_messages($form, &$form_state) {

    $form['#tree'] = true;

    $form['description'] = array(
        '#markup' => t('<p>This page allows you to configure settings which applies to e-mail messages.</p>'),
        );

    if (swift_mailer_validate_library(variable_get(SWIFT_MAILER_VARIABLE_PATH, SWIFT_MAILER_VARIABLE_PATH_DEFAULT))) {

        $form['format'] = array(
            '#type' => 'fieldset',
            '#title' => t('Message format'),
            '#description' => t('You can set the default message format which should be applied to e-mail
                                 messages.'),
            );

        $form['format']['type'] = array(
            '#type' => 'radios',
            '#options' => array(SWIFT_MAILER_FORMAT_PLAIN => t('Plain Text'), SWIFT_MAILER_FORMAT_HTML => t('HTML')),
            '#default_value' => variable_get(SWIFT_MAILER_VARIABLE_FORMAT, SWIFT_MAILER_VARIABLE_FORMAT_DEFAULT),
            );

        $form['character_set'] = array(
            '#type' => 'fieldset',
            '#title' => t('Character Set'),
            '#description' => t('<p>E-mails need to carry details about the character set which the
                                 receiving client should use to understand the content of the e-mail.
                                 The default character set is UTF-8.</p>'),
            );

        $form['character_set']['type'] = array(
            '#type' => 'select',
            '#options' => swift_mailer_get_character_set_options(),
            '#default_value' => variable_get(SWIFT_MAILER_VARIABLE_CHARACTER_SET, SWIFT_MAILER_VARIABLE_CHARACTER_SET_DEFAULT),
            );

        $form['submit'] = array(
            '#type' => 'submit',
            '#value' => t('Save'),
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

function swift_mailer_page_admin_config_swift_mailer_form_messages_submit($form, &$form_state) {

    if (isset($form_state['values']['format']['type'])) {
        variable_set(SWIFT_MAILER_VARIABLE_FORMAT, $form_state['values']['format']['type']);
    }

    if (isset($form_state['values']['character_set']['type'])) {
        variable_set(SWIFT_MAILER_VARIABLE_CHARACTER_SET, $form_state['values']['character_set']['type']);
    }

}