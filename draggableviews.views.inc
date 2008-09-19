<?php
// $Id$

/**
 * @file
 * Derives the view style plugin
 */
function draggableviews_views_plugins(){
  return array(
    'module' => 'draggableviews', // This just tells our themes are elsewhere.

    'style' => array(
      'draggabletable' => array(
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

?>