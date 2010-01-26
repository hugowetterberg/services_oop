<?php
// $Id: ServicesOop.inc,v 1.1 2010/01/26 08:36:20 hugowetterberg Exp $

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
  public $arguments;
  public $allow_overrides;
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
            $rdef = self::definitionFromMethod($class_name, $method);
            $rdef['callback'] = $class_name . '::' . $a->controller;
            $def['relationships'][$a->name] = $rdef;
          }
        break;
        case 'ActionAnnotation':
          if ($class->hasMethod($a->controller)) {
            $method = $class->getMethod($a->controller);
            $adef = self::definitionFromMethod($class_name, $method);
            $adef['callback'] = $class_name . '::' . $a->controller;
            $def['actions'][$a->name] = $adef;
          }
        break;
        case 'TargetedActionAnnotation':
          if ($class->hasMethod($a->controller)) {
            $method = $class->getMethod($a->controller);
            $adef = self::definitionFromMethod($class_name, $method);
            $adef['callback'] = $class_name . '::' . $a->controller;
            $def['targeted actions'][$a->name] = $adef;
          }
        break;
      }
    }

    // The crud methods
    $methods = array(
      'retrieve' => array(),
      'create' => array(),
      'update' => array(),
      'delete' => array(),
      'index' => array(),
    );

    // Add crud controllers
    foreach ($methods as $name => $base) {
      if ($class->hasMethod($name)) {
        $method = $class->getMethod($name);
        $mdef = self::definitionFromMethod($class_name, $method);
        $mdef['callback'] = $class_name . '::' . $name;
        $def[$name] = array_merge($base, $mdef);
      }
    }

    return $def;
  }

  private static function definitionFromMethod($class_name, $method) {
    $def = array();
    $doc = $method->getDocComment();
    $desc = new ServicesOopDescriptionParser($doc);
    $annotations = $method->getAllAnnotations();

    foreach ($annotations as $a) {
      $type = get_class($a);
      switch ($type) {
        case 'AccessAnnotation':
          $def['access callback'] = $a->callback;
          $def['access arguments'] = $a->args;
          $def['access arguments append'] = $a->appendArgs;
        break;
        case 'ModelAnnotation':
          $def['models'][$a->class] = array(
            'class' => $a->implementation,
            'arguments' => $a->arguments,
            'allow_overrides' => $a->allow_overrides,
          );
        break;
        case 'RESTRequestParserAnnotation':
          $def['rest request parsers'][$a->mime] = $a->parser;
        break;
      }
    }

    // Get doc info for arguments
    $matches = array();
    $argdoc = array();
    if (preg_match_all('/\@param ([a-z]+) \$([a-z_]+) (\[.*\])/i', $doc, $matches)) {
      for ($i = 0; $i<count($matches[1]); $i++) {
        $source = json_decode($matches[3][$i]);
        if (count($source)===1) {
          $source = $source[0];
        }
        else {
          $source = array($source[0] => $source[1]);
        }

        $type = $matches[1][$i];
        switch ($type) {
          case 'stdClass':
          case 'object':
            $type = 'struct';
            break;
        }

        $argdoc[$matches[2][$i]] = array(
          'name' => $matches[2][$i],
          'type' => $type,
          'description' => $desc->getDescription('param', $matches[2][$i]),
          'source' => $source,
        );
      }
    }

    // Construct the arg array
    $args = array();
    foreach ($method->getParameters() as $arginfo) {
      $doc = $argdoc[$arginfo->getName()];

      if (!is_array($doc)) {
        drupal_set_message(t('ServicesOOP: Missing documentation for the parameter "$!name" in !class::!method', array(
          '!name' => $arginfo->getName(),
          '!class' => $class_name,
          '!method' => $method->getName(),
        )), 'error');
      }

      $arg = array_merge($doc, array(
        'optional' => $arginfo->isOptional(),
      ));
      if ($arg['optional']) {
        $arg['default value'] = $arginfo->isDefaultValueAvailable() ? $arginfo->getDefaultValue() : NULL;
      }
      $args[] = $arg;
    }
    $def['args'] = $args;
    $def['help'] = $desc->getDescription();
    return $def;
  }
}

class ServicesOopDescriptionParser {
  private $desc;

  public function __construct($doc) {
    $this->desc = $this->descriptionsFromDoc($doc);
  }

  /**
   * Gets a description. Defaults to returning the description of the comment
   * target.
   *
   * @param string $type
   *  The type to get the description of. Could be 'target', 'param' or any
   *  used @tag in the doc comment.
   * @param string $context
   *  The context to get the description of. If $type is 'param' this should be
   *  the parameter name.
   * @param bool $trimmed_string
   *  Set to FALSE to get the raw line array. Defaults to TRUE.
   * @return mixed
   *  An array containing the description lines, or a trimmed string.
   */
  public function getDescription($type='target', $context=NULL, $trimmed_string=TRUE) {
    $desc = array();
    if ($context) {
      if (isset($this->desc[$type][$context])) {
        $desc = $this->desc[$type][$context];
      }
    }
    else {
      if (isset($this->desc[$type])) {
        $desc = $this->desc[$type];
      }
    }

    if ($trimmed_string) {
      return trim(join($desc, "\n"));
    }
    return $desc;
  }

  /**
   * Constructs an array with the main description and the description of
   * any @... that exists in the doc-comment.
   *
   * @param string $doc
   *  The raw doc-comment from the reflected method
   * @return array
   *  A associative array containing all descriptions as arrays of lines. The
   *  main description is in ['target']. Descriptions of parameters are in
   *  ['param']['param_name'].
   */
  private function descriptionsFromDoc($doc) {
    $desc = array();
    $doc = preg_replace('/^\s*(\/\*\*)|(\*\/)|\*/', '', $doc);
    $lines = split("\n", $doc);
    $state = 'target';
    $context = NULL;
    foreach ($lines as $line) {
      $line = trim($line);
      $state_matches = array();
      if (preg_match('/^@([a-z]+)(\s+[a-z]+)?(\s+\$([a-z_]+))?/i', $line, $state_matches)) {
        $state = $state_matches[1];
        $context = empty($state_matches[4]) ? NULL : $state_matches[4];
        continue;
      }

      if ($context) {
        $desc[$state][$context][] = $line;
      }
      else {
        $desc[$state][] = $line;
      }
    }
    return $desc;
  }
}