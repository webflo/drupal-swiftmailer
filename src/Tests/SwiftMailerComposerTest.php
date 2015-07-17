<?php

/**
 * @file
 * Contains \Drupal\swiftmailer\Tests\SwiftMailerComposerTest.
 */

namespace Drupal\swiftmailer\Tests;

use Drupal\simpletest\WebTestBase;
use Swift_Message;

/**
 * Tests the composer integration.
 *
 * @group swiftmailer
 */
class SwiftMailerComposerTest extends WebTestBase {

  /**
   * Modules to enable.
   *
   * @var array
   */
  public static $modules = [
    'swiftmailer'
  ];

  public function testComposer() {
    // Create the message
    $message = Swift_Message::newInstance();
    $this->assertTrue($message instanceof Swift_Message);
  }

}
