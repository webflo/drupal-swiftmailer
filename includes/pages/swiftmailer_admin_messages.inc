<?php
/**
 * @file
 * An administration page which allows for configuration of how messages are
 * to be formatted.
 */

/**
 * Page builder.
 */
function swiftmailer_admin_messages_page() {
  return drupal_get_form('swiftmailer_admin_messages_form');
}

/**
 * Form builder; the messages form.
 *
 * @see swiftmailer_admin_messages_form_submit()
 */
function swiftmailer_admin_messages_form($form, &$form_state) {

  // Include helper functions.
  require_once(dirname(dirname(__FILE__)) . '/helpers/utilities.inc');

  $form['#tree'] = TRUE;

  $form['description'] = array(
    '#markup' => '<p>' . t('This page allows you to configure settings which determines how e-mail messages are created.') . '</p>',
  );

  if (swiftmailer_validate_library(variable_get('swiftmailer_path', SWIFTMAILER_VARIABLE_PATH_DEFAULT))) {

    $form['format'] = array(
      '#type' => 'fieldset',
      '#title' => t('Message format'),
      '#description' => t('You can set the default message format which should be applied to e-mail
        messages.'),
    );

    $form['format']['type'] = array(
      '#type' => 'radios',
      '#options' => array(SWIFTMAILER_FORMAT_PLAIN => t('Plain Text'), SWIFTMAILER_FORMAT_HTML => t('HTML')),
      '#default_value' => variable_get('swiftmailer_format', SWIFTMAILER_VARIABLE_FORMAT_DEFAULT),
    );

    $form['format']['respect'] = array(
      '#type' => 'checkbox',
      '#title' => t('Respect provided e-mail format.'),
      '#default_value' => variable_get('swiftmailer_respect_format', SWIFTMAILER_VARIABLE_RESPECT_FORMAT_DEFAULT),
      '#description' => t('The e-mail format provided in the e-mail, if any, will be respected if you enable this setting.'),
    );

    $form['character_set'] = array(
      '#type' => 'fieldset',
      '#title' => t('Character Set'),
      '#description' => '<p>' . t('E-mails need to carry details about the character set which the
        receiving client should use to understand the content of the e-mail.
        The default character set is UTF-8.') . '</p>',
    );

    $form['character_set']['type'] = array(
      '#type' => 'select',
      '#options' => swiftmailer_get_character_set_options(),
      '#default_value' => variable_get('swiftmailer_character_set', SWIFTMAILER_VARIABLE_CHARACTER_SET_DEFAULT),
    );

    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
    );

  }
  else {

    $form['message'] = array(
      '#markup' => '<p>' . t('You need to configure the location of the Swift Mailer library. Please visit the !page
        and configure the library to enable the configuration options on this page.',
        array('!page' => l(t('library configuration page'), 'admin/config/people/swiftmailer'))) . '</p>',
    );

  }

  return $form;
}

/**
 * Submit handler for swiftmailer_admin_messages_form($form, &$form_state).
 */
function swiftmailer_admin_messages_form_submit($form, &$form_state) {

  if (isset($form_state['values']['format']['type'])) {
    variable_set('swiftmailer_format', $form_state['values']['format']['type']);
  }

  if (isset($form_state['values']['format']['respect'])) {
    variable_set('swiftmailer_respect_format', $form_state['values']['format']['respect']);
  }

  if (isset($form_state['values']['character_set']['type'])) {
    variable_set('swiftmailer_character_set', $form_state['values']['character_set']['type']);
  }

}
