<?php
// $Id$

/**
 * @file
 * The default implementation of draggableviews handlers.
 */

/*
 * Default Implementation: storage done by draggableviews
 */
class draggableviews_handler_auto {
  var $view;
  
  function init($field_name, &$view) {
  	$this->field_name = $field_name;
  	$this->view       = $view;
  	
  	// TODO: besser machen
  	$this->delta = $this->view->field[$field_name]->definition['draggableviews_delta'];
  }
  
  function save($nid, $value) {
    db_query('DELETE FROM {draggableviews_structure} WHERE vid=%d AND nid=%d AND delta=%d', $this->view->vid, $nid, $this->delta);
    db_query('INSERT INTO {draggableviews_structure}(vid, nid, delta, value) VALUES(%d, %d, %d, %d)', $this->view->vid, $nid, $this->delta, $value);
  }
  
  function get_form_element($value, $attributes = array()) {
    if (!isset($attributes['class'])) $attributes['class'] = $field['field_name'];

    switch ($this->field->content_field['widget']['type']) {
      
  	  default:
      
        return array(
          '#type'  => 'select',
          '#name'  => $attributes['field_name'],
          '#value' => $value,
          // slice out the first and the last element, so we make it possible
          // that new nodes may appear on the very top or the very bottom
          '#options' => array(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20),
          '#attributes' => array('class' => $attributes['class']),
        );
        
        break;

      case 'hidden':

        return array(
          '#type' => 'hidden',
          '#name' => $attributes['field_name'],
          '#value' => $value,
          '#attributes' => array('class' => $attributes['class']),
        );
    }
  }
  
  function get_minimum_value() {
    return -1;
  }
}