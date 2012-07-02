<?php
/**
 * @file
 * The implementation of MailSystemInterface which delegates handling of e-mails
 * to the Swift Mailer library.
 */

class SWIFTMailSystem implements MailSystemInterface {

  /**
   * Formats a message composed by drupal_mail().
   *
   * @see http://api.drupal.org/api/drupal/includes--mail.inc/interface/MailSystemInterface/7
   *
   * @param array $message
   *   A message array holding all relevant details for the message.
   *
   * @return string
   *   The message as it should be sent.
   */
  public function format(array $message) {
    if (!empty($message) && is_array($message)) {

      // Get default mail line endings and merge all lines in the e-mail body
      // separated by the mail line endings.
      $line_endings = variable_get('mail_line_endings', MAIL_LINE_ENDINGS);
      $message['body'] = implode($line_endings, $message['body']);

      // Validate message format.
      if (empty($message['params']['format'])) {
        $message['params']['format'] = variable_get('swiftmailer_format', SWIFTMAILER_VARIABLE_FORMAT_DEFAULT);
      }

      // Validate message theming.
      if (!isset($message['params']['theme'])) {
        $message['params']['theme'] = ($message['params']['format'] == SWIFTMAILER_FORMAT_HTML) ? TRUE : FALSE;
      }

      // Theme message if theming is enabled and format is set to be HTML.
      if (!empty($message['params']['theme']) && $message['params']['format'] == SWIFTMAILER_FORMAT_HTML) {
        $message['body'] = theme('swiftmailer', $message);
      }

      // Process any images specified by 'image:' which are to be added later
      // in the process. All we do here is to alter the message so that image
      // paths are replaced with cid's. Each image gets added to the array
      // which keeps track of which images to embed in the e-mail.
      $embeddable_images = array();
      preg_match_all('/"image:([^"]+)"/', $message['body'], $embeddable_images);
      for ($i = 0; $i < count($embeddable_images[0]); $i++) {

        $image_id = $embeddable_images[0][$i];
        $image_path = trim($embeddable_images[1][$i]);
        $image_name = basename($image_path);

        if (drupal_substr($image_path, 0, 1) == '/') {
          $image_path = drupal_substr($image_path, 1);
        }

        $image = new stdClass();
        $image->uri = $image_path;
        $image->filename = $image_name;
        $image->filemime = file_get_mimetype($image_path);
        $image->cid = rand(0, 9999999999);
        $message['params']['images'][] = $image;
        $message['body'] = preg_replace($image_id, 'cid:' . $image->cid, $message['body']);
      }

      return $message;
    }
  }

