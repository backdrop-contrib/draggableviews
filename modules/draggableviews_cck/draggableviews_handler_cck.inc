<?php
// $Id$

/**
 * @file
 * The default implementation for draggableviews.
 */

/*
 * Default Implementation with CCK-fields
 */
class draggableviews_handler_cck {
	
  function init($field_name, &$view) {
  	$this->field_name      = $field_name;
  	
    $this->options         = $view->display['default']->display_options['style_options'];
    $this->fields          = $view->field;
    $this->field           = $this->fields[$field_name];
    
    $this->field_alias     = $this->fields[$field_name]->field_alias;
    
    $this->real_field_name = $this->fields[$field_name]->content_field['field_name'];
    $this->field_type      = $this->fields[$field_name]->content_field['type'];
    $this->minimum_value   = 0;
  }
  
  function save($nid, $value) {
    
    $node = node_load(array('nid' => $nid));
  // get field
  if (isset($node->{$this->real_field_name})) {
    $field = &$node->{$this->real_field_name};
  }
  else {
    // return if field does not exist
    return;
  }
  
  if (!isset($value)) {
    // sometimes there is no value available (e.g. a root without a parent)
    
    // set empty array
    $value = 0;
  }
  
  //differ between certain field types
  switch ($this->field_type) {
    case 'nodereference':
      $field[0]['nid'] = $value;
      break;
    
    default:
    case 'number_integer':
      $field[0]['value'] = $value;
      break;
  }
  
    // finally save the node
    if ($node = node_submit($node)) {
      node_save($node); // CAUTION, BUG: node_save destroys og_groups array
    }
  }
  
  function get_form_element($value, $attributes = array()) {
    if (!isset($attributes['class'])) $attributes['class'] = $field['field_name'];

  switch ($this->field->content_field['widget']['type']) {
  
    case 'optionwidgets_select':
    
      return array(
        '#type'  => 'select',
        '#name'  => $attributes['field_name'],
        '#value' => $value,
        // slice out the first and the last element, so we make it possible
        // that new nodes may appear on the very top or the very bottom
        '#options' => array_slice(content_allowed_values($this->field->content_field), 1, -1, TRUE),
        '#attributes' => array('class' => $attributes['class']),
      );
      
      break;
      
    case '__nodereference_select':
    
      return array(
        '#type' => 'select',
        '#name' => $specific_field_name != NULL ? $specific_field_name: $field_name .'_'. $nid,
        '#value' => $value,
        '#options' => content_allowed_values($field),
        '#attributes' => array('class' => $field_name),
      );
      
      break;
      
    return array
        (
            '#theme' => 'content_multiple_values',
            '#title' => 'parent',
            '#required' => 0,
            '#description' => NULL, 
            0 => array
                (
                    '#type' => 'nodereference_autocomplete',
                    '#default_value' => array
                        (
                            'nid' => NULL,
                            '_error_element' => 'default_value_widget][field_parent][0][nid][nid',
                        ),

                    '#value_callback' => 'nodereference_autocomplete_value',
                    '#title' => 'parent',
                    '#description' => NULL,
                    '#required' => NULL,
                    '#weight' => 0,
                    '#delta' => 0,
                    '#columns' => array
                        (
                            0 => 'nid',
                        ),

                    '#field_name' => 'field_parent',
                    '#type_name' => 'task',
                ),

            '#field_name' => 'field_parent',
            '#tree' => 1,
            '#weight' => 3,
            '#access' => 1,
            '#count' => 5,
        );
        
      break;
      
    case 'hidden':
    default:
     return array(
        '#type' => 'hidden',
        '#name' => $attributes['field_name'],
        '#value' => $value,
        '#attributes' => array('class' => $attributes['class']),
      );
  }
  }
  
  function get_minimum_value() {
    return 1;
  }
}