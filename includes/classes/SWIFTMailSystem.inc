<?php

class SWIFTMailSystem implements MailSystemInterface {

    private static $mailer = null;

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
        $message['body'] = implode("", $message['body']);
        return $message;
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
     *  TRUE if the message was successfully sent, and otherwise FALSE.
     */
    public function mail(array $message) {

        // Include the swift_mailer library.
        require_once('sites/all/libraries/swift_mailer/lib/swift_required.php');

        try {

            dpm($message);

            // Create a new message.
            $m = Swift_Message::newInstance();

            // Process headers provided by Drupal. We want to add all headers which
            // are provided by Drupal to be added to the message. For each header we
            // first have to find out what type of header it is, and then add it to
            // the message as the particular header type.
            foreach($message['headers'] as $header_key => $header_value) {
                $header_type = swift_mailer_get_headertype($header_key, $header_value);

                switch($header_type) {
                    case swift_mailer_HEADER_ID:
                        swift_mailer_add_id_header($m, $header_key, $header_value);
                        break;
                    case swift_mailer_HEADER_PATH:
                        swift_mailer_add_path_header($m, $header_key, $header_value);
                        break;
                    case swift_mailer_HEADER_MAILBOX:
                        swift_mailer_add_mailbox_header($m, $header_key, $header_value);
                        break;
                    case swift_mailer_HEADER_DATE:
                        swift_mailer_add_date_header($m, $header_key, $header_value);
                        break;
                    case swift_mailer_HEADER_PARAMETERIZED:
                        swift_mailer_add_parametrized_header($m, $header_key, $header_value);
                        break;
                    default:
                        swift_mailer_add_text_header($m, $header_key, $header_value);
                        break;
                }
            }
            
            // Set basic message details.
            swift_mailer_remove_header($m, 'From');
            swift_mailer_remove_header($m, 'To');
            swift_mailer_remove_header($m, 'Subject');
            $m->setFrom($message['from']);
            $m->setTo($message['to']);
            $m->setSubject($message['subject']);

            // Add content to the message. If the message is to be sent as HTML,
            // then we also need to update the 'Content-Type' as Drupal by default
            // sets content type to 'text/plain'. Otherwise, just add the content
            // and go with the default Drupal header.
            if (variable_get(swift_mailer_VARIABLE_FORMAT) == swift_mailer_FORMAT_HTML) {
                $m->getHeaders()->removeAll('Content-Type');
                $m->setBody($message['body'], swift_mailer_FORMAT_HTML);
            } else {
                $m->setBody($message['body']);
            }

            // Process attachments. All attachments needs to be provided in the
            // array 'attachments' in the $message variable. The array needs to
            // hold instances of the stdObject each representing a file. Each of
            // the instances is simply a file object as provided by Drupal.
            if (!empty($message['attachments'])) {
                foreach($message['attachments'] as $attachment) {
                    if ($attacment instanceof stdClass) {

                        // Prepare attachment
                        $attachment_content = file_get_contents(drupal_realpath($attachment->uri));
                        $attachment_filename = $attachment->filename;
                        $attachment_filemime = $attachment->filemime;

                        // Create attachment
                        $m->attach(Swift_Attachment::newInstance($attachment_content, $attachment_filename, $attachment_filemime));
                    }
                }
            }

            // Create a mailer which we will use to send the message.
            if (empty(static::$mailer)) {

                // Create a transport.
                $transport = Swift_SendmailTransport::newInstance('/usr/sbin/sendmail -t');

                //$transport = Swift_MailTransport::newInstance();
                //$transport = Swift_SmtpTransport::newInstance('outlook.sfp-net.com', 587, 'ssl');
                //$transport->setUsername('webmaster@upright-music.com');
                //$transport->setPassword('@12345Ab');

                // Create a mailer for the message.
                static::$mailer = Swift_Mailer::newInstance($transport);
            }

            // Send the message.
            return static::$mailer->send($m);

        } catch (Exception $e) {

            // Log error and notify user.
            watchdog('swift_mailer', 'An attempt to send an e-mail message failed: ' . ((substr(trim($e->getMessage()), -1) == '.') ? trim($e->getMessage()) : trim($e->getMessage()) . '.'), array(), WATCHDOG_ERROR);
            drupal_set_message(t('An attempt to send an e-mail message failed.'), 'error');

        }
    }
}
?>