  /**
   * Sends a message composed by drupal_mail().
   *
   * @see http://api.drupal.org/api/drupal/includes--mail.inc/interface/MailSystemInterface/7
   *
   * @param array $message
   *   A message array holding all relevant details for the message.
   *
   * @return boolean
   *   TRUE if the message was successfully sent, and otherwise FALSE.
   */
  public function mail(array $message) {

    // Include required files.
    require_once(dirname(dirname(__FILE__)) . '/helpers/conversion.inc');
    require_once(dirname(dirname(__FILE__)) . '/helpers/utilities.inc');

    // Validate whether the Swift Mailer module has been configured.
    $library_path = variable_get('swiftmailer_path', SWIFTMAILER_VARIABLE_PATH_DEFAULT);
    if (empty($library_path)) {
      watchdog('swiftmailer', 'An attempt to send an e-mail failed. The Swift Mailer library could not be found by the Swift Mailer module.', array(), WATCHDOG_ERROR);
      drupal_set_message(t('An attempt to send the e-mail failed. The e-mail has not been sent.'), 'error');
      return;
    }

    // Include the Swift Mailer library.
    require_once(drupal_realpath($library_path . '/lib/swift_required.php'));

    try {

      // Create a new message.
      $m = Swift_Message::newInstance();

      // Not all Drupal headers should be added to the e-mail message.
      // Some headers must be supressed in order for Swift Mailer to
      // do its work properly.
      $suppressable_headers = swiftmailer_get_supressable_headers();

      // Process headers provided by Drupal. We want to add all headers which
      // are provided by Drupal to be added to the message. For each header we
      // first have to find out what type of header it is, and then add it to
      // the message as the particular header type.
      if (!empty($message['headers']) && is_array($message['headers'])) {
        foreach ($message['headers'] as $header_key => $header_value) {

          // Check wether the current header key is empty or represents
          // a header that should be supressed. If yes, then skip header.
          if (empty($header_key) || in_array($header_key, $suppressable_headers)) {
            continue;
          }

          // Get header type.
          $header_type = swiftmailer_get_headertype($header_key, $header_value);

          // Add the current header to the e-mail message.
          switch ($header_type) {
            case SWIFTMAILER_HEADER_ID:
              swiftmailer_add_id_header($m, $header_key, $header_value);
              break;

            case SWIFTMAILER_HEADER_PATH:
              swiftmailer_add_path_header($m, $header_key, $header_value);
              break;

            case SWIFTMAILER_HEADER_MAILBOX:
              swiftmailer_add_mailbox_header($m, $header_key, $header_value);
              break;

            case SWIFTMAILER_HEADER_DATE:
              swiftmailer_add_date_header($m, $header_key, $header_value);
              break;

            case SWIFTMAILER_HEADER_PARAMETERIZED:
              swiftmailer_add_parameterized_header($m, $header_key, $header_value);
              break;

            default:
              swiftmailer_add_text_header($m, $header_key, $header_value);
              break;

          }
        }
      }

      // Set basic message details.
      swiftmailer_remove_header($m, 'From');
      swiftmailer_remove_header($m, 'To');
      swiftmailer_remove_header($m, 'Subject');

      // Parse 'from' and 'to' mailboxes.
      $from = swiftmailer_parse_mailboxes($message['from']);
      $to = swiftmailer_parse_mailboxes($message['to']);

      // Set 'from', 'to' and 'subject' headers.
      $m->setFrom($from);
      $m->setTo($to);
      $m->setSubject($message['subject']);

      // Add content to the message. If the provided format is to be respected,
      // and a format is provided, then use that format. Otherwise, use the
      // configured format unless a format is provided in 
      // $message['params']['format'].
      $default_format = variable_get('swiftmailer_format', SWIFTMAILER_VARIABLE_FORMAT_DEFAULT);
      $respect_format = variable_get('swiftmailer_respect_format', SWIFTMAILER_VARIABLE_RESPECT_FORMAT_DEFAULT);
      $applicable_format = !empty($message['params']['format']) ? $message['params']['format'] : $default_format;

      if ($respect_format && $m->getHeaders()->has('Content-Type')) {
        $format = $m->getHeaders()->get('Content-Type');
        $format = preg_match('/:.*\;/U', $format, $matches);
        $applicable_format = (($format > 0) && ($matches[0] > 2)) ? trim(substr($matches[0], 2)) : $default_format;
      }

      $m->setBody($message['body'], $applicable_format);
      
      // Apply character set to the message. The default character set will
      // be applied to the message unless a charset is provided in
      // $message['params']['charset'].
      if (empty($message['params']['charset'])) {
        $m->setCharset(variable_get('swiftmailer_character_set', SWIFTMAILER_VARIABLE_CHARACTER_SET_DEFAULT));
      }
      else {
        $m->setCharset($message['params']['charset']);
      }

      // Validate that $message['params']['files'] is an array.
      if (empty($message['params']['files']) || !is_array($message['params']['files'])) {
        $message['params']['files'] = array();
      }

      // Let other modules get the chance to add attachable files.
      $files = module_invoke_all('swiftmailer_attach', $message['key']);
      if (!empty($files) && is_array($files)) {
        $message['params']['files'] = array_merge(array_values($message['params']['files']), array_values($files));
      }

      // Attach files.
      if (!empty($message['params']['files']) && is_array($message['params']['files'])) {
        $this->attach($m, $message['params']['files']);
      }

      // Embed images.
      if (!empty($message['params']['images']) && is_array($message['params']['images'])) {
        $this->embed($m, $message['params']['images']);
      }

      static $mailer;

      // If required, create a mailer which will be used to send the message.
      if (empty($mailer)) {

        // Get the configured transport type.
        $transport_type = variable_get('swiftmailer_transport', SWIFTMAILER_VARIABLE_TRANSPORT_DEFAULT);

        // Configure the mailer based on the configured transport type.
        switch ($transport_type) {
          case SWIFTMAILER_TRANSPORT_SMTP:
            // Get transport configuration.
            $host = variable_get('swiftmailer_smtp_host', SWIFTMAILER_VARIABLE_SMTP_HOST_DEFAULT);
            $port = variable_get('swiftmailer_smtp_port', SWIFTMAILER_VARIABLE_SMTP_PORT_DEFAULT);
            $encryption = variable_get('swiftmailer_smtp_encryption', SWIFTMAILER_VARIABLE_SMTP_ENCRYPTION_DEFAULT);
            $username = variable_get('swiftmailer_smtp_username', SWIFTMAILER_VARIABLE_SMTP_USERNAME_DEFAULT);
            $password = variable_get('swiftmailer_smtp_password', SWIFTMAILER_VARIABLE_SMTP_PASSWORD_DEFAULT);

            // Instantiate transport.
            $transport = Swift_SmtpTransport::newInstance($host, $port);

            // Set encryption (if any).
            if (!empty($encryption)) {
              $transport->setEncryption($encryption);
            }

            // Set username (if any).
            if (!empty($username)) {
              $transport->setUsername($username);
            }

            // Set password (if any).
            if (!empty($password)) {
              $transport->setPassword($password);
            }

            $mailer = Swift_Mailer::newInstance($transport);
            break;

          case SWIFTMAILER_TRANSPORT_SENDMAIL:
            // Get transport configuration.
            $path = variable_get('swiftmailer_sendmail_path', SWIFTMAILER_VARIABLE_SENDMAIL_PATH_DEFAULT);
            $mode = variable_get('swiftmailer_sendmail_mode', SWIFTMAILER_VARIABLE_SENDMAIL_MODE_DEFAULT);

            // Instantiate transport.
            $transport = Swift_SendmailTransport::newInstance($path . ' -' . $mode);
            $mailer = Swift_Mailer::newInstance($transport);
            break;

          case SWIFTMAILER_TRANSPORT_NATIVE:
            // Instantiate transport.
            $transport = Swift_MailTransport::newInstance();
            $mailer = Swift_Mailer::newInstance($transport);
            break;
        }
      }

      // Send the message.
      return $mailer->send($m);

    }
    catch (Exception $e) {

      // Log error and notify user.
      watchdog('swiftmailer', 'An attempt to send an e-mail message failed: @exception_message', array('@exception_message' => (drupal_substr(trim($e->getMessage()), -1) == '.') ? trim($e->getMessage()) : trim($e->getMessage()) . '.'), WATCHDOG_ERROR);
      drupal_set_message(t('An attempt to send an e-mail message failed.'), 'error');

    }
  }

