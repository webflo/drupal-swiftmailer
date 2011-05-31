<?php
function swift_mailer_page_swift_mailer_test() {

    $file = new stdClass();
    $file->uri = 'CHANGELOG.txt';
    $file->filename = 'CHANGELOG.txt';
    $file->filemime = 'text/plain';
    $p['attachments'][] = $file;

    drupal_mail('swift_mailer', 'test', 'stian@brattland.no', language_default(), $p);
    return t('An e-mail has been sent to stian@brattland.no.');
}
?>
