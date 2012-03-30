<?php
/**
 * @file
 * The default template file for e-mails.
 */
?>

<div>
    <table width="100%" cellpadding="0" cellspacing="0">
        <tr style="background: rgb(255,255,255);">
            <td width="125px">
              <img src="image:<?php print drupal_get_path('module', 'swiftmailer') . '/images/drupal.jpg'; ?>" />
            </td>
            <td valign="middle">
                <h3 style="margin: 0px; padding: 0px;"><?php print $subject; ?></h3>
            </td>
        </tr>
        <tr>
            <td></td>
            <td>
                <div style="padding: 10px 0px 0px 0px; font-family: Arial; font-size: 10px;">
                    <?php print $body; ?>
                </div>
            </td>
    </table>
</div>