  /**
   * Process attachments.
   *
   * @param Swift_Message $m
   *   The message which attachments are to be added to.
   * @param array $files
   *   The files which are to be added as attachments to the provided message.
   */
  private function attach(Swift_Message $m, array $files) {

    // Iterate through each array element.
    foreach ($files as $file) {

      if ($file instanceof stdClass) {

        // Validate required fields.
        if (empty($file->uri) || empty($file->filename) || empty($file->filemime)) {
          continue;
        }

        // Get file data.
        $content = file_get_contents(drupal_realpath($file->uri));
        $filename = $file->filename;
        $filemime = $file->filemime;

        // Attach file.
        $m->attach(Swift_Attachment::newInstance($content, $filename, $filemime));
      }
    }

  }

  /**
   * Process inline images..
   *
   * @param Swift_Message $m
   *   The message which inline images are to be added to.
   * @param array $images
   *   The images which are to be added as inline images to the provided
   *   message.
   */
  private function embed(Swift_Message $m, array $images) {

    // Iterate through each array element.
    foreach ($images as $image) {

      if ($image instanceof stdClass) {

        // Validate required fields.
        if (empty($image->uri) || empty($image->filename) || empty($image->filemime) || empty($image->cid)) {
          continue;
        }

        // Keep track of the 'cid' assigned to the embedded image.
        $cid = NULL;

        // Get image data.
        $content = file_get_contents(drupal_realpath($image->uri));
        $filename = $image->filename;
        $filemime = $image->filemime;

        // Embed image.
        $cid = $m->embed(Swift_Image::newInstance($content, $filename, $filemime));

        // The provided 'cid' needs to be replaced with the 'cid' returned
        // by the Swift Mailer library.
        $body = $m->getBody();
        $body = preg_replace('/cid.*' . $image->cid . '/', $cid, $body);
        $m->setBody($body);
      }
    }
  }
}
