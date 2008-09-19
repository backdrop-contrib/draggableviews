<?php
// $Id$

/**
 * @file
 * Draggableviews style plugin definition.
 */

/**
 * Style plugin to render each item as a row in a draggable table;
 * Inherits all from views_plugin_table.
 */
class draggableviews_plugin_style_draggabletable extends views_plugin_style_table {
  /**
   * Render the given style.
   */
  function options_form(&$form, &$form_state) {
    // inherit options from style_table
    parent::options_form($form, $form_state);
    
    // set theme handler
    // theme function is registered in *.module file
    $form['#theme'] = 'draggableviews_ui_style_plugin_draggabletable';
    
    // get field handler
    $handlers = $this->display->handler->get_handlers('field');

    
    // DRAGGABLE VIEW OPTIONS
    // 
    // + Set field(s) to save the order in
    // + Set field to save the parent in
    // + Apply tabledrag-type to content-types (Root (can't have parent), Leaf (can't have children)).
    // + Set if expand/collapse links should be shown
    // + Provide realtimeedit integration
    
    // get display object
    $display_obj = $this->view->display['default'];
    
    $input = array();
    $input = $form_state['input'];
    
    // get all system-wide node types as a keyed array
    foreach (node_get_types('types') AS $node_type) {
      $node_types[$node_type->type] = $node_type->type;
    }
    
    $form['tabledrag_hierarchy'] = array();
    $form['tabledrag_order']     = array();
    $form['tabledrag_types']     = array();
    
    // check for input
    if (count($input) > 0) {
      // define the input data as the current data
      $current = $form_state['input']['style_options'];
    }
    else {
      // define the already stored data as the current data
      $current = $display_obj->display_options['style_options'];
    }
    
    $form['tabledrag_header'] = array(
      '#prefix' => '<div class="">',
      '#suffix' => '</div>',
      '#value' => t('Draggable Table Settings:'),
    );
    $form['tabledrag_desription'] = array(
      '#prefix' => '<div class="description form-item">',
      '#suffix' => '</div>',
      '#value' => t('Specify..'),
    );
    $form['tabledrag_description_types'] = array(
      '#prefix' => '<div class="description form-item">',
      '#suffix' => '</div>',
      '#value' => t('Type "root" cannot be subordinated.<br>Type "leaf" cannot have child nodes.<br>Content types with type "root" assigned will own a link to collapse/expand the subordinated nodes (if checked below).'),
    );
    $form['tabledrag_description_hierarchy'] = array(
      '#prefix' => '<div class="description form-item">',
      '#suffix' => '</div>',
      '#value' => t('Choose a nodereference CCK-field if you want to use hierarchies.'),
    );
    
    /* Build order fields
     * These fields will be used to store the weight of each hierarchy level
     */
    $extra_row = isset($input['tabledrag_order_add']) ? TRUE : FALSE;
    for ($i = 0, $index = 0;
         $i < count($current['tabledrag_order']) + ($extra_row == TRUE ? 1 : 0);
         $i++) {
         
      // if option should be deleted, continue loop
      if (isset($input['tabledrag_order_del_'. $i])) continue;
      
      $form['tabledrag_order'][$index] = array(
        'field' => array(
          '#type' => 'select',
          '#options' => array('none' => 'none') + _draggableviews_filter_fields(array('number'), $handlers),
          '#value' => $current['tabledrag_order'][$i]['field'] ? $current['tabledrag_order'][$i]['field'] : 'none',
        ),
        'del' => array(
          '#type' => 'button',
          '#name' => 'tabledrag_order_del_'. $index,
          '#value' => t('Delete'),
        ),
      );
      $index++;
    }
    $form['tabledrag_order_add'] = array(
        '#type' => 'button',
        '#name' => 'tabledrag_order_add',
        '#value' => t('Add order field'),
    );
    
    /* TABLEDRAG SHOW/HIDE INPUT FIELDS
     * Build checkbox
     * decide whether order input fields should be visible or not
     */
    $form['tabledrag_order_visible'] = array(
        '#type' => 'checkboxes',
        '#name' => 'tabledrag_order_visible',
        '#options' => array('visible' => 'Show input fields?'),
        '#title' => t('Decide whether order input fields (e.g. weight, parent) should be visible or not'),
        '#default_value' => isset($current['tabledrag_order_visible']) ? $current['tabledrag_order_visible'] : array(),
    );
    
    $form['tabledrag_hierarchy'] = array(
      '#weight' => 10,
      'parent_field' => array(
        '#type' => 'select',
        '#options' => array('none' => 'none') + _draggableviews_filter_fields(array('nodereference'), $handlers),
        '#value' => isset($current['tabledrag_hierarchy']) ? $current['tabledrag_hierarchy']['parent_field'] : 'none',
      ),
    );
    
    /* TABLEDRAG SHOW/HIDE INPUT FIELDS
     * Build checkbox
     * decide whether parent input fields should be visible or not
     */
    $form['tabledrag_hierarchy_visible'] = array(
        '#type' => 'checkboxes',
        '#name' => 'tabledrag_hierarchy_visible',
        '#options' => array('visible' => 'Show input fields?'),
        '#title' => t('Decide whether parent input fields (e.g. weight, parent) should be visible or not'),
        '#default_value' => isset($current['tabledrag_hierarchy_visible']) ? $current['tabledrag_hierarchy_visible'] : array(),
    );
    
    /* TABLEDRAG TYPES (ROOT,LEAF)
     * These fields will save the behaviour of a node-type (root, leaf)
     */
    $extra_row = isset($input['tabledrag_types_add']) ? TRUE : FALSE;
    for ($i = 0, $index = 0;
        $i < count($current['tabledrag_types']) + ($extra_row == TRUE ? 1 : 0);
        $i++) {
        
      // if option should be deleted, continue loop
      if (isset($input['tabledrag_types_del_'. $i])) continue;
      
      $form['tabledrag_types'][$index] = array(
        'node_type' => array(
          '#type' => 'select',
          '#options' => $node_types,
          '#value' => $current['tabledrag_types'][$i]['node_type'] ? $current['tabledrag_types'][$i]['node_type'] : $node_types[0],
        ),
        'type' => array(
          '#type' => 'select',
          '#options' => array('root' => 'root', 'leaf' => 'leaf'),
          '#value' => $current['tabledrag_types'][$i]['type'] ? $current['tabledrag_types'][$i]['type'] : $node_types[0],
        ),
        'tabledrag_type_del_button' => array(
          '#type' => 'button',
          '#name' => 'tabledrag_types_del_'. $index,
          '#value' => t('Delete'),
        ),
      );
      $index++;
    }
    $form['tabledrag_types_add'] = array(
        '#type' => 'button',
        '#name' => 'tabledrag_types_add',
        '#value' => t('Add type'),
    );
    
    /* TABLEDRAG EXPAND/COLLAPSE OPTION
     * Build checkbox
     * decide if epand links should be shown
     */
    $form['tabledrag_expand'] = array(
        '#type' => 'checkboxes',
        '#name' => 'tabledrag_expand',
        '#options' => array('expand_links' => 'Show expand Links?', 'collapsed' => 'default is collapsed'),
        '#title' => t('Decide whether expand/collapse Links should be shown or not'),
        '#default_value' => isset($current['tabledrag_expand']) ? $current['tabledrag_expand'] : array('expand_links'),
    );
    
    
    /*****************************************
     * IMPLEMENTATION OF REALTIMEEDIT MODULE *
    ******************************************/
    
    //check if realtimeedit module exists
    if (module_exists('realtimeedit')) {
      // set tabledrag types (root, leaf) for specific node-types
      $form['realtimeedit_enabled_fields'] = array();
      $leave_ndex_realtimeedit = -1; // index of tabledrag types that should not be left out
                                     // (..to delete a specific tabledrag type)
      $extra_rows_realtimeedit = 0;  // new tabledrag-type-rows to add at end of array
      
      // REALTIMEEDIT ENABLED FIELDS
      /* Build realtimeedit enabled fields
       ***************************************
         These fields will be realtime-editable
      */
      $max_count = count($current['realtimeedit_enabled_fields']) + $extra_rows_realtimeedit;
      $button_index = 0; // initalize count variable and button index
      for ($i = 0; $i < $max_count; $i++) {
      
        // if realtimeedit enabled field should be left out, continue loop
        // (delete enabled field from array)
        if ($i == $leave_ndex_realtimeedit) continue;
        
        // if current enabled field is an extra row,
        // don't use existing data to fill form-elements (because they are not existing yet)
        $overwrite = $i < count($current['realtimeedit_enabled_fields']) ? TRUE : FALSE;
      
        //get options of current enabled field
        $tmp_options = $current['realtimeedit_enabled_fields'][$i];
      
        $form['realtimeedit_enabled_fields'][] = array(
          '#weight' => 10,
          'field' => array(
            '#type' => 'select',
            '#options' => $available_fields,
            '#value' => $overwrite ? $tmp_options['field'] : $available_fields[0],
          ),
          'realtimeedit_del_button' => array(
            '#type' => 'button',
            '#name' => 'realtimeedit_del_button_'. $button_index,
            '#value' => t('Delete'),
          ),
        );
      
        // increase button index
        $button_index++;
      }
    
      $form['realtimeedit_add_button'] = array(
        '#type' => 'button',
        '#name' => 'realtimeedit_add_button',
        '#value' => t('Enable new field'),
      );
    }
  }

