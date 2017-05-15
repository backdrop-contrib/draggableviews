<?php

/**
 * @file
 * Hooks provided by the Draggableviews module.
 */

/**
 * If Native handler used, you can alter arguments set before they are saved to 
 * the database.
 *
 * This can be used when you would like to exclude or add some of arguments
 * to be recorded to database. Also you can add new records to be saved to
 * database (for example for translated nodes, etc.)
 *
 * @param array $arguments
 *   Array of arguments before saving.
 * @param array $form_values
 *   Array of submitted entity ids and weights.
 * @param object $view
 *   Views object.
 */
function hook_draggableviews_handler_native_arguments_alter(&$arguments, $view, &$form_values) {
  
}


/**
 * If Field API handler used, you can alter values set before they are saved to 
 * the database.
 *
 * @param array $form_values
 *   Array of submitted entity ids and weights.
 * @param object $view
 *   Views object.
 */
function hook_draggableviews_handler_native_arguments_alter(&$form_values, $view) {
  
}


/**
 * Provides a list of Draggableviews handlers.
 *
 * @param return $return
 *   An array of handlers, each being an associative array with two required
 *   keys:
 *     - label: A translated string for the name of the handler.
 *     - class: The name of the handler Class.
 */
function hook_draggableviews_handler_info() {
  $plugins = array(
    'draggableviews_handler_fieldapi' => array(
      'label' => t('FieldAPI'),
      'class' => 'draggableviews_handler_fieldapi',
    ),
    'draggableviews_handler_native' => array(
      'label' => t('Native'),
      'class' => 'draggableviews_handler_native',
    ),
    'draggableviews_hierarchy_handler_native' => array(
      'label' => 'Native',
      'class' => 'draggableviews_hierarchy_handler_native',
    ),
  );

  return $plugins;
}

