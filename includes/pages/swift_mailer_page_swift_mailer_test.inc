<?php
function swift_mailer_page_swift_mailer_test() {
    drupal_mail('swift_mailer', 'test', 'sb@upright-music.com', language_default());
    return t('An e-mail has been sent to stian@brattland.no.');
}
?>
