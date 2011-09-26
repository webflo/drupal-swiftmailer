The default mail system in Drupal requires that PHP is configured to send e-mail.
However, some servers are not set up to handle e-mail, and in some cases it may
prove difficult to get e-mail support up and running. This module attempts to solve 
this problem by delegating all e-mail handling to the Swift Mailer library which 
is capable of sending e-mails directly through a SMTP server of your choice. The 
module installs the Swift Mailer library as the default mail system in Drupal.
Drupal can then be configured to send e-mails using an SMTP server of your choice,
a local MTA such as sendmail or the mail functionality offered by PHP. 

Furthermore, the module also lets module developers add attachments and inline 
images to e-mails.

1.0 Configuration

The module required you to download the Swift Mailer library separately to a
directory of your choice (relative to the Drupal installation). However, it is
advised that libraries are kept in the 'sites/all/libraries' directory. If you keep
the Swift Mailer library in 'sites/all/libraries/swiftmailer', then you need to
configure the module to look for the Swift Mailer library in that directory. The
location of the Swift Mailer library can be found on the page 'admin/config/swiftmailer'.

After the module has been configured with the Swift Mailer library you are advised
to make sure that the Swift Mailer library sends e-mails using the right transport
option. You can choose between SMTP, sendmail (or any other locally installed MTA)
and PHP's mail() functidocson. Please observe the various configuration options which
are available for each of the transport options.

You also have the chance to decide whether e-mail messages should be sent as plain
text or HTML by default. Furthermore, you can also set the character set which is
to be used as default. You are advised not to change any of these options if you
are uncertain about what they mean.

You can test whether the Swift Mailer library sends e-mail messages correctly by
going to 'admin/config/swiftmailer/test' when the module has been configured and
you are ready to put it into action.

2.0 Theming

All e-mails sent by the SwiftMailer module can be themed. This is useful when
e-mails should reflect the look and feel of the website it was sent from. The
SwiftMailer modules delegates theming to the Drupal theming system. This means
that you can theme e-mails the same way you theme other content.

2.1 Theme File

The easiest way to theme e-mails is to create a file named 'swiftmailer.tpl.php'
in your theme folder. Please see section 2.1.1 for an overview of the variables
you can use in the 'swiftmailer.tpl.php' file. The theme file should hold markup
which wraps the actual content of the message. The below block of code demonstrates
what a 'swiftmailer.tpl.php' might look like.

-- start of 'swiftmailer.tpl.php' --

<div>
    <?php print $body; ?>
</div>

-- end of 'swiftmailer.tpl.php' --

2.1.1 Theme File Variables

This section is an overview of the basic variables that are available from the
theme file 'swiftmailer.tpl.php'. Additional variables might be added by Drupal
or other modules.

$key
  The key which identifies the e-mail. This is the $key provided to drupal_mail()
  which identifies the e-mail.
$to
  The recipient's e-mail address.
$from
  The sender's e-mail address.
$language
  The language used to compose the e-mail.
$params
  An array of parameters. This is the $params optionally provided to drupal_mail().
$subject
  The subject.
$body
  The actual content.

2.2 Theme Functions

Just as you have the chance to control how e-mails are formatted with a theme file,
you also have the chance to use a theme function to achieve the same result. The
hook which you will be using to theme e-mails is called 'swiftmailer'. Thus, if you
are developing a theme which is to theme e-mails with a function, you would need
to call the function yourtheme_process_swiftmailer(&$variables). For a complete
list of all available theme functions, please refer to the theme() documentation
at http://api.drupal.org/api/drupal/includes--theme.inc/function/theme/7. The

2.3 Theming Scenarios

This section demonstrates how to solve different theming scenarios. The scenarios
are intended to cover the most usual use cases.

2.3.1 Theme Function/Theme File Need To Add An Inline Image.

Some themes includes an image which need to be displayed in all e-mails. This might
be a logo or artwork which need to be displayed in the header or footer of the e-mail.
However, the hook_mail() function should not be responsible for adding the logo or
artwork as this ideally is the responsibility of the theme.

What you need to do in this case is a two step process. The first step will be
to add the markup of the logo or artwork from the theme file or theme function.
However, the image markup needs to reference the image by CID - and not by URL.
The below line of code demonstrates this.

<img src="cid:logo" />

Now, the actual image (identified by "logo") needs to be added to the e-mail.
This is done by having your theme implement the hook 'swiftmailer_embed'. Thus,
your theme would have a function called mytheme_swiftmailer_embed($key) which
returns an array holding one or more images to embed. The returned array needs
to hold stdClass instances each representing an image. Please see section 3.3 for
more details about this. The below block of code demonstrates this.

function mytheme_swiftmailer_embed($key) {

    $image = new stdClass();
    $image->uri = 'sites/all/modules/swiftmailer/images/drupal.jpg';
    $image->type = 'file';
    $image->filemime = 'image/jpeg';
    $image->filename = 'logo.jpg';
    $image->cid = 'logo';

    return $image;

}

Please observe how the image defined in the above block of code has the 'cid'
field set to 'logo'. This establishes a link between the markup and the actual
image file.

3.0 Developers

This section is targeted towards developers. It demonstrates how the message format
of any given e-mail can be set, along with how files and inline images can be attached
to e-mails.

