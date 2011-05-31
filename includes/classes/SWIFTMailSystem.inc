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
        $message['body'] = implode("\r\n", $message['body']);
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

        //Validate whether the Swift Mailer module has been configured.
        $library_path = variable_get(SWIFT_MAILER_VARIABLE_PATH, SWIFT_MAILER_VARIABLE_PATH_DEFAULT);
        if (empty($library_path)) {
            watchdog('swift_mailer', 'An attempt to send an e-mail failed. The Swift Mailer library could not be found by the Swift Mailer module.', array(), WATCHDOG_ERROR);
            drupal_set_message(t('An attempt to send the e-mail failed. The e-mail has not been sent.'), 'error');
            return;
        }

        //Include the Swift Mailer library.
        require_once(drupal_realpath($library_path . '/lib/swift_required.php'));

        try {

            // Create a new message.
            $m = Swift_Message::newInstance();

            // Not all Drupal headers should be added to the e-mail message.
            // Some headers must be supressed in order for Swift Mailer to
            // do its work properly.
            $supressable_headers = swift_mailer_get_supressable_headers();

            // Process headers provided by Drupal. We want to add all headers which
            // are provided by Drupal to be added to the message. For each header we
            // first have to find out what type of header it is, and then add it to
            // the message as the particular header type.
            foreach($message['headers'] as $header_key => $header_value) {
                $header_type = swift_mailer_get_headertype($header_key, $header_value);

                // Check wheter the current header key represents a header that
                // should be supressed.
                if (in_array($header_key, $supressable_headers)) {
                    continue;
                }

                // Add the current header to the e-mail message.
                switch($header_type) {
                    case SWIFT_MAILER_HEADER_ID:
                        swift_mailer_add_id_header($m, $header_key, $header_value);
                        break;
                    case SWIFT_MAILER_HEADER_PATH:
                        swift_mailer_add_path_header($m, $header_key, $header_value);
                        break;
                    case SWIFT_MAILER_HEADER_MAILBOX:
                        swift_mailer_add_mailbox_header($m, $header_key, $header_value);
                        break;
                    case SWIFT_MAILER_HEADER_DATE:
                        swift_mailer_add_date_header($m, $header_key, $header_value);
                        break;
                    case SWIFT_MAILER_HEADER_PARAMETERIZED:
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
            
            // Add content to the message. The default content type will be applied
            // to the message (via setBody()) unless a charset is provided in
            // $message['params']['charset'].
            $m->getHeaders()->removeAll('Content-Type');
            if (empty($message['params']['format'])) {
                $m->setBody($message['body'], variable_get(SWIFT_MAILER_VARIABLE_FORMAT, SWIFT_MAILER_VARIABLE_FORMAT_DEFAULT));
            } else {
                $m->setBody($message['body'], $message['params']['format']);
            }

            // Apply character set to the message. The default character set will
            // be applied to the message unless a charset is provided in
            // $message['params']['charset'].
            if (empty($message['params']['charset'])) {
                $m->setCharset(variable_get(SWIFT_MAILER_VARIABLE_CHARACTER_SET, SWIFT_MAILER_VARIABLE_CHARACTER_SET_DEFAULT));
            } else {
                $m->setCharset($message['params']['charset']);
            }

            // Process attachments. All attachments needs to be provided in the
            // array ['params']['attachments'] in the $message variable. The
            // array needs to hold stdClass instances each representing a file.
            // Each of the instances can simply be a file instance provided by
            // Drupal, or a file instance programatically generated.
            if (!empty($message['params']['attachments'])) {
                foreach($message['params']['attachments'] as $attachment) {
                    if ($attachment instanceof stdClass) {

                        //Validate attachment properties.
                        if (empty($attachment->uri) || empty($attachment->filename) || empty($attachment->filemime)) {
                            continue;
                        }

                        // Prepare attachment details.
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

                //Get the configured transport type.
                $transport_type = variable_get(SWIFT_MAILER_VARIABLE_TRANSPORT, SWIFT_MAILER_VARIABLE_TRANSPORT_DEFAULT);

                switch($transport_type) {
                    case SWIFT_MAILER_TRANSPORT_SMTP:

                        //Get transport configuration.
                        $host = variable_get(SWIFT_MAILER_VARIABLE_SMTP_HOST, SWIFT_MAILER_VARIABLE_SMTP_HOST_DEFAULT);
                        $port = variable_get(SWIFT_MAILER_VARIABLE_SMTP_PORT, SWIFT_MAILER_VARIABLE_SMTP_PORT_DEFAULT);
                        $encryption = variable_get(SWIFT_MAILER_VARIABLE_SMTP_ENCRYPTION, SWIFT_MAILER_VARIABLE_SMTP_ENCRYPTION_DEFAULT);
                        $username = variable_get(SWIFT_MAILER_VARIABLE_SMTP_USERNAME, SWIFT_MAILER_VARIABLE_SMTP_USERNAME_DEFAULT);
                        $password = variable_get(SWIFT_MAILER_VARIABLE_SMTP_PASSWORD, SWIFT_MAILER_VARIABLE_SMTP_PASSWORD_DEFAULT);

                        //Instantiate transport.
                        $transport = Swift_SmtpTransport::newInstance($host, $port);

                        //Set encryption (if any).
                        if (!empty($encryption)) {
                            $transport->setEncryption($encryption);
                        }

                        //Set username (if any).
                        if (!empty($username)) {
                            $transport->setUsername($username);
                        }

                        //Set password (if any).
                        if (!empty($password)) {
                            $transport->setPassword($password);
                        }

                        static::$mailer = Swift_Mailer::newInstance($transport);
                        break;

                    case SWIFT_MAILER_TRANSPORT_SENDMAIL:

                        //Get transport configuration.
                        $path = variable_get(SWIFT_MAILER_VARIABLE_SENDMAIL_PATH, SWIFT_MAILER_VARIABLE_SENDMAIL_PATH_DEFAULT);
                        $mode = variable_get(SWIFT_MAILER_VARIABLE_SENDMAIL_MODE, SWIFT_MAILER_VARIABLE_SENDMAIL_MODE_DEFAULT);

                        //Instantiate transport.
                        $transport = Swift_SendmailTransport::newInstance($path .' -'. $mode);
                        static::$mailer = Swift_Mailer::newInstance($transport);
                        break;

                    case SWIFT_MAILER_TRANSPORT_NATIVE:

                        //Instantiate transport.
                        $transport = Swift_MailTransport::newInstance();
                        static::$mailer = Swift_Mailer::newInstance($transport);
                        break;
                }
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