  /**
   * Render the table style.
   */
  function render() {
    /* call form handler to wrap around a form.
     * -> this makes it possible to submit changes.
     */
    return drupal_get_form('draggableviews_view_draggabletable_form', $this);
  }
}

/**
 * Theme the form for the table style plugin
 */
function theme_draggableviews_ui_style_plugin_draggabletable($form) {
  // the following lines are copied from the table style plugin
  /* COPY BEGIN */
  $output = drupal_render($form['description_markup']);

  $header = array(
    t('Field'),
    t('Column'),
    t('Separator'),
    array(
      'data' => t('Sortable'),
      'align' => 'center',
    ),
    array(
      'data' => t('Default sort'),
      'align' => 'center',
    ),
  );
  $rows = array();
  foreach (element_children($form['columns']) as $id) {
    $row = array();
    $row[] = drupal_render($form['info'][$id]['name']);
    $row[] = drupal_render($form['columns'][$id]);
    $row[] = drupal_render($form['info'][$id]['separator']);
    if (!empty($form['info'][$id]['sortable'])) {
      $row[] = array(
        'data' => drupal_render($form['info'][$id]['sortable']),
        'align' => 'center',
      );
      $row[] = array(
        'data' => drupal_render($form['default'][$id]),
        'align' => 'center',
      );
    }
    else {
      $row[] = '';
      $row[] = '';
    }
    $rows[] = $row;
  }

  // Add the special 'None' row.
  $rows[] = array(t('None'), '', '', '', array('align' => 'center', 'data' => drupal_render($form['default'][-1])));

  $output .= theme('table', $header, $rows);
  /* COPY END */
  
  
  /* Render Draggable view settings
   * The following lines add the new output from draggableviews
   */
  
  // build order table
  $tabledrag_order_rows = array();
  $i = 1;
  foreach (element_children($form['tabledrag_order']) AS $id) {
    $columns = array();
    $columns[] = 'Level '. $i++;
    foreach (element_children($form['tabledrag_order'][$id]) AS $col) {
      $columns[] = drupal_render($form['tabledrag_order'][$id][$col]);
    }
    $tabledrag_order_rows[] = $columns;
  }
  $tabledrag_order_add = drupal_render($form['tabledrag_order_add']);
  $tabledrag_order_visible = drupal_render($form['tabledrag_order_visible']);
  
  $tabledrag_hierarchy = drupal_render($form['tabledrag_hierarchy']);
  $tabledrag_hierarchy_visible = drupal_render($form['tabledrag_hierarchy_visible']);
  $description_hierarchy = drupal_render($form['tabledrag_description_hierarchy']);
  
  // build types table
  $tabledrag_types_rows = array();
  foreach (element_children($form['tabledrag_types']) AS $id) {
    $columns = array();
    foreach (element_children($form['tabledrag_types'][$id]) AS $col) {
      $columns[] = drupal_render($form['tabledrag_types'][$id][$col]);
    }
    $tabledrag_types_rows[] = $columns;
  }
  $tabledrag_types_add = drupal_render($form['tabledrag_types_add']);
  $description_types   = drupal_render($form['tabledrag_description_types']);
  
  $tabledrag_expand  = drupal_render($form['tabledrag_expand']);
  
  if (module_exists('realtimeedit')) {
    // Implementing realtimeedit module
    $realtimeedit_enabled_add = drupal_render($form['realtimeedit_enabled_add']);
    
    foreach (element_children($form['realtimeedit_enabled_fields']) AS $id) {
      $columns = array();
      foreach (element_children($form['realtimeedit_enabled_fields'][$id]) AS $col) {
        $columns[] = drupal_render($form['realtimeedit_enabled_fields'][$id][$col]);
      }
      $realtimeedit_enabled_rows[] = $columns;
    }
  }
  
  // add form data left to output
  $output .= drupal_render($form);
  
  
  // add specific output
  $output .= theme('table', array('&nbsp;', t('Field'), '&nbsp;'), $tabledrag_order_rows);
  $output .= $tabledrag_order_add;
  $output .= $tabledrag_order_visible;
  
  $output .= $tabledrag_hierarchy;
  $output .= $submit_button;
  $output .= $description_hierarchy;
  $output .= $tabledrag_hierarchy_visible;
  
  $output .= theme('table', array(t('Node type'), t('Type'), '&nbsp;'), $tabledrag_types_rows);
  $output .= $tabledrag_types_add;
  $output .= $description_types;
  
  // add expand yes/no checkbox to output
  $output .= $tabledrag_expand;
  $output .= $tabledrag_expand_default;
  
  
  if (module_exists('realtimeedit')) {
    // Implementing realtimeedit module
  
    // add editable fields table to output
    $output .= theme('table', array('', ''), $realtimeedit_enabled_rows);
    // add button
    $output .= $realtimeedit_enabled_add;
  }
  
  // return output
  return $output;
}