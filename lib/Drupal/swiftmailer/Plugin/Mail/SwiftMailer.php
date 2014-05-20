<?php

/**
 * @file
 * Contains \Drupal\swiftmailer\Plugin\Mail\SwiftMailer.
 */

namespace Drupal\swiftmailer\Plugin\Mail;

use Drupal\Core\Mail\MailInterface;
use Exception;
use Swift_Attachment;
use Swift_FileSpool;
use Swift_Mailer;
use Swift_MailTransport;
use Swift_Message;
use Drupal\swiftmailer\Utility\Conversion;
use Swift_SendmailTransport;
use Swift_SmtpTransport;
use Swift_SpoolTransport;

/**
 * Provides a 'Swift Mailer' plugin to send emails.
 *
 * @Mail(
 *   id = "swiftmailer",
 *   label = @Translation("Swift Mailer"),
 *   description = @Translation("Swift Mailer Plugin.")
 * )
 */
class SwiftMailer implements MailInterface {

  protected $config;

  function __construct() {
    $this->config['transport'] = \Drupal::config('swiftmailer.transport')->getRawData();
    $this->config['message'] = \Drupal::config('swiftmailer.message')->getRawData();
  }

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
      $line_endings = $this->config['message']['mail_line_endings'];
      $message['body'] = implode($line_endings, $message['body']);

      // Get applicable format.
      $applicable_format = $this->getApplicableFormat($message);

      // Theme message if format is set to be HTML.
      if ($applicable_format == SWIFTMAILER_FORMAT_HTML) {
        if (isset($message['params']['theme'])) {
          $message['body'] = _theme($message['params']['theme'], $message);
        }
        else {
          $message['body'] = _theme('swiftmailer', $message);
        }

        if ($this->config['message']['convert_mode'] || !empty($message['params']['convert'])) {
          $converter = new html2text($message['body']);
          $message['plain'] = $converter->get_text();
        }
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
    // Validate whether the Swift Mailer module has been configured.
    /*
    $library_path = variable_get('swiftmailer_path', SWIFTMAILER_VARIABLE_PATH_DEFAULT);
    if (empty($library_path)) {
      watchdog('swiftmailer', 'An attempt to send an e-mail failed. The Swift Mailer library could not be found by the Swift Mailer module.', array(), WATCHDOG_ERROR);
      drupal_set_message(t('An attempt to send the e-mail failed. The e-mail has not been sent.'), 'error');
      return;
    }

    // Include the Swift Mailer library.
    require_once(DRUPAL_ROOT . '/' . $library_path . '/lib/swift_required.php');
    */

    try {

      // Create a new message.
      $m = Swift_Message::newInstance();

      // Not all Drupal headers should be added to the e-mail message.
      // Some headers must be supressed in order for Swift Mailer to
      // do its work properly.
      $suppressable_headers = swiftmailer_get_supressable_headers();

      // Keep track of whether we need to respect the provided e-mail
      // format or not
      $respect_format = $this->config['message']['respect_format'];

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

          // Skip 'Content-Type' header if the message to be sent will be a
          // multipart message or the provided format is not to be respected.
          if ($header_key == 'Content-Type' && (!$respect_format || swiftmailer_is_multipart($message))) {
            continue;
          }

          // Get header type.
          $header_type = Conversion::swiftmailer_get_headertype($header_key, $header_value);

          // Add the current header to the e-mail message.
          switch ($header_type) {
            case SWIFTMAILER_HEADER_ID:
              Conversion::swiftmailer_add_id_header($m, $header_key, $header_value);
              break;

            case SWIFTMAILER_HEADER_PATH:
              Conversion::swiftmailer_add_path_header($m, $header_key, $header_value);
              break;

            case SWIFTMAILER_HEADER_MAILBOX:
              Conversion::swiftmailer_add_mailbox_header($m, $header_key, $header_value);
              break;

            case SWIFTMAILER_HEADER_DATE:
              Conversion::swiftmailer_add_date_header($m, $header_key, $header_value);
              break;

            case SWIFTMAILER_HEADER_PARAMETERIZED:
              Conversion::swiftmailer_add_parameterized_header($m, $header_key, $header_value);
              break;

            default:
              Conversion::swiftmailer_add_text_header($m, $header_key, $header_value);
              break;

          }
        }
      }

      // Set basic message details.
      Conversion::swiftmailer_remove_header($m, 'From');
      Conversion::swiftmailer_remove_header($m, 'To');
      Conversion::swiftmailer_remove_header($m, 'Subject');

      // Parse 'from' and 'to' mailboxes.
      $from = Conversion::swiftmailer_parse_mailboxes($message['from']);
      $to = Conversion::swiftmailer_parse_mailboxes($message['to']);

      // Set 'from', 'to' and 'subject' headers.
      $m->setFrom($from);
      $m->setTo($to);
      $m->setSubject($message['subject']);

      // Get applicable format.
      $applicable_format = $this->getApplicableFormat($message);

      // Get applicable character set.
      $applicable_charset = $this->getApplicableCharset($message);

      // Set body.
      $m->setBody($message['body'], $applicable_format, $applicable_charset);

