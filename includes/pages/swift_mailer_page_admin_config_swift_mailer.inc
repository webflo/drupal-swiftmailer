<?php
function swift_mailer_page_admin_config_swift_mailer() {
    return drupal_get_form('SWIFT_MAILER_page_admin_config_SWIFT_MAILER_form');
}

function swift_mailer_page_admin_config_swift_mailer_form($form, &$form_state) {

    $form['#tree'] = true;

    $form['transport'] = array(
        '#type' => 'fieldset',
        '#title' => t('Transport Types'),
        '#description' => t('You can configure how messages should be sent.'),
        );

    $form['transport']['type'] = array(
        '#type' => 'radios',
        '#options' => array(SWIFT_MAILER_TRANSPORT_SMTP => t('SMTP'), SWIFT_MAILER_TRANSPORT_SENDMAIL => t('Sendmail'), SWIFT_MAILER_TRANSPORT_NATIVE => t('PHP')),
        '#default_value' => variable_get(SWIFT_MAILER_VARIABLE_TRANSPORT, SWIFT_MAILER_TRANSPORT_NATIVE),
        );

    $form['transport'][SWIFT_MAILER_TRANSPORT_SMTP] = array(
        '#type' => 'item',
        '#title' => t('SMTP'),
        '#states' => array(
                'visible' => array(
                    'input[id="edit-transport-type-smtp"]' => array(
                        'checked' => TRUE,
                    ),
                ),
            ),
        );

    $form['transport'][SWIFT_MAILER_TRANSPORT_SMTP]['server'] = array(
        '#type' => 'textfield',
        '#title' => t('SMTP server'),
        '#description' => t('The hostname or IP address identifying the SMTP server.'),
        );

    $form['transport'][SWIFT_MAILER_TRANSPORT_SMTP]['port'] = array(
        '#type' => 'textfield',
        '#title' => t('Port'),
        '#description' => t('The port at which the SMTP server can be contacted.'),
        );

    $form['transport'][SWIFT_MAILER_TRANSPORT_SMTP]['encryption'] = array(
        '#type' => 'select',
        '#title' => t('Encryption'),
        '#options' => SWIFT_MAILER_get_encryption_options(),
        '#description' => t('The type of encryption which should be used (if any).'),
        );

    $form['transport'][SWIFT_MAILER_TRANSPORT_SMTP]['username'] = array(
        '#type' => 'textfield',
        '#title' => t('Username'),
        '#description' => t('A username required by the SMTP server (leave blank if not required).'),
        );

    $form['transport'][SWIFT_MAILER_TRANSPORT_SMTP]['password'] = array(
        '#type' => 'textfield',
        '#title' => t('Password'),
        '#description' => t('A password required by the SMTP server (leave blank if not required).'),
        );

    $form['transport'][SWIFT_MAILER_TRANSPORT_SENDMAIL] = array(
        '#type' => 'item',
        '#title' => t('Sendmail'),
        '#states' => array(
                'invisible' => array(
                    'input[value="' . SWIFT_MAILER_TRANSPORT_SENDMAIL . '"]' => array(
                        'checked' => FALSE,
                    ),
                ),
            ),
        );

    $form['transport'][SWIFT_MAILER_TRANSPORT_SENDMAIL]['server'] = array(
        '#type' => 'textfield',
        '#title' => t('Application Path'),
        '#description' => t('The absolute path to the Sendmail application.'),
        );

    $form['format'] = array(
        '#type' => 'fieldset',
        '#title' => t('Message Format'),
        '#description' => t('You can specify the message format which should apply for messages sent from this site.'),
        );

    $form['format']['type'] = array(
        '#type' => 'radios',
        '#options' => array(SWIFT_MAILER_FORMAT_PLAIN => t('Plain Text'), SWIFT_MAILER_FORMAT_HTML => t('HTML')),
        '#default_value' => variable_get(SWIFT_MAILER_VARIABLE_FORMAT, SWIFT_MAILER_FORMAT_PLAIN),
        );

    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => 'Save',
        );
    
    return $form;
}

function swift_mailer_page_admin_config_swift_mailer_form_submit($form, &$form_state) {

    variable_set(SWIFT_MAILER_VARIABLE_FORMAT, $form_state['values']['format']['type']);
}

function SWIFT_MAILER_get_encryption_options() {

    //Define a variable to hold the various encryption options.
    $encryption_options['none'] = t('No Encryption');

    //Get available encryption options on this system.
    $system_transports = stream_get_transports();

    //Validate wether this system supports SSL.
    if (in_array('ssl', $system_transports)) {
        $encryption_options['ssl'] = t('SSL');
    }

    //Validate wether this system supports TLS.
    if (in_array('tls', $system_transports)) {
        $encryption_options['tls'] = t('TLS');
    }

    //Return available options.
    return $encryption_options;

}
?>
