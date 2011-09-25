The default mail system in Drupal requires that PHP is configured to send e-mail.
If your Drupal installation resides on a server which for some reason is not
capable of sending e-mails, then this module might be worth your time.

This module installs the Swift Mailer library as the default mail system in Drupal.
The Swift Mailer library is capable of sending e-mails using SMTP, a local MTA such
as sendmail and the mail functionality offered by PHP. The module lets you configure
how e-mails should be sent.

The module also lets other module developers easily add attachments to e-mails.
The Swift Mailer library handles large attachments with low memory use, and adding
file attachments is easy and uncomplicated.

Last, the module also lets you decide whether e-mails should be sent as plain text
or as HTML by default.

1.0 Configuration

The module required you to download the Swift Mailer library separately to a
directory of your choice (relative to the Drupal installation). However, it is
advised that libraries are kept in the 'sites/all/libraries' directory. If you keep
the Swift Mailer library in 'sites/all/libraries/swiftmailer', then you need to
configure the module to look for the Swift Mailer library in that directory. The
location of the Swift Mailer library can be found on the page admin/config/swiftmailer.

After the module has been configured with the Swift Mailer library you are advised
to make sure that the Swift Mailer library sends e-mails using the right transport
option. You can choose between SMTP, sendmail (or any other locally installed MTA)
and PHP's mail() function. Please observe the various configuration options which
are available for each of the transport options.

You also have the chance to decide whether e-mail messages should be sent as plain
text or HTML by default. Furthermore, you can also set the character set which is
to be used as default. You are advised not to change any of these options if you
are uncertain about what they mean.

You can test whether the Swift Mailer library sends e-mail messages correctly by
going to 'admin/config/swiftmailer/test' when the module has been configured and
you are ready to put it into action.

2.0 Adding attachments

The module allows you to add attachments from other modules. You can also change
the message format (plain text or HTML) and character set when sending e-mail messages
from other modules.

The standard procedure when sending e-mails from Drupal is to invoke the function
drupal_mail(). Furthermore, you also need to implement the hook_mail() function
which will be responsible for generating the actual content of the e-mail message.

/**
 * Send an e-mail.
 */
function test() {

  // Override default e-mail message options.
  $p = array(
    'format' => 'text/html',
    'charset' => 'UTF-8'
    );

  // Add attachment.
  $p['attachments'][] = file_load(1);

  // Send e-mail.
  drupal_mail('modulename', 'key', 'test@test.com', language_default(), $p);
}

/**
 * Implementation of hook_mail().
 */
function modulename_mail($key, &$message, $params) {

  switch($key) {
    default:
      $text[] = t('<strong>Hi</strong>');
      $text[] = t('<p>This is an automatically generated test e-mail.</p>');

      $message['subject'] = t('Test');
      $message['body'] = $text;
      break;
    }

}

The above lines of code shows you how to override the default e-mail message options.
Furthermore, it also shows you how to add attachments to an e-mail. Attachments needs
to be represented by stdClass instances each representing an an attachment. The
stdClass instances needs to have the properties 'uri', 'filename' and 'filemime'.
Drupal's drupal_realpath() will be used to determine the actual location of the
location of the file.

It should be stressed that the Swift Mailer library only supports already existing
files. In other words, dynamically generated files which are to be added as attachment
to an e-mail needs to be generated and stored in a permanent or temporary location
before it is provided as an attachment. A recommended way to handle temporary files
is to utilise Drupals file system and mark the files as temporary. Drupal will then
take care of deleting those files after a set amount of time.