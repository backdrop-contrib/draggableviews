<?php
// $Id$

/**
 * @file
 * Derives the view style plugin
 */

/**
 * Implemening hook_views_plugins
 */
function draggableviews_views_plugins() {
  return array(
    'module' => 'draggableviews', // This just tells our themes are elsewhere.

    'style' => array(
      'draggabletable' => array(
        'path' => drupal_get_path('module', 'draggableviews') . '/views',
        'title' => t('Draggable Table'),
        'help' => t('Displays rows in a table and makes them draggable.'),
        'handler' => 'draggableviews_plugin_style_draggabletable',
        'theme' => 'draggableviews_view_draggabletable',
        'theme file' => 'draggableviews_theme.inc',
        'uses row plugin' => TRUE,
        'uses options' => TRUE,
        'uses fields' => TRUE,
        'type' => 'normal',
        'parent' => 'table',
      ),
    ),
  );
}

/**
 * Implemening hook_views_handlers
 */
function draggableviews_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'draggableviews') . '/views/handlers',
    ),
    'handlers' => array(

      // field handlers
      'views_handler_field_draggableviews_structure' => array(
        'parent' => 'views_handler_field',
      ),

      // sort handlers
      'views_handler_sort_draggableviews_structure' => array(
        'parent' => 'views_handler_sort',
      ),
    ),
  );
}

/**
 * Implementing of hook_views_data.
 */
function draggableviews_views_data() {
  $data = array();

  for ($i = 0; $i < 3; $i++) {
    $data['draggableviews_structure'.$i]['table'] = array(
      'group' => t('Draggableviews'),
      'join' => array(
        'node' => array(
          'table' => 'draggableviews_structure',
          'left_field' => 'nid',
          'field' => 'nid',
          'extra' => array(
            array(
              'field' => 'delta',
              'operator' => '=',
              'value' => $i,
              'numeric' => TRUE,
            ),
          ),
        ),
      ),
    );
    $data['draggableviews_structure'. $i]['value'. $i] = array(
      'real field' => 'value',
      'title' => t('Value #'. ($i + 1)),
      'field' => array(
        'handler' => 'views_handler_field_draggableviews_structure',
        'help' => t('This value provided by draggableviews can be used either for order or for parent. It is unique for each view.'),
        'click sortable' => TRUE,
        'draggableviews_delta' => $i,
      ),
      'sort' => array(
        'handler' => 'views_handler_sort_draggableviews_structure',
        'help' => t('Sort by value'),
      ),
    );
  }

  return $data;
}

function draggableviews_views_query_alter(&$view, &$query) {
  for ($i = 0; $i < 3; $i++) {
    if (isset($query->table_queue['draggableviews_structure'. $i])) {
      $query->table_queue['draggableviews_structure'. $i]['join']->extra[] = array(
          'field' => 'vid',
          'operator' => '=',
          'value' => $view->vid,
          'numeric' => TRUE,
      );
    }
  }
}