3.1 Message Format

You can specify wether a message should be sent as plain text ('text/plain') or 
HTML ('text/html'). Furthermore, the character set can also be set. The below 
example demonstrates how this can be achieved.

/**
 * Send an e-mail
 *
 */
function test() {
  
  // Define message format. 
  $p = array(
    'format' => 'text/html', 
    'charset' => 'UTF-8',
    );
   
  // Send message.
  drupal_mail('mymodule', 'key', 'test@test.com', language_default(), $p);
  
}

3.2 Attachments

You can easily add attachments to e-mails. This can be done programatically by defining 
one or more files to attach. 

All files which are to be attached to an e-mail need to be represented as instances of 
stdClass. This makes it easy for you to add files that are managed by Drupal, as the 
file_load() function will return an stdClass instance which represents a given file. 

All stdClass instances returned by Drupal which represents files are populated with the 
fields 'uri', 'filename' and 'filemime'. Thus, if you would like to attach a file that
are not managed by drupal, you then need to create an instance of stdClass and populate
that instance with the fields 'uri', 'filename' and 'filemime'. Drupal's drupal_realpath() 
will be used to determine the actual location of the provided file as given in the 'uri' 
field. Thus, files from both public and private file systems can be attached to e-mails.

The below example demonstrates both how to attach a file managed by Drupal and a file
which is not managed by Drupal.

Please note! You can specify which files to add as attachments both from the code block
where you invoke drupal_mail() and from your module's implementation of hook_mail(). In
hook_mail(), simply make sure you add attachments to $message['files'] and not to the
provided $params argument.

/**
 * Send an e-mail.
 */
function test() {

  //File one (managed by Drupal).
  $file_one = file_load(1);

  //File two (not managed by Drupal).
  $file_two = new stdClass();
  $file_two->uri = 'sites/default/files/images/logo.jpg';
  $file_two->filename = 'drupal_logo.jpg';
  $file_two->filemime = 'image/jpeg';
  
  // Add attachments.
  $p['files'][] = $file_one;
  $p['files'][] = $file_two;

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

      //File three (managed by Drupal).
      $file_three = file_load(2);
      $message['files'][] = $file_three;

      $message['subject'] = t('Test');
      $message['body'] = $text;
      break;
    }

}

It should be stressed that the module only supports attaching already existing files. 
In other words, dynamically generated files which are to be added as attachment
to an e-mail needs to be generated and stored in a permanent or temporary location
before it is provided as an attachment. A recommended way to handle temporary files
is to utilise Drupals file system and mark the files as temporary. Drupal will then
take care of deleting those files after a set amount of time.

3.2.1 The Attachment Hook

The SwiftMailer module allows other modules to add attachments to e-mails. This is
done through the hook 'swiftmailer_attach' which other modules need to implement in
order to add e-mails. The below block of code demonstrates how a module can add
attachments to an e-mail using the 'swiftmailer_attach' hook.

function swiftmailer_swiftmailer_attach($key) {

    // Load file which is managed by Drupal.
    return file_load(2);

}

3.3 Inline Images

Adding inline images to e-mails is just as easy as adding files as attachments. The
procedure is more or less identical.

All images which are to be attached to an e-mail need to be represented as instances of 
stdClass. This makes it easy for you to add image files that are managed by Drupal, as 
the file_load() function will return an stdClass instance which represents a given image
file. However, in contrast to attachments, you will ALWAYS need to manually apply the field
'cid' to an image file which is to be used as an inline image. The 'cid' field need to hold
the id of the image file, and will be used to establish a link between the attached image 
and its display location in the e-mail body.

All stdClass instances returned by Drupal which represents image files are populated with 
the fields 'uri', 'filename' and 'filemime'. Thus, if you would like to attach an image file 
that are not managed by Drupal, you then need to create an instance of stdClass and populate
that instance with the fields 'uri', 'filename' and 'filemime'. Drupal's drupal_realpath() 
will be used to determine the actual location of the provided image file as given in the 'uri' 
field. Thus, image files from both public and private file systems can be attached to e-mails.

The below example demonstrates both how to attach an image file managed by Drupal and an image
file which is not managed by Drupal.

/**
 * Implementation of hook_mail().
 */
function modulename_mail($key, &$message, $params) {

  switch($key) {
    default:
      
      $logo_id = '390ffcm-sdfd94f';
    
      $text[] = '<img src="cid:' . $logo_id . '" />;
      $text[] = t('<p>This is an automatically generated test e-mail with an inline image..</p>');

			//Inline image (managed by Drupal).
			$inline_image = file_load(1);
			$inline_image->cid = $logo_id;
			$message['images'][] = inline_image;

      $message['subject'] = t('Test');
      $message['body'] = $text;
      break;
    }

}

3.3.1 The Embed Hook

The SwiftMailer module allows other modules to add embeddable images to e-mails. This
is done through the hook 'swiftmailer_embed' which other themes or modules need to
implement in order to add e-mails. The below block of code demonstrates how a theme or
module can add attachments to an e-mail using the 'swiftmailer_embed' hook.

function swiftmailer_swiftmailer_embed($key) {

    // Load file which is managed by Drupal.
    $image =  file_load(2);
    $image->cid = "logo";

    return $image;

}