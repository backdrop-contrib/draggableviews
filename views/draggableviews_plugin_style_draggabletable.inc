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
    
    $options        = _draggableviews_filter_fields(array('number'), $handlers);
    $draggableviews_handlers = draggableviews_get_handlers_list();
    
    $first_option   = each($options);
    $first_handler  = each($draggableviews_handlers);
      
      
    // Build order fields
    // These fields will be used to store the weight of each hierarchy level
    $extra_row = isset($input['tabledrag_order_add']) ? TRUE : FALSE;
    for ($i = 0, $index = 0;
         $i < count($current['tabledrag_order']) + ($extra_row == TRUE ? 1 : 0);
         $i++) {
         
      // if option should be deleted, continue loop
      if (isset($input['tabledrag_order_del_'. $i])) continue;
      
      $form['tabledrag_order'][$index] = array(
        'field' => array(
          '#type' => 'select',
          '#options' => $options,
          '#value' => $current['tabledrag_order'][$i]['field'] ? $current['tabledrag_order'][$i]['field'] : $first_option['key'],
        ),
        'handler' => array(
          '#type' => 'select',
          '#options' => $draggableviews_handlers,
          '#value' => $current['tabledrag_order'][$i]['handler'] ? $current['tabledrag_order'][$i]['handler'] : $first_handler['key'],
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
    
    // TABLEDRAG SHOW/HIDE INPUT FIELDS
    // Build checkbox
    // decide whether order input fields should be visible or not
    $form['tabledrag_order_visible'] = array(
        '#type' => 'checkboxes',
        '#name' => 'tabledrag_order_visible',
        '#options' => array('visible' => 'Show input fields?'),
        '#title' => t('Decide whether order input fields (e.g. weight, parent) should be visible or not'),
        '#default_value' => isset($current['tabledrag_order_visible']) ? $current['tabledrag_order_visible'] : array(),
    );
    
    $form['tabledrag_hierarchy'] = array(
      '#weight' => 10,
      'field' => array(
        '#type' => 'select',
        '#options' => array('none' => 'none') + _draggableviews_filter_fields(array('nodereference'), $handlers),
        '#value' => isset($current['tabledrag_hierarchy']) ? $current['tabledrag_hierarchy']['field'] : 'none',
      ),
      'handler' => array(
        '#type' => 'select',
        '#options' => $draggableviews_handlers,
        '#value' => $current['tabledrag_order'][$i]['handler'] ? $current['tabledrag_order'][$i]['handler'] : $first_handler['key'],
      ),
    );
    
    // TABLEDRAG SHOW/HIDE INPUT FIELDS
    // Build checkbox
    // decide whether parent input fields should be visible or not
    $form['tabledrag_hierarchy_visible'] = array(
        '#type' => 'checkboxes',
        '#name' => 'tabledrag_hierarchy_visible',
        '#options' => array('visible' => 'Show input fields?'),
        '#title' => t('Decide whether parent input fields (e.g. weight, parent) should be visible or not'),
        '#default_value' => isset($current['tabledrag_hierarchy_visible']) ? $current['tabledrag_hierarchy_visible'] : array(),
    );
    
    // TABLEDRAG TYPES (ROOT,LEAF)
    // These fields will save the behaviour of a node-type (root, leaf)
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
    
    
    // Let extension modules alter the output
    foreach (module_implements('draggableviews_style_plugin_form_alter') as $module) {
      $function = $module . '_draggableviews_style_plugin_form_alter';
      $function($form, $form_state, $this);
    }
  }

  /**
   * Render the draggable table style.
   */
  function render() {
    // call form handler to wrap around a form.
    // -> this makes it possible to submit changes.
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
  
  // Let extension modules append to the output
  $extensions = array();
  foreach (module_implements('draggableviews_style_plugin_render') as $module) {
    $function = $module . '_draggableviews_style_plugin_render';
    $extensions[] = $function($form);
  }
  
  // append form data left to output
  // all items that have not been rendered until this point
  // will be rendered now. So we can seperate the settings we
  // inherited from plugin_table from plugin_draggableviews settings.
  $output .= drupal_render($form);
  
  // append order settings
  $output .= theme('table', array('&nbsp;', t('Field'), t('Handler'), '&nbsp;'), $tabledrag_order_rows);
  $output .= $tabledrag_order_add;
  $output .= $tabledrag_order_visible;
  // append hierarchy settings
  $output .= $tabledrag_hierarchy;
  $output .= $submit_button;
  $output .= $description_hierarchy;
  $output .= $tabledrag_hierarchy_visible;
  // append tabledrag types settings
  $output .= theme('table', array(t('Node type'), t('Type'), '&nbsp;'), $tabledrag_types_rows);
  $output .= $tabledrag_types_add;
  $output .= $description_types;
  // append expand yes/no checkbox to output
  $output .= $tabledrag_expand;
  $output .= $tabledrag_expand_default;
  // append extension output
  $output .= implode('', $extensions);
  
  // return output
  return $output;
}
