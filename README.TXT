The default mail system in Drupal requires that PHP is configured to send e-mai
l. If your Drupal installation resides on a server which for some reason is not
capable of sending e-mails, then this module might be worth your time.

This module installs the Swift Mailer library as the default mail system in Dru
pal. The Swift Mailer library is capable of sending e-mails using SMTP, a local
MTA such as sendmail and the mail functionality offered by PHP. The module lets
you configure how e-mails should be sent.

The module also lets other module developers easily add attachments to e-mails.
The Swift Mailer library handles large attachments with low memory use, and add
ing file attachments is easy and uncomplicated.

Last, the module also lets you decide whether e-mails should be sent as plain t
ext or as HTML by default.

1.0 Configuration

The module required you to download the Swift Mailer library separately to a di
rectory of your choice (relative to the Drupal installation). However, it is ad
vised that libraries are kept in the 'sites/all/libraries' directory. If you ke
ep the Swift Mailer library in 'sites/all/libraries/swift_mailer', then you nee
d to configure the module to look for the Swift Mailer library in that director
y. The location of the Swift Mailer library can be found on the page admin/conf
ig/swift_mailer.

After the module has been configured with the Swift Mailer library you are advi
sed to make sure that the Swift Mailer library sends e-mails using the right tr
ansport option. You can choose between SMTP, sendmail (or any other locally ins
talled MTA) and PHP's mail() function. Please observe the various configuration
options which are available for each of the transport options.

You also have the chance to decide whether e-mail messages should be sent as pl
ain text or HTML by default. Furthermore, you can also set the character set wh
ich is to be used as default. You are advised not to change any of these option
s if you are uncertain about what they mean.

You can test whether the Swift Mailer library sends e-mail messages correctly b
y going to 'admin/config/swift_mailer/test' when the module has been configured
and you are ready to put it into action.

2.0 Adding attachments

The module allows you to add attachments from other modules. You can also chang
e the message format (plain text or HTML) and character set when sending e-mail
messages from other modules.

The standard procedure when sending e-mails from Drupal is to invoke the functi
on drupal_mail(). Furthermore, you also need to implement the hook_mail() funct
ion which will be responsible for generating the actual content of the e-mail m
essage.

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

The above lines of code shows you how to override the default e-mail message op
tions. Furthermore, it also shows you how to add attachments to an e-mail. Atta
chments needs to be represented by stdClass instances each representing an an a
ttachment. The stdClass instances needs to have the properties 'uri', 'filename
' and 'filemime'. Drupal's drupal_realpath() will be used to determine the actu
al location of the location of the file.

It should be stressed that the Swift Mailer library only supports already exist
ing files. In other words, dynamically generated files which are to be added as
attachment to an e-mail needs to be generated and stored in a permanent or temp
orary location before it is provided as an attachment. A recommended way to han
dle temporary files is to utilise Drupals file system and mark the files as tem
porary. Drupal will then take care of deleting those files after a set amount o
f time.

