<?php
// $Id$

/**
 * @file
 * Implements preprocess function hook_submit for draggable views
 */
function draggableviews_view_draggabletable_form($form_state, $view){
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function template_preprocess_draggableviews_view_draggabletable_form($vars){
  // get view object
  $viewObj=$vars['form']['#parameters'][2];
  
  // add javascript
  // BE AWARE: realtimeedit module js has to be loaded first,
  // generated links should be appended in the right order
  
  //include realtimeedit module
  if(module_exists('realtimeedit')){ // check if module exists
    drupal_add_js(drupal_get_path('module', 'realtimeedit').'/realtimeedit.js');
    drupal_add_css(drupal_get_path('module', 'realtimeedit').'/styles.css');
  }
  // check if "expand" links should be shown
  if( $viewObj->options['tabledrag_expand'] == 1 ){
    drupal_add_js(drupal_get_path('module', 'draggableviews').'/draggableviews.js');
    drupal_add_css(drupal_get_path('module', 'draggableviews').'/styles.css');
  }
  
  
  //theme view
  $sets = $viewObj->render_grouping($viewObj->view->result, $viewObj->options['grouping']);
  
  $output = '';
  foreach ($sets as $title => $records) {
    $output .= theme($viewObj->theme_functions(), $viewObj->view, $viewObj->options, $records, $title);
  }
  
  $vars['view'] = $output;

  
  //render submit form
  $vars['submit_form'] = drupal_render($vars['form']);
}

/**
 * Implementing hook_submit 
 */
function draggableviews_view_draggabletable_form_submit($vars){
  //check permissions
  if(!user_access('administer nodes')){
    drupal_set_message(t('You are not allowed to edit/reorder these nodes.'), 'error');
    return;
  }

  $viewObj = $vars['#parameters'][2]; // get view object
  $results = $viewObj->view->result;  // get results
  $fields  = $viewObj->view->field;   // get fields
  $input = $vars['submit']['#post'];  // get input
  
  // get style options
  $style_options=$viewObj->view->display['default']->display_options['style_options'];
  // check for available tabledrag options
  if( ! isset($style_options['tabledrag_options']) ){
    // if there are no tabledrag options defined
    // we have finished -> return
    return;
  }
  // get tabledrag options
  $tabledrag_options = $style_options['tabledrag_options'];

  // some variables
  $match_used = false; // need to check if tabledrag{action:match;relationship:parent) is beeing used
  
  //find out if {action:match,relationship:parent} is used
  foreach($tabledrag_options as $tabledrag){
    if( $tabledrag['tabledrag_action'] == 'match' &&    // check if tabledrag{action:match}
        $tabledrag['tabledrag_relationship']=='parent') // check if tabledrag{relationship:sibling}
    {
      $match_used = true; // notice that action:math is beeing used
      
      // get name of field where the parent id is saved in
      // (In case of tabledrag{action:match,relationship:parent} the tabledrag_source field
      //  describes the field that contains the parent node)
      $tabledrag_parent_field_name = $tabledrag['tabledrag_source'];
      
      // get actual field name
      $tabledrag_real_parent_field_name = $fields[$tabledrag_parent_field_name]->content_field['field_name'];
    }
  }
  
  // iterate through all tabledrag options
  foreach($tabledrag_options as $tabledrag){
  
    //get content field
  	$content_field = $fields[$tabledrag['tabledrag_source']]->content_field;
  	
    $real_field_name   = $content_field['field_name']; // get actual field name
    $field_type        = $content_field['type'];       // get actual field type
    
    //go through all rows resulting from the view
    foreach($results as $res){
      
      //load node
      $node = node_load(array('nid' => $res->nid));
      
      /* calculate depth of current node *
       ***********************************/
      // set depth
      $depth = 0;
      // if {action:match,relationship:parent} is present
      // and current table drag is neither action:match nor relationship:parent
      if($match_used &&
         $tabledrag['tabledrag_action'] != 'match' &&
         $tabledrag['tabledrag_relationship'] != 'parent')
      {
      	
        $tmpNid = $res->nid; // use a temporary node id
        $depth  = -1;        // set depth
        
        // check if nid of current node exists
      	while($tmpNid > 0){
            // increase depth
            $depth++;
          
      	  // prepare new node id (use parent node id) for the next loop cycle
          $tmpNid = $input[$tabledrag_parent_field_name . '_' . $tmpNid];
          
          // end of loop
      	}
      }
      
      //if current node's depth == 0 skip the following section
      if($depth > 0){
        /* assign weights to the specified depth-weight-fields, *
         * depending on their depth
         ********************************************************/
      	
      	// use a temporary node id
        $tmpNid = $res->nid;
        
        //iterate down to the root level
        for($i = $depth; $i >= 0; $i--){
          /* Start in the deepest level.
             The weight of the deepest level (level=depth) will be set to the submitted weight of the current node.
             The weight of the level above (level=depth-1) will be set to the weight of its parent
             The weight of the level       (level=depth-2) will be set to the parent's parent node...
             ...
             Due to the fact that we iterate TOP-DOWN we are able to collect the parent's weights by the way.
             
             Unused depth-weight-fields will be set to the minimum value. Because of this convention
             we can assure that the rows will be shown in the expected order.
             (..provided the accurate sort-criterias are set)
          */
          
          // check if desired weight field exists
          if( isset($style_options['tabledrag_depth_fields']['field_'.$i]) ){
            // set field of level $i
            
            // build field key
            $tmp_field_key = $tabledrag['tabledrag_source'] . '_' . $tmpNid;
            // get submitted weight value
            $weight        = $input[$tmp_field_key];
            
            // get field name
            $tmp_field_name      = $style_options['tabledrag_depth_fields']['field_' . $i];
            // get actual field name
            $tmp_real_field_name = $fields[$tmp_field_name]->content_field['field_name'];
            // get field type
            $tmp_field_type      = $fields[$tmp_field_name]->content_field['type'];
            
            // write new weight to node
            _draggableviews_node_set_value(&$node, $tmp_real_field_name, $tmp_field_type, $weight);
          }
          
          // prepare new node id (use parent node id) for the next loop cycle
          $tmpNid = $input[$tabledrag_parent_field_name . '_' . $tmpNid];
          
          //end of loop
        }
        
      }else{
        /* if no {action:match,relationship:parent} defined
           OR depth == 0
           we don't need to set multiple weights of certain levels
           So we just have to save the submitted weight
        */
      	
        // build field key
        $tmp_field_key = $tabledrag['tabledrag_source'] . '_' . $res->nid;
        // get submitted weight value
        $value         = $input[$tmp_field_key];
        
        // set new weight to node
        // ($real_field_name and $field_type defined above)
        _draggableviews_node_set_value(&$node, $real_field_name, $field_type, $value);
        
        
        // check if tabledrag{action:match,relationship:parent} is not present
        if( ! ($tabledrag['tabledrag_action'] == 'match' || $tabledrag['tabledrag_relationship'] == 'parent') ){
          /* Set all unused depth-weight-fields to the minimum value.
             This should prevent that a parent appears under its child
             (because parent and child have the same depth-weight on the parents level)
          */
        
          for($i = count($style_options['tabledrag_depth_fields']) - 1; $i > 0; $i--) {
            // get field key
            $tmp_field_key       = $style_options['tabledrag_depth_fields']['field_' . $i];
            // get actual field name
            $tmp_real_field_name = $fields[$tmp_field_key]->content_field['field_name'];
            // get field type
            $tmp_field_type      = $fields[$tmp_field_key]->content_field['type'];
            
            // get minimum
            $minimum_value = _draggableviews_field_get_minimum_value($fields[$tmp_field_key]);
            
            // write minimum to node
            _draggableviews_node_set_value(&$node, $tmp_real_field_name, $tmp_field_type, $minimum_value - 1);
          }
        }
      }
      
      // finally save node
      if($node = node_submit($node)){
        $og_groups = $node->og_groups;
        $nid = $node->nid;
        
        node_save($node); // CAUTION, BUG: node_save destroys og_groups array
        
        // assing og groups manually
        if(isset($og_groups) && is_array($og_groups)){
          $sql = "INSERT INTO {og_ancestry} (nid, group_nid, is_public) VALUES (%d, %d, 0)";
          foreach( $og_groups AS $gid ){
            db_query($sql, $node->nid, $gid);
          }
        }
      }
    }
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
function _draggableviews_node_set_value(&$node, $field_name, $field_type, $value){
  // get field
  $field = &$node->$field_name;
  
  //differ between certain field types
  switch($field_type){
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
function _draggableviews_field_get_minimum_value($field){
  // check if min value is present
  if( $field->content_field['min'] ){
    // return min value
    return $field->content_field['min'];
  }
  
  // else check if allowed values are present
  elseif( $field->content_field['allowed_values'] ){
  	// return first element of array
    return $field->content_field['allowed_values'][0];
  }
  	
  // else check if PHP code is present
  elseif( $field->content_field['allowed_values_php'] ){
    // evaluate code to get values array
    $values = eval($field->content_field['allowed_values_php']);
    
    //return first element of array
    $value = each($values);
    return $value['key'];
  }
  
  // if there are no values available -> use -100 as default
  return -100;
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
?>