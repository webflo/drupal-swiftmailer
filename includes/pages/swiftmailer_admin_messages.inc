<?php
/**
 * @file
 * An administration page which allows for configuration of how messages are
 * to be formatted.
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

  $form['#tree'] = TRUE;

  $form['description'] = array(
    '#markup' => t('<p>This page allows you to configure settings which determines how e-mail messages are created.</p>'),
    );

  if (swiftmailer_validate_library(variable_get(SWIFTMAILER_VARIABLE_PATH, SWIFTMAILER_VARIABLE_PATH_DEFAULT))) {

    $form['format'] = array(
      '#type' => 'fieldset',
      '#title' => t('Message format'),
      '#description' => t('You can set the default message format which should be applied to e-mail
        messages.'),
      );

    $form['format']['type'] = array(
      '#type' => 'radios',
      '#options' => array(SWIFTMAILER_FORMAT_PLAIN => t('Plain Text'), SWIFTMAILER_FORMAT_HTML => t('HTML')),
      '#default_value' => variable_get(SWIFTMAILER_VARIABLE_FORMAT, SWIFTMAILER_VARIABLE_FORMAT_DEFAULT),
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
      '#options' => swiftmailer_get_character_set_options(),
      '#default_value' => variable_get(SWIFTMAILER_VARIABLE_CHARACTER_SET, SWIFTMAILER_VARIABLE_CHARACTER_SET_DEFAULT),
      );

    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
      );

  } else {

    $form['message'] = array(
      '#markup' => t('<p>You need to configure the location of the Swift Mailer library. Please visit the !page
        and configure the library to enable the configuration options on this page.</p>',
        array('!page' => l(t('library configuration page'), 'admin/config/swiftmailer'))),
      );

  }

  return $form;
}

function swiftmailer_admin_messages_form_submit($form, &$form_state) {

  if (isset($form_state['values']['format']['type'])) {
    variable_set(SWIFTMAILER_VARIABLE_FORMAT, $form_state['values']['format']['type']);
  }

  if (isset($form_state['values']['character_set']['type'])) {
    variable_set(SWIFTMAILER_VARIABLE_CHARACTER_SET, $form_state['values']['character_set']['type']);
  }

}