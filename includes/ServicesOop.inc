<?php
// $Id$

/**
 * @Target("class")
 */
class RelationshipAnnotation extends Annotation {
  public $name;
  public $controller;
}

/**
 * @Target("class")
 */
class ActionAnnotation extends RelationshipAnnotation {}

/**
 * @Target("class")
 */
class TargetedActionAnnotation extends RelationshipAnnotation {}

/**
 * @Target("method")
 */
class RESTRequestParserAnnotation extends Annotation {
  public $mime;
  public $parser;
}

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

class ServicesOop {
  public static function definitionFromClass($class_name) {
    $def = array();

    $class = new ReflectionAnnotatedClass($class_name);

    // Add relationship controllers
    $annotations = $class->getAllAnnotations();
    foreach ($annotations as $a) {
      $type = get_class($a);
      switch ($type) {
        case 'RelationshipAnnotation':
          if ($class->hasMethod($a->controller)) {
            $method = $class->getMethod($a->controller);
            $rdef = self::definitionFromMethod($method);
            $rdef['#callback'] = $class_name . '::' . $a->controller;
            array_shift($rdef['#args']); // Remove the resource id argument
            $def['#relationships'][$a->name] = $rdef;
          }
        break;
        case 'ActionAnnotation':
          if ($class->hasMethod($a->controller)) {
            $method = $class->getMethod($a->controller);
            $adef = self::definitionFromMethod($method);
            $adef['#callback'] = $class_name . '::' . $a->controller;
            $def['#actions'][$a->name] = $adef;
          }
        break;
        case 'TargetedActionAnnotation':
          if ($class->hasMethod($a->controller)) {
            $method = $class->getMethod($a->controller);
            $adef = self::definitionFromMethod($method);
            $adef['#callback'] = $class_name . '::' . $a->controller;
            array_shift($adef['#args']); // Remove the resource id argument
            $def['#targeted actions'][$a->name] = $adef;
          }
        break;
      }
    }

    // The crud methods together with their default authorization levels
    $methods = array(
      'retrieve' => array('#authorization level' => 'read'),
      'create' => array('#authorization level' => 'create'),
      'update' => array('#authorization level' => 'update'),
      'delete' => array('#authorization level' => 'delete'),
      'index' => array('#authorization level' => 'read'),
    );

    // Add crud controllers
    foreach ($methods as $name => $base) {
      if ($class->hasMethod($name)) {
        $method = $class->getMethod($name);
        $mdef = self::definitionFromMethod($method);
        $mdef['#callback'] = $class_name . '::' . $name;
        $def['#' . $name] = array_merge($base, $mdef);
      }
    }

    return $def;
  }

  private static function definitionFromMethod($method) {
    $def = array();
    $doc = $method->getDocComment();
    $annotations = $method->getAllAnnotations();

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
        case 'RESTRequestParserAnnotation':
          $def['#rest request parsers'][$a->mime] = $a->parser;
        break;
      }
    }

    // Get doc info for arguments
    $matches = array();
    $argdoc = array();
    if (preg_match_all('/\@param ([a-z]+) \$([a-z]+) (\[.*\])/', $doc, $matches)) {
      for ($i = 0; $i<count($matches[1]); $i++) {
        $source = json_decode($matches[3][$i]);
        if (count($source)===1) {
          $source = $source[0];
        }
        else {
          $source = array($source[0] => $source[1]);
        }

        $argdoc[$matches[2][$i]] = array(
          '#name' => $matches[2][$i],
          '#type' => $matches[1][$i],
          '#source' => $source,
        );
      }
    }

    // Construct the arg array
    $args = array();
    foreach ($method->getParameters() as $arginfo) {
      $doc = $argdoc[$arginfo->getName()];
      $arg = array_merge($doc, array(
        '#optional' => $arginfo->isOptional(),
      ));
      if ($arg['#optional']) {
        $arg['#default value'] = $arginfo->isDefaultValueAvailable() ? $arginfo->getDefaultValue() : NULL;
      }
      $args[] = $arg;
    }
    $def['#args'] = $args;

    return $def;
  }
}