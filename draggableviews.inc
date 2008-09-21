d<?php
// $Id$

/**
 * @file
 * Draggableviews processing functions.
 */

include_once('draggableviews_theme.inc');

/**
 * Build the form.
 */
function draggableviews_view_draggabletable_form($form_state, $view) {
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}


/**
 * Implementing hook_submit
 */
function draggableviews_view_draggabletable_form_submit($vars) {
  // check permissions
  if (!user_access('administer nodes')) {
    drupal_set_message(t('You are not allowed to edit/reorder these nodes.'), 'error');
    return;
  }

  // gather all needed information
  $view = $vars['#parameters'][2]->view;
  $results = $view->result;
  // get input
  $input = $vars['submit']['#post'];
  
  $info = _draggableviews_info($view);
  
  
  // loop through all resulting nodes
  foreach ($results as $row) {
    
    if (isset($info['order']['fields'])) {
      $info['input'][$row->nid][$info['order']['fields'][0]['field_name']] = $input[$info['order']['fields'][0]['field_name'] .'_'. $row->nid];
    }
    
    if (isset($info['hierarchy'])) {
      $info['input'][$row->nid][$info['hierarchy']['field']['field_name']] = $input[$info['hierarchy']['field']['field_name'] .'_'. $row->nid];
    }
  }
  
  // build hierarchy
  _draggableviews_build_hierarchy($info);
  
  // save structure
  _draggableviews_save_hierarchy($info);
  
  // save expanded/collapsed state
  global $user;
  
  foreach ($vars['submit']['#post'] AS $key => $val) {
    if (ereg('draggableviews_collapsed_', $key)) {
      $parent_nid = substr($key, 25);
      db_query("DELETE FROM {draggableviews_collapsed}
                WHERE uid=". $user->uid ." AND 
                parent_nid=". $parent_nid);

      db_query("INSERT INTO {draggableviews_collapsed}
                (uid, parent_nid, collapsed) VALUES (%d, %d, %d)",
                $user->uid, $parent_nid, $val);
    }
  }
}

/**
 * Collect all known information in a handy array
 * 
 * @param $view
 *   The views object
 * 
 * @return
 *   An structured array containt the extracted draggableviews settings
 *   and additional field information.
 *   array(
 *     'order' => array(
 *       'fields' => array(
 *         0 => array(
 *           'field_name'      => field_name
 *           'real_field_name' => real_field_name,
 *           'field_alias'     => field_alias
 *           'field_type'      => field_type,
 *           'minimum_value'   => minimum
 *         ),
 *         ..
 *       ),
 *       'visible' => True/False
 *     ),
 *     'hierarchy' => array(
 *       'field' => array(
 *         'field_name'      => field_name
 *         'real_field_name' => real_field_name,
 *         'field_alias'     => field_alias
 *         'field_type'      => field_type,
 *       ),
 *       'visible' => True/False
 *     ),
 *
 *     'nodes' => array(
 *       nid1 => array(
 *         field_name1 => value
 *         field_name2 => value
 *          ..
 *       ),
 *       ..
 *     ),
 *   );
 */
function _draggableviews_info(&$view) {
  $options = $view->display['default']->display_options['style_options'];
  $fields  = $view->field;
  $results = $view->result;
  
  // build information array
  $info = array();
  if (isset($options['tabledrag_order'])) {
    foreach ($options['tabledrag_order'] as $field) {
      $info['order']['fields'][] = array(
        'field_name'        => $field['field'],
        'real_field_name'   => $fields[$field['field']]->content_field['field_name'],
        'field_alias'       => $fields[$field['field']]->field_alias,
        'field_type'        => $fields[$field['field']]->content_field['type'],
        'minimum_value'     => _draggableviews_field_get_minimum_value($fields[$field['field']]->content_field),
      );
    }
    $info['order']['visible'] = strcmp($options['tabledrag_order_visible']['visible'], 'visible') == 0 ? TRUE : FALSE;
    $info['order']['minimum_value'] = _draggableviews_field_get_minimum_value($fields[$field['field']]->content_field);
  }
  if ($options['tabledrag_hierarchy']['parent_field'] != 'none') {
    $info['hierarchy'] = array(
      'field' => array(
        'field_name'        => $options['tabledrag_hierarchy']['parent_field'],
        'real_field_name' => $fields[$options['tabledrag_hierarchy']['parent_field']]->content_field['field_name'],
        'field_alias'       => $fields[$options['tabledrag_hierarchy']['parent_field']]->field_alias,
        'field_type'        => $fields[$options['tabledrag_hierarchy']['parent_field']]->content_field['type'],
      ),
      'visible' => strcmp($options['tabledrag_hierarchy_visible']['visible'], 'visible') == 0 ? TRUE : FALSE,
    );
  }
  foreach ($options['tabledrag_types'] as $type) {
    $info['types'][$type['node_type']] = $type['type'];
  }
  if (isset($options['realtimeedit_enabled_fields'])) {
    foreach ($options['realtimeedit_enabled_fields'] as $field) {
      $info['realtimeedit'][$field['field']] = TRUE;
    }
  }
  
  // loop through all resulting nodes
  foreach ($results as $row) {
    foreach ($info['order']['fields'] as $field) {
      $info['nodes'][$row->nid][$field['field_name']] = $row->{$field['field_alias']};
    }
    if (isset($info['hierarchy'])) {
      $info['nodes'][$row->nid][$info['hierarchy']['field']['field_name']] = $row->{$info['hierarchy']['field']['field_alias']};
    }
  }
  
  return $info;
}

/**
 * Build hierarchy
 * 
 * This function also handles broken structures
 * 
 * @param info
 *   The structured information array
 */
function _draggableviews_build_hierarchy(&$info) {

  $nodes = &$info['nodes'];
  $input = &$info['input'];
  
  foreach ($nodes as $nid => $prop) {
    
    // get depth
    if (($depth = _draggableviews_get_hierarchy_depth($nid, $input, $info)) === FALSE) {
      // Error! The hierarchy structure is broken and could
      // look like the following: (we're currently processing X)
      // A
      //   --X
      // --D
      //
      // The next steps:
      //  1) bring it down to the root level
      //  2) Set order fields to the minimum
      
      $nodes[$nid][$info['hierarchy']['field']['field_name']] = 0;
      
      // We gracefully sidestep the order-loop
      $depth = -1;
      
      drupal_set_message("Draggableviews: An error was detected. The structure has been repaired.");
    }
    
    // Let's take a look at the following expample, to understand
    // what is beeing done.
    //
    // A
    // --B
    // --C
    //   --X
    // --D
    // E
    // Imagine we're currently processing X:
    //
    // We know that X is in depth=3, so we save the received 
    // weight value in the 3rd order field of node X.
    //
    // The 2nd order field must inherit the received weight of 
    // node C (the next parent). And the 1st order field must
    // inherit the received weight of node A (the parent of C).
    //
    // When we finally order the view by weight1, weight2, weight3 then
    // weight1 and weight2 from node X will always equal with
    // those from node A and B, and weight3 defines the order of the 3rd level.
    
    $temp_nid = $nid;
    
    for ($i = $depth; $i >= 0; $i--) {
      // we're operating top-down, so we determine the parents nid by the way
      
      $depth_field_name = $info['order']['fields'][$i]['field_name'];
      $first_field_name = $info['order']['fields'][0]['field_name'];
      
      $nodes[$nid][$depth_field_name] = $input[$temp_nid][$first_field_name];
      
      if (isset($info['hierarchy']) && $i > 0) {
        if (!($temp_nid = $input[$temp_nid][$info['hierarchy']['field']['field_name']])) {
          // this line should never be reached assumed the depth
          // was calculated correctly.
          drupal_set_message(t('Undefined State called in draggableviews_build_hierarchy(..)'), 'error');
          break;
        }
      }
    }
    
    if (isset($info['hierarchy']) && $depth > -1) {
      // Simply set the parent value
      
      $parent_field_name = $info['hierarchy']['field']['field_name'];
      
      $nodes[$nid][$parent_field_name] = $input[$nid][$parent_field_name];
    }
    
    // Now set all unused weights to a minimum value. Otherwise
    // it could happen that a child appears above its parent.
    // The ? can be anything, unfortunately also > 5
    //
    // --A (3,5)
    // B   (3,?)
    //
    // To guaranteer that the ? is always the lowest, we choose
    // the minimum minus 1 (stupidly cannot be displayed @todo).
    // Due to this it's recommended that all order fields have
    // the same minimum value!
    
    $depth = ($depth == -1) ? 0 : $depth;
    
    for ($i = $depth + 1; $i < count($info['order']['fields']); $i++) {
      
      $field_info = $info['order']['fields'][$i];
      
      $info['nodes'][$nid][$field_info['field_name']] = $field_info['minimum_value'];
    }
  }
}

/**
 * Rebuild hierarchy
 * 
 * This function is called when the structure is broken
 * 
 * @param info
 *   The structures information array
 */
function _draggableviews_rebuild_hierarchy(&$info) {

  drupal_set_message("Draggableviews: Rebuilding structure..");
  
  $nodes         = &$info['nodes'];
  $info['input'] = array();
  $input         = &$info['input'];
  
  // Build an input-array to simulate the form data we would receive on submit
  
  // loop through all nodes
  foreach ($nodes as $nid => $prop) {
    

    $depth = $prop['depth'] ? $prop['depth'] : 0;
    
    // use order weight of the hierarchy level the field is situated in
    $input[$nid][$info['order']['fields'][0]['field_name']] = $prop[$info['order']['fields'][$depth]['field_name']];
    
    if (isset($info['hierarchy'])) {
    
      // set parent
      $field_name = $info['hierarchy']['field']['field_name'];
      $input[$nid][$field_name] = $prop[$field_name];
    }
  }
  
  // build hierarchy
  _draggableviews_build_hierarchy($info);
  
  // save hierarchy
  _draggableviews_save_hierarchy($info);
  
  // redirect here
  // @todo
  drupal_set_message(t('Draggableviews: Refresh required (..will redirect automatically after testing)'), 'error');
}

/**
 * Detect and repair order collisions
 */
function _draggableviews_detect_order_collisions(&$info) {

  $nodes = &$info['nodes'];
  
  $collision = FALSE;
  
  // Detect order collisions
  // Check for the following:
  //  1) The minimum value should not be used as it
  //     should be possible for new nodes to default on top
  //     without order collisions
  //  2) @todo The last value should not be used...
  //  3) A order value should only be used once per depth
  
  // array(
  //   depth => array(order1, ..),
  //   ..
  // )
  $order = array();
  
  $min_value = $info['order']['minimum_value'];
  
  // loop through all nodes
  foreach ($nodes as $nid => $prop) {
    
    $depth = $prop['depth'];
    
    $begin_search = TRUE;
    
    // make sure that the minimum value cannot be used
    if (!is_array($order[$depth])) $order[$depth] = array($min_value);
    
    // first try to keep current value
    $tmp_order = $prop[$info['order']['fields'][$depth]['field_name']];
    
    while (is_numeric(array_search($tmp_order, $order[$depth]))) {
      // if there already exists an order value for this depth
      // we have to find another one.
      // Try to find a free order:
      
      if ($begin_search == TRUE) {
      
        $tmp_order = $min_value + 1;
        
        $begin_search = FALSE;
      }
      else {
        
        $tmp_order++;
      }
      
      $collision = TRUE;
    }
    
    $order[$depth] = array_merge(array($tmp_order), $order[$depth]);
    
    $nodes[$nid][$info['order']['fields'][$depth]['field_name']] = $tmp_order;
  }
  
  return !$collision;
}

/**
 * Set values and save nodes
 * 
 * @param $info
 *   The structured information array
 */
function _draggableviews_save_hierarchy($info) {

  // loop through all nodes
  foreach ($info['nodes'] as $nid => $prop) {
  
    $node = node_load(array('nid' => $nid));
  
    if (isset($info['hierarchy'])) {
    
      $field_info = $info['hierarchy']['field'];
      $value      = $prop[$field_info['field_name']];
      
      _draggableviews_node_set_value($node, $field_info['real_field_name'], $field_info['field_type'], $value);
    }
    
    foreach ($info['order']['fields'] as $field_info) {
      // loop through all order fields
      
      $value      = $prop[$field_info['field_name']];
    
      _draggableviews_node_set_value($node, $field_info['real_field_name'], $field_info['field_type'], $value);
    }
    
    // finally save the node
    if ($node = node_submit($node)) {
      node_save($node); // CAUTION, BUG: node_save destroys og_groups array
    }
  }
}

/**
 * Check order settings
 * 
 */
function _draggableviews_check_order($nid, &$info) {
  
  $nodes = &$info['nodes'];

  $temp_nid = $nid;
    
  for ($i = $nodes[$nid]['depth']; $i >= 0; $i--) {
    // we're operating top-down, so we determine the parents nid by the way
      
    $depth_field_name = $info['order']['fields'][$i]['field_name'];
    
    if ($nodes[$nid][$depth_field_name] != $nodes[$temp_nid][$depth_field_name]) {
      return FALSE;
    }
    
    if (isset($info['hierarchy']) && $i > 0) {
      if (!($temp_nid = $nodes[$temp_nid][$info['hierarchy']['field']['field_name']])) {
        // this line should never be reached assumed the depth
        // was calculated correctly.
        drupal_set_message(t('Undefined State called in draggableviews_build_hierarchy(..)'), 'error');
        return FALSE;
      }
    }
  }
  
  return TRUE;
}

/**
 * Calculate depth of all nodes
 * 
 * @param $info
 *   The structured information array
 */
function _draggableviews_calculate_depths(&$info) {

  $error = FALSE;
  
  // loop through all rows the view returns
  foreach ($info['nodes'] as $nid => $prop) {

    // determine hierarchy depth of current row
    $info['nodes'][$nid]['depth'] = _draggableviews_get_hierarchy_depth($nid, $info['nodes'], $info);
    
    if ($info['nodes'][$nid]['depth'] === FALSE) $error = TRUE;
  }
  
  return !$error;
}

/**
 * Get Hierarchy depth
 * 
 * This function detects cycles,
 * broken hierarchy structures
 * and wrong weight settings
 * 
 * @param $node
 *   The node from wich we want to know the depth
 * @param $info
 *   The structured information array
 * return
 *   The hierarchy depth,
 *   on error FALSE.
 */
function _draggableviews_get_hierarchy_depth($nid, &$nodes, &$info) {
  $depth = 0;
  $error = FALSE;
  $temp_nid = $nid;
  $field_name = $info['hierarchy']['field']['field_name'];
  $used_nids = array();
  $used_nids[] = $temp_nid;
  
  while (!($error = !isset($nodes[$temp_nid])) && ($temp_nid = $nodes[$temp_nid][$field_name]) > 0) {
    
    // In order to detect cycles we use an array,
    // where all used node ids are saved in. If any node id
    // occurs twice -> return FALSE
    $cycle_found = array_search($temp_nid, $used_nids);
    
    if (isset($cycle_found) && $cycle_found !== FALSE) {
      drupal_set_message(t("Draggableviews: A cycle was found."));
      return FALSE;
    }
    
    $used_nids[] = $temp_nid;
    $depth++;
  }
  
  if ($error) {
    // If loop breaked caused by an error
    return FALSE;
  }
  
  return $depth;
}

/*
 * Get accurate form element
 * 
 * @param $nid
 * @param $field_name
 * @param $value
 * @param $view
 *   The view object
 */
function _draggableviews_get_form_element($value, &$field, &$info, $attributes = array()) {
  if (!isset($attributes['field_name'])) $attributes['field_name'] = $field['field_name'];
  if (!isset($attributes['class'])) $attributes['class'] = $field['field_name'];
/*
  $type = content_types($node->type);
  $field = $type['fields'][$field_name];
  $field_types = _content_field_types();
  $field_type = $field_types[$field['type']];
  $widget_types = _content_widget_types();
  $widget_type = $widget_types[$field['widget']['type']];
*/
  switch ($field['widget']['type']) {
  
    case 'optionwidgets_select':
    
      return array(
        '#type'  => 'select',
        '#name'  => $attributes['field_name'],
        '#value' => $value,
        // slice out the first and the last element, so we make it possible
        // that new nodes may appear on the very top or the very bottom
        '#options' => array_slice(content_allowed_values($field), 1, -1, TRUE),
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

/*
 * Set cck fields in a node with a specific field-type
 * 
 * @param $node
 *   The node which contains the cck fields
 * @param $field_name
 *   The cck_field_name that should be set
 * @param $field_type
 *   The field-type of the cck field
 * @param $value
 */
function _draggableviews_node_set_value(&$node, $field_name, $field_type, $value) {

  // get field
  if (isset($node->$field_name)) {
    $field = &$node->$field_name;
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
  switch ($field_type) {
    case 'nodereference':
      $field[0]['nid'] = $value;
      break;
    
    default:
    case 'number_integer':
      $field[0]['value'] = $value;
      break;
  }
}

/*
 * get the minimum allowed value of a cck-field  
 * 
 * @param $field
 *   the cck fields
 */
function _draggableviews_field_get_minimum_value($field) {
  // get allowed values and return first element found
  $allowed_values = each(content_allowed_values($field));
  
  return $allowed_values['key'];
}

/*
 * filter handlers by type  
 * 
 * @param $type
 *   the field type
 * @param $fields
 *   the fields array
 * return
 *   the available fields
 */
function _draggableviews_filter_fields($types = array(), $handlers) {

  $available_fields = array();
  
  foreach ($handlers as $field => $handler) {
    $available = FALSE;
    // search given type in content-field-type
    if (isset($handler->content_field)) {
      foreach ($types AS $type) {
        if (ereg($type, $handler->content_field['type'])) {
          $available = TRUE;
          break;
        }
      }
    
      if ($available) {
        if ($label = $handler->label()) {
          $available_fields[$field] = $label;
        }
        else {
          $available_fields[$field] = $handler->ui_name();
        }
      }
    }
  }
  return $available_fields;
}

/*
 * Implementing hook_views_pre_view
 */
/*
function draggableviews_views_pre_view($view, $display_id, $view_args){
  // check  if view uses our style plugin
  if( $view->display[$display_id]->handler->get_option('style_plugin') == 'draggabletable' ){
  	
    // Add the node id field if it's not yet present.
    if( !array_key_exists('nid', $view->display[$display_id]->display_options['fields']) ){
      $view->add_item($display_id, 'field', 'node', 'nid');
    }
  }
}*/
