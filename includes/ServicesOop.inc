<?php
// $Id$

/**
 * @Target("method")
 */
class AccessAnnotation extends Annotation {
  public $callback;
  public $args;
  public $appendArgs;
}

/**
 * @Target("method")
 */
class ModelAnnotation extends Annotation {
  public $class;
  public $implementation;
}

class Services {
  public static function definitionFromClass($class_name) {
    $def = array();

    $class = new ReflectionAnnotatedClass($class_name);

    $methods = array(
      'retrieve' => array(),
      'create' => array(),
      'update' => array(),
      'delete' => array(),
      'index' => array(),
    );
    foreach ($methods as $name => $params) {
      if ($class->hasMethod($name)) {
        $method = $class->getMethod($name);
        $mdef = self::definitionFromMethod($method);
        $mdef['#callback'] = $class_name . ':' . $name;
        $def['#' . $name] = $mdef;
      }
    }

    return $def;
  }

  private static function definitionFromMethod($method) {
    $def = array();
    $doc = $method->getDocComment();
    $annotations = $method->getAnnotations();

    foreach ($annotations as $a) {
      $type = get_class($a);
      switch ($type) {
        case 'AccessAnnotation':
          $def['#access callback'] = $a->callback;
          $def['#access arguments'] = $a->args;
          $def['#access arguments append'] = $a->appendArgs;
        break;
        case 'ModelAnnotation':
          $def['#models'][$a->class] = array(
            'class' => $a->implementation,
          );
        break;
      }
    }

    // Create a argument array
    $num_req = $method->getNumberOfRequiredParameters();
    $matches = array();
    $args = array();
    if (preg_match_all('/\@param ([a-z]+) \$([a-z]+) (\[.*\])/', $doc, $matches)) {
      for ($i = 0; $i<count($matches[1]); $i++) {
        $source = json_decode($matches[3][$i]);
        if (count($source)===1) {
          $source = $source[0];
        }
        else {
          $source = array($source[0] => $source[1]);
        }

        $args[] = array(
          '#name' => $matches[2][$i],
          '#type' => $matches[1][$i],
          '#source' => $source,
          '#optional' => $i >= $num_req,
        );
      }
    }
    $def['#args'] = $args;

    return $def;
  }
}