      // Add alternative plain text version if format is HTML and plain text
      // version is available.
      if ($applicable_format == SWIFTMAILER_FORMAT_HTML && !empty($message['plain'])) {
        $m->addPart($message['plain'], SWIFTMAILER_FORMAT_PLAIN, $applicable_charset);
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
        $transport_type = $this->config['transport']['transport'];

        // Configure the mailer based on the configured transport type.
        switch ($transport_type) {
          case SWIFTMAILER_TRANSPORT_SMTP:
            // Get transport configuration.
            $host = $this->config['transport']['smtp_host'];
            $port = $this->config['transport']['smtp_port'];
            $encryption = $this->config['transport']['smtp_encryption'];
            $username = $this->config['transport']['smtp_username'];
            $password = $this->config['transport']['smtp_password'];

            // Instantiate transport.
            $transport = Swift_SmtpTransport::newInstance($host, $port);
            $transport->setLocalDomain('[127.0.0.1]');

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
            $path = $this->config['transport']['sendmail_path'];
            $mode = $this->config['transport']['sendmail_mode'];

            // Instantiate transport.
            $transport = Swift_SendmailTransport::newInstance($path . ' -' . $mode);
            $mailer = Swift_Mailer::newInstance($transport);
            break;

          case SWIFTMAILER_TRANSPORT_NATIVE:
            // Instantiate transport.
            $transport = Swift_MailTransport::newInstance();
            $mailer = Swift_Mailer::newInstance($transport);
            break;

          case SWIFTMAILER_TRANSPORT_SPOOL:
            // Instantiate transport.
            $spooldir = $this->config['transport']['spool_directory'];
            $spool = new Swift_FileSpool($spooldir);
            $transport = Swift_SpoolTransport::newInstance($spool);
            $mailer = Swift_Mailer::newInstance($transport);
            break;
        }
      }

      // Send the message.
      return $mailer->send($m);



    }
    catch (Exception $e) {

      $headers = !empty($m) ? $m->getHeaders() : '';
      $headers = !empty($headers) ? nl2br($headers->toString()) : 'No headers were found.';
      watchdog('swiftmailer',
        'An attempt to send an e-mail message failed, and the following error
        message was returned : @exception_message<br /><br />The e-mail carried
        the following headers:<br /><br />!headers',
        array('@exception_message' => $e->getMessage(), '!headers' => $headers),
        WATCHDOG_ERROR);
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
        if (valid_url($file->uri, TRUE)) {
          $content = file_get_contents($file->uri);
        }
        else {
          $content = file_get_contents(drupal_realpath($file->uri));
        }

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
        if (valid_url($image->uri, TRUE)) {
          $content = file_get_contents($image->uri);
        }
        else {
          $content = file_get_contents(drupal_realpath($image->uri));
        }

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

  /**
   * Returns the applicable format.
   *
   * @param array $message
   *   The message for which the applicable format is to be determined.
   *
   * @return string
   *   A string being the applicable format.
   *
   */
  private function getApplicableFormat($message) {

    // Get the configured default format.
    $default_format = $this->config['message']['format'];

    // Get whether the provided format is to be respected.
    $respect_format = $this->config['message']['respect_format'];

    // Check if a format has been provided particularly for this message. If
    // that is the case, then apply that format instead of the default format.
    $applicable_format = !empty($message['params']['format']) ? $message['params']['format'] : $default_format;

    // Check if the provided format is to be respected, and if a format has been
    // set through the header "Content-Type". If that is the case, the apply the
    // format provided. This will override any format which may have been set
    // through $message['params']['format'].
    if ($respect_format && !empty($message['headers']['Content-Type'])) {
      $format = $message['headers']['Content-Type'];
      $format = preg_match('/.*\;/U', $format, $matches);

      if ($format > 0) {
        $applicable_format = trim(substr($matches[0], 0, -1));
      } else {
        $applicable_format = $default_format;
      }

    }

    return $applicable_format;

  }

  /**
   * Returns the applicable charset.
   *
   * @param array $message
   *   The message for which the applicable charset is to be determined.
   *
   * @return string
   *   A string being the applicable charset.
   *
   */
  private function getApplicableCharset($message) {

    // Get the configured default format.
    $default_charset = $this->config['message']['character_set'];

    // Get whether the provided format is to be respected.
    $respect_charset = $this->config['message']['respect_format'];

    // Check if a format has been provided particularly for this message. If
    // that is the case, then apply that format instead of the default format.
    $applicable_charset = !empty($message['params']['charset']) ? $message['params']['charset'] : $default_charset;

    // Check if the provided format is to be respected, and if a format has been
    // set through the header "Content-Type". If that is the case, the apply the
    // format provided. This will override any format which may have been set
    // through $message['params']['format'].
    if ($respect_charset && !empty($message['headers']['Content-Type'])) {
      $format = $message['headers']['Content-Type'];
      $format = preg_match('/charset.*=.*\;/U', $format, $matches);

      if ($format > 0) {
        $applicable_charset = trim(substr($matches[0], 0, -1));
        $applicable_charset = preg_replace('/charset=/', '', $applicable_charset);
      } else {
        $applicable_charset = $default_charset;
      }

    }

    return $applicable_charset;

  }

}
