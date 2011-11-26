<?php

/**
 * Implements hook_views_data_alter().
 */
function draggableviews2_views_data_alter(&$data) {
  foreach (entity_get_info() as $entity_type => $info) {
    if (isset($info['base table']) && isset($data[$info['base table']])) {
      $data[$info['base table']]['draggableviews'] = array(
        'title' => t('Draggable views'),
        'help' => t('Provide a draggable functionality.'),
        'real field' => $info['entity keys']['id'],
        'field' => array(
          'handler' => 'draggableviews_handler_field_draggable',
          'click sortable' => FALSE,
        ),
      );
    }
  }
}
