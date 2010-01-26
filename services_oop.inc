<?php
// $Id: services_oop.inc,v 1.1 2010/01/26 08:36:20 hugowetterberg Exp $

function _services_oop_autoload_info() {
  return array(
    'ServicesOop' => array(
      'file' => 'includes/ServicesOop.inc',
    ),
    'RelationshipAnnotation' => array(
      'file' => 'includes/ServicesOop.inc',
    ),
    'ActionAnnotation' => array(
      'file' => 'includes/ServicesOop.inc',
    ),
    'TargetedActionAnnotation' => array(
      'file' => 'includes/ServicesOop.inc',
    ),
    'RESTRequestParserAnnotation' => array(
      'file' => 'includes/ServicesOop.inc',
    ),
    'AccessAnnotation' => array(
      'file' => 'includes/ServicesOop.inc',
    ),
    'ModelAnnotation' => array(
      'file' => 'includes/ServicesOop.inc',
    ),
    'ReflectionAnnotatedClass' => array(
      'file' => 'lib/annotations.php',
    ),
    'Annotation' => array(
      'file' => 'lib/annotations.php',
    ),
  );
}