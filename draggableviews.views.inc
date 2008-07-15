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
        'uses row plugin' => TRUE,
        'uses options' => TRUE,
        'uses fields' => TRUE,
        'type' => 'normal',
      ),
    ),
  );
  }
  
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

    
    /**************************
     * DRAGGABLE VIEW OPTIONS *
     **************************
       + Set tabledrag options (multiple).
       + Set depth and weight-fields of each depth.
       + Apply tabledrag-type to content-types (Root (can't have parent), Leaf (can't have children)).
    */
    
    // get display object
    $displayObj = $this->view->display['default'];
    // define the saved data in the view object as the current data
    $current = $displayObj->display_options['style_options'];
    
    // Create an array of allowed columns from the data we know:
    foreach ($handlers as $field => $handler) {
      if ($label = $handler->label()) {
        $available_fields[$field] = $label;
      } else {
        $available_fields[$field] = $handler->ui_name();
      }
    }
    
    // get all system-wide node types as a keyed array
    foreach(node_get_types('types') AS $node_type){     /* get value for each */
      $node_types[$node_type->type] = $node_type->type; /* build new array */
    }
    
    $form['tabledrag_options'] = array();      // set tabledrag options
    $form['tabledrag_depth_fields'] = array(); // set depth fields (hierarchy weights) where weights will be saved
    $form['tabledrag_types'] = array();        // set tabledrag types (root, leaf) for specific node-types
    
    // define some variables
    $leaveIndex = -1;      // index of tabledrag data that should not be left out
                           // (..to delete a specific tabledrag data row)
    $leaveIndex_type = -1; // index of tabledrag types that should not be left out
                           // (..to delete a specific tabledrag type)
    $extra_rows = 0;       // new tabledrag-data-rows to add at end of array
    $extra_rows_type = 0;  // new tabledrag-type-rows to add at end of array
    //check if realtimeedit module exists
    if(module_exists('realtimeedit')){
      // set tabledrag types (root, leaf) for specific node-types
      $form['realtimeedit_enabled_fields'] = array();
      $leaveIndex_realtimeedit = -1; // index of tabledrag types that should not be left out
      $extra_rows_realtimeedit = 0;  // new tabledrag-type-rows to add at end of array
    }
    
    // check for input
    if( count($form_state['input']) > 0 ) {
      // Input is available (form has been submitted) ->
      
      // React on clicked buttons:
      // Button: add action
      if( isset( $form_state['input']['tabledrag_action_add_button'] ) ){
      	$extra_rows = 1; // 1 extra row should be built
      }
      // Buttons: check all tabledrag delete buttons
      for($i = 0; $i < count( $form_state['input']['style_options']['tabledrag_options'] ); $i++){
        // Button: delete action
        if( isset( $form_state['input']['tabledrag_action_del_button_' . $i] ) ){
          $leaveIndex = $i; // set tabledrag index to delete
        }
      }
      
      // Button: add tabledrag type
      if(isset($form_state['input']['tabledrag_type_add_button'])){
      	  $extra_rows_type = 1;
      }
      // Buttons: check all delete buttons
      for($i = 0; $i < count( $form_state['input']['style_options']['tabledrag_types'] ); $i++){
        // Button: delete action
        if( isset( $form_state['input']['tabledrag_type_del_button_' . $i] ) ){
          $leaveIndex_type = $i; // set tabledrag type index to delete
        }
      }
      
      //check if realtimeedit module exists
      if(module_exists('realtimeedit')){
        // Button: add realtimeedit enabled field
        if(isset($form_state['input']['realtimeedit_add_button'])){
          $extra_rows_realtimeedit = 1;
        }
        // Buttons: check all delete buttons
        for($i = 0; $i < count( $form_state['input']['style_options']['realtimeedit_enabled_fields'] ); $i++){
          // Button: delete action
          if( isset( $form_state['input']['realtimeedit_del_button_' . $i] ) ){
            $leaveIndex_realtimeedit = $i; // set tabledrag type index to delete
          }
        }
      }
      
      // define the input data as the current data
      $current = $form_state['input']['style_options'];
    }
    
    
    /* Some markup to introduce DRAGGABLE VIEW form elements
     *******************************************************
    */
    $form['head_draggabletable_markup'] = array(
      '#prefix' => '<div class="">',
      '#suffix' => '</div>',
      '#value' => t('Draggable Table Settings:'),
    );
    $form['description_draggabletable_markup'] = array(
      '#prefix' => '<div class="description form-item">',
      '#suffix' => '</div>',
      '#value' => t('Draggable Table description.'),
    );
    $form['head_draggabletable_markup'] = array(
      '#prefix' => '<div class="">',
      '#suffix' => '</div>',
      '#value' => t('Add new tabledrag:'),
    );
    
    
    /* Build tabledrag actions form elements
     *******************************************************
    */
    
    // loop: go through all defined tabldrag options
    for($i = 0, $button_index = 0;                                 // initalize count variable and button index
        $i < count($current['tabledrag_options']) + $extra_rows;   // set break condition (+1 new extra row if desired)
        $i++)                                                      // increase index
    {
      // if tabldrag option should be left out, continue loop
      // (delete tabledrag option from array)
      if($i == $leaveIndex) continue;
      
      // if current table drag option is an extra row,
      // don't use existing data to fill form-elements (because they would not exist yet)
      $overwrite = $i < count($current['tabledrag_options']) ? true : false;
      
      if( $overwrite == true){
        //get options of current table drag
        $tmpOptions = $current['tabledrag_options'][$i];
      }
      
      //build form elements
      $form['tabledrag_options'][] = array(
        '#weight' => 10,
        'tabledrag_action' => array(
          '#type' => 'select',
          '#options' => array('match' => 'match', 'order' => 'order'),
          '#value' => $overwrite ? $tmpOptions['tabledrag_action'] : 'match',
        ),
        'tabledrag_relationship' => array(
          '#type' => 'select',
          '#options' => array('parent' => 'parent', 'sibling' => 'sibling'),
          '#value' => $overwrite ? $tmpOptions['tabledrag_relationship'] : 'parent',
        ),
        'tabledrag_group' => array(
          '#type' => 'select',
          '#options' => $available_fields,
          '#value' => $overwrite ? $tmpOptions['tabledrag_group'] : $available_fields[0],
        ),
        'tabledrag_subgroup' => array(
          '#type' => 'select',
          '#options' => $available_fields,
          '#value' => $overwrite ? $tmpOptions['tabledrag_subgroup'] : $available_fields[0],
        ),
        'tabledrag_source' => array(
          '#type' => 'select',
          '#options' => $available_fields,
          '#value' => $overwrite ? $tmpOptions['tabledrag_source'] : $available_fields[0],
        ),
        'tabledrag_action_del_button' => array(
          '#type' => 'button',
          '#name' => 'tabledrag_action_del_button_' . $button_index,
          '#value' => t('Delete'),
        ),
      );
      
      // increase button index
      $button_index++;
    }
    
    //build button to add a new tabldrag
    $form['tabledrag_action_add_button'] = array(
        '#type' => 'button',
        '#name' => 'tabledrag_action_add_button',
        '#value' => t('Add Tabledrag'),
    );
    
    
    /* Build tabledrag_depth fields
     ***************************************
       These fields will be used to store the weight of each hierarchy.
       This section will not be shown if tabledrag{action:oder,relationship:sibling} is not used
    */
    
    // get tabledrag depth
    $tabledrag_depth=isset($current['tabledrag_depth_fields']) ? $current['tabledrag_depth'] : 1;
    
    // loop: add a select for each depth
    for($i = 0; $i < $tabledrag_depth; $i++){
      // set current depth field
      $tmpDepthField = $current['tabledrag_depth_fields']['field_' . $i];
      
      //SELECT: choose field where weight of depth should be saved
      $form['tabledrag_depth_fields']['field_'.$i] = array(
        '#type' => 'select',
        '#options' => $available_fields,
        '#value' => $tmpDepthField ? $tmpDepthField : $available_fields[0],
      );
    }
    
    // Button: set depth
    $form['tabledrag_set_depth_button'] = array(
        '#type' => 'button',
        '#name' => 'button_set_depth',
        '#value' => t('Set depth'),
    );
    
    // SELECT: choose depth
    $form['tabledrag_depth'] = array(
      '#type' => 'select',
      '#options' => array(1 => 1, 2 => 2, 3 => 3, 4 => 4), //DUMMY OPTIONS
      '#default_value' => $tabledrag_depth,
    );
    
    //TABLEDRAG TYPES (ROOT,LEAF)
    /* Build tabledrag type fields
     ***************************************
       These fields will save the behaviour of a node-type (root, leaf)
    */
    
    for($i=0, $button_index=0;                                    // initalize count variable and button index
        $i<count($current['tabledrag_types']) + $extra_rows_type; // set break condition (+1 new extra row if desired)
        $i++)                                                     // increase index
    {
      
      // if tabldrag option should be left out, continue loop
      // (delete tabledrag option from array)
      if($i == $leaveIndex_type) continue;
      
      // if current table drag type is an extra row,
      // don't use existing data to fill form-elements (because they are not existing yet)
      $overwrite = $i < count($current['tabledrag_types']) ? true : false;
      
      //get options of current table drag
      $tmpOtions = $current['tabledrag_types'][$i];
      
      $form['tabledrag_types'][] = array(
        '#weight' => 10,
        'field_type' => array(
          '#type' => 'select',
          '#options' => $node_types,
          '#value' => $overwrite ? $tmpOtions['field_type'] : $node_types[0],
        ),
        'type' => array(
          '#type' => 'select',
          '#options' => array('root' => 'root', 'leaf' => 'leaf'),
          '#value' => $overwrite ? $tmpOtions['type'] : 'root',
        ),
        'tabledrag_type_del_button' => array(
          '#type' => 'button',
          '#name' => 'tabledrag_type_del_button_' . $button_index,
          '#value' => t('Delete'),
        ),
      );
      
      // increase button index
      $button_index++;
    }
    
    $form['tabledrag_type_add_button'] = array(
        '#type' => 'button',
        '#name' => 'tabledrag_type_add_button',
        '#value' => t('Add type'),
    );
    
    //TABLEDRAG EXPAND/COLLAPSE OPTION
    /* Build checkbox
     ***************************************
       decide if epand links should be shown
    */
    // @todo not yet ready for use
    /*
    $form['tabledrag_expand'] = array(
        '#type' => 'checkbox',
        '#name' => 'tabledrag_expand',
        '#title' => 'decide if expand links should be shown',
        '#value' => isset($current['tabledrag_expand']) ? $current['tabledrag_expand'] : 1,
    );*/
    
    
    /*****************************************
     * IMPLEMENTATION OF REALTIMEEDIT MODULE *
    ******************************************/
    
    //check if realtimeedit module exists
    if(module_exists('realtimeedit')){
      // set tabledrag types (root, leaf) for specific node-types
      $form['realtimeedit_enabled_fields'] = array();
      $leaveIndex_type = -1; // index of tabledrag types that should not be left out
                             // (..to delete a specific tabledrag type)
      $extra_rows_type = 0;  // new tabledrag-type-rows to add at end of array
      
      // REALTIMEEDIT ENABLED FIELDS
      /* Build realtimeedit enabled fields
       ***************************************
         These fields will be realtime-editable
      */
      $max_count = count($current['realtimeedit_enabled_fields']) + $extra_rows_realtimeedit;
      $button_index=0; // initalize count variable and button index
      for($i = 0; $i < $max_count; $i++) {
      
        // if realtimeedit enabled field should be left out, continue loop
        // (delete enabled field from array)
        if($i == $leaveIndex_realtimeedit) continue;
        
        // if current enabled field is an extra row,
        // don't use existing data to fill form-elements (because they are not existing yet)
        $overwrite = $i < count($current['realtimeedit_enabled_fields']) ? true : false;
      
        //get options of current enabled field
        $tmpOtions = $current['realtimeedit_enabled_fields'][$i];
      
        $form['realtimeedit_enabled_fields'][] = array(
          '#weight' => 10,
          'field' => array(
            '#type' => 'select',
            '#options' => $available_fields,
            '#value' => $overwrite ? $tmpOtions['field'] : $available_fields[0],
          ),
          'realtimeedit_del_button' => array(
            '#type' => 'button',
            '#name' => 'realtimeedit_del_button_' . $button_index,
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
    // call form handler to wrap around a form.
    // -> this makes it possible to submit order changes.
    return drupal_get_form('draggableviews_view_draggabletable_form', &$this);
    /*
    $sets = $this->render_grouping($this->view->result, $this->options['grouping']);

    $output = '';
    foreach ($sets as $title => $records) {
      $output .= theme($this->theme_functions(), $this->view, $this->options, $records, $title);
    }
    return $output;*/
  }
}

/**
 * Theme the form for the table style plugin
 */
function theme_draggableviews_ui_style_plugin_draggabletable($form) {
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
  
  
  
  /* Render Draggable view settings
   ********************************
  */
  
  // set labels
  $labels=array(t('Action'), t('Relationship'), t('Group'), t('Subgroup'), t('Source'), '&nbsp;');
  $labels_types=array(t('Content type'), t('Tabledrag type'), '&nbsp;');
  $labels_orderfields = array(t('Save order in..'));
  // initialize table rows
  $rows=array();
  $rows_types=array();
  $rows_orderfields = array();
  
  $submit_button = drupal_render($form['tabledrag_action_add_button']);   // render submit button
  $add_type_button = drupal_render($form['tabledrag_type_add_button']);   // render add type button
  $set_depth_button = drupal_render($form['tabledrag_set_depth_button']); // render set depth button
  $set_depth_select = drupal_render($form['tabledrag_depth']);            // render depth select element
  $tabledrag_expand = drupal_render($form['tabledrag_expand']);           // render expand link checkbox

  // prepare table data:
  
  // assign tabledrag options to an array
  foreach(element_children($form['tabledrag_options']) AS $id){
    $columns=&$form['tabledrag_options'][$id]; // get columns
    
    $tmpRow=array();
    foreach(element_children($columns) AS $el_id){
      // build row
      $tmpRow[] = drupal_render($columns[$el_id]);
    }
    $rows[]=$tmpRow; // append row to array
  }
  
  // assing tabledrag types to an array
  foreach(element_children($form['tabledrag_types']) AS $id){
    $columns=&$form['tabledrag_types'][$id]; // get columns
    
    $tmpRow=array();
    foreach(element_children($columns) AS $el_id){
      $tmpRow[] = drupal_render($columns[$el_id]);
    }
    $rows_types[]=$tmpRow; // append row to array
  }
  // assing tabledrag depth fields to an array
  foreach(element_children($form['tabledrag_depth_fields']) AS $id){
    $column=&$form['tabledrag_depth_fields'][$id]; // get column
    
    $rows_orderfields[] = array(drupal_render($column)); // append row to array
  }
  
  /* IMPLEMENTATION OF REALTIMEEDIT MODULE */
  //check if realtimeedit module exists
  if(module_exists('realtimeedit')){
    $add_realtimeedit_button = drupal_render($form['realtimeedit_add_button']); // render set depth button
    
    //set label
    $labels_realtimeeditfields = array(t('Enabled Fields'), '&nbsp;');
    
    // assing tabledrag depth fields to an array
    foreach(element_children($form['realtimeedit_enabled_fields']) AS $id){
      $columns=&$form['realtimeedit_enabled_fields'][$id]; // get columns
      
      $tmpRow=array();
      foreach(element_children($columns) AS $el_id){
        $tmpRow[] = drupal_render($columns[$el_id]);
      }
      $rows_realtimeedit_fields[]=$tmpRow; // append row to array
    }
  }
  
  // add form data left to output
  $output .= drupal_render($form);
  
  // add tabledrags to output
  $output .= theme('table', $labels, $rows);
  $output .= $submit_button;
  
  // add tabledrag types to output (choose if type = leaf|root)
  $output .= theme('table', $labels_types, $rows_types);
  $output .= $add_type_button;
  
  // add tabledrag order depth table to output
  $output .= theme('table', $labels_orderfields, $rows_orderfields);
  $output .= theme('table', array("",""), array( array($set_depth_select, $set_depth_button) ) );
  
  // add expand yes/no checkbox to output
  $output .= $tabledrag_expand;
  
  /* IMPLEMENTATION OF REALTIMEEDIT MODULE */
  //check if realtimeedit module exists
  if(module_exists('realtimeedit')){
    // add editable fields table to output
    $output .= theme('table', $labels_realtimeeditfields, $rows_realtimeedit_fields);
    // add button
    $output .= $add_realtimeedit_button;
  }
  
  // return output
  return $output;
}

/**
 * Display a view as a draggable table style.
 */
function template_preprocess_draggableviews_view_draggabletable(&$vars) {
  $view     = $vars['view'];

  // We need the raw data for this grouping, which is passed in as $vars['rows'].
  // However, the template also needs to use for the rendered fields.  We
  // therefore swap the raw data out to a new variable and reset $vars['rows']
  // so that it can get rebuilt.
  $result   = $vars['rows'];

  $vars['rows'] = array();

  $options  = $view->style_plugin->options;
  $handler  = $view->style_plugin;

  $fields   = &$view->field;
  $columns  = $handler->sanitize_columns($options['columns'], $fields);

  $active   = !empty($handler->active) ? $handler->active : '';
  $order    = !empty($handler->order) ? $handler->order : 'asc';

  $query    = tablesort_get_querystring();
  if ($query) {
    $query = '&' . $query;
  }

  foreach ($columns as $field => $column) {
    // render the header labels
    if ($field == $column && empty($fields[$field]->options['exclude'])) {
      $label = check_plain(!empty($fields[$field]) ? $fields[$field]->label() : '');
      if (empty($options['info'][$field]['sortable'])) {
        $vars['header'][$field] = $label;
      }
      else {
        // @todo -- make this a setting
        $initial = 'asc';

        if ($active == $field && $order == 'asc') {
          $initial = 'desc';
        }

        $image = theme('tablesort_indicator', $initial);
        $title = t('sort by @s', array('@s' => $label));
        $link_options = array(
          'html' => true,
          'attributes' => array('title' => $title),
          'query' => 'order=' . urlencode($field) . '&sort=' . $initial . $query,
        );
        $vars['header'][$field] = l($label . $image, $_GET['q'], $link_options);
      }
    }

    // Create a second variable so we can easily find what fields we have and what the
    // CSS classes should be.
    $vars['fields'][$field] = views_css_safe($field);
    if ($active == $field) {
      $vars['fields'][$field] .= ' active';
    }
    
    // Render each field into its appropriate column.
    foreach ($result as $num => $row) {
      if (!empty($fields[$field]) && empty($fields[$field]->options['exclude'])) {
        $field_output = $fields[$field]->theme($row);

        // Don't bother with separators and stuff if the field does not show up.
        if (!isset($field_output) && isset($vars['rows'][$num][$column])) {
          continue;
        }

        // Place the field into the column, along with an optional separator.
        if (isset($vars['rows'][$num][$column])) {
          if (!empty($options['info'][$column]['separator'])) {
            $vars['rows'][$num][$column] .= filter_xss_admin($options['info'][$column]['separator']);
          }
        }
        else {
          $vars['rows'][$num][$column] = '';
        }

        $vars['rows'][$num][$column] .= $field_output;
      }
    }
  }

  $vars['class'] = 'views-table';
  if (!empty($options['sticky'])) {
    drupal_add_js('misc/tableheader.js');
    $vars['class'] .= " sticky-enabled";
  }
  
  
  

  /*********************
   * TABLE_DRAG OUTPUT *
  **********************/
  
  // get style options
  $style_options = $vars['view']->display['default']->display_options['style_options'];
  // check for available tabledrag options
  if( ! isset($style_options['tabledrag_options']) ){
    // if there are no tabledrag options defined
    // we have finished -> return
    return;
  }
  // get tabledrag options
  $tabledrag_options = $style_options['tabledrag_options'];
  //get node type specifications
  $tabledrag_types = $style_options['tabledrag_types'];
  //get depth weight fields
  $tabledrag_depth_fields = $style_options['tabledrag_depth_fields'];
    
  // get node type sepcification
  $node_type_spec = array();
  if( count($tabledrag_types) > 0 ){
    foreach($tabledrag_types as $tabledrag_type){
      $node_type_spec[$tabledrag_type['field_type']] = $tabledrag_type['type'];
    }
  }
  
  // some variables
  $match_used = false; // need to check if tabledrag{action:match;relationship:parent) is beeing used

  // output data
  $vars['tabledrag']         = array();                        // tabledrag settings
  $vars['tabledrag_tableId'] = 'draggableview_' . $view->name; // set table id
  
  // prepare tabledrag settings for output
  foreach($tabledrag_options as $tabledrag){
    $vars['tabledrag'][] = array(
      'source' => $tabledrag['tabledrag_action']=='match' ? 'hidden_nid' : $tabledrag['tabledrag_source'],
      /* Tabledrag action:match uses the node id to refernce the parent node.
         The nid - if needed - will be provided as a hidden field. */
      'action' => $tabledrag['tabledrag_action'],
      'relationship' => $tabledrag['tabledrag_relationship'],
      'group' => $tabledrag['tabledrag_group'],
      'subgroup' => $tabledrag['tabledrag_subgroup'],
      'hidden' => TRUE, // @todo make this a setting
      'limit' => count($tabledrag_depth_fields)-1, // set limit to number of specified depth fields -1.
                                                   // It wouldn't make sense to allow a dragging to a level
                                                   // that cannot be saved. (0 would mean unlimited)
    );
    
    //find out if {action:match,relationship:parent} is used
    if( $tabledrag['tabledrag_action'] == 'match' &&    // check if tabledrag{action:match}
        $tabledrag['tabledrag_relationship']=='parent') // check if tabledrag{relationship:sibling}
    {
      $match_used = true; // notice that action:match is beeing used
      
      // get name of field where the parent id is saved in
      // (In case of tabledrag{action:match,relationship:parent} the tabledrag_source field
      //  describes the field that contains the parent node)
      $tabledrag_parent_field_name = $tabledrag['tabledrag_source'];
      
      // get field alias
      $tabledrag_parent_field_alias=$fields[$tabledrag_parent_field_name]->field_alias;
    }
  }
  
  /* Make selected fields editable
   *******************************
   */
  foreach($vars['rows'] as $num => $row){
    $node_id = $result[$num]->nid;                 // get node id
    $tmpNode = node_load(array('nid' => $node_id)); // load node
    
    // set node type specification, if available (such as 'tabledrag-root')
    if( isset($node_type_spec[$tmpNode->type]) ){
      $vars['tabledrag_type'][$num] = 'tabledrag-' . $node_type_spec[$tmpNode->type];
    }
    
    // get hierarchy depth of current row
    $depth = $match_used ? _draggableviews_get_hierarchy_depth($node_id, $result, $tabledrag_parent_field_alias) : 0;
    
    // build indentation (as drupal-tabledrag requires)
    $indentation = theme('indentation', $depth);
    
    // rebuild fields as form elements
    reset($tabledrag_options);
    foreach($tabledrag_options as $tabledrag){
      
      $field_name = $tabledrag['tabledrag_source']; // get name of field
      
      // get field alias
      // we need to determine the views field alias to get access to the fields value
      
      //unset field alias
      unset($field_alias);
      
      // if tabledrag{action:match;relationship:sibling} is present
      // or hierarchy depth = 0
      // always use tabledrag_source (specified in tabledrag-options)
      if( $depth == 0 ||
          ($tabledrag['tabledrag_action'] == 'match' || $tabledrag['tabledrag_relationship'] == 'parent') ){
          	
        // set field alias
        $field_alias = $fields[$field_name]->field_alias;
      }else{
        // else (tabledrag{action:sibling;relationship:order} is present)
        // use weight field of highest hierarchy level (as drupal-tabledrag requires)
        
      	// check if field exists
        if( isset($fields[$tabledrag_depth_fields['field_' . $depth]]->field_alias)){
          // set field alias
          $field_alias = $fields[$tabledrag_depth_fields['field_' . $depth]]->field_alias;
        }
      }
      
      // now we are able to get the value of the result..
      // ..by using the accurate field alias we determined before
      if( isset($field_alias) ){
        $value = $result[$num]->$field_alias;
      }else{
        // if field alias is empty
        
      }
      
      // get accurate form element depending on field type
      $form_element = _draggableviews_get_form_element($node_id, $field_name, $value, &$view);
      
      // render new form element
      $vars['rows'][$num][$field_name] = drupal_render($form_element);
    }
    
    // if required add node id field
    if($match_used == true){
      // build form element
      $hidden_nid = array(
        '#type' => 'hidden',
        '#name' => 'hidden_nid',
        '#value' => $node_id,
        '#attributes' => array('class' => 'hidden_nid'),
      );
      // append rendered hidden node id to last rendered field
      $vars['rows'][$num][$field_name] .= drupal_render($hidden_nid);
      
      // put indentation in front of first rendered field
      $first_field = each($vars['rows'][$num]);
      $vars['rows'][$num][$first_field['key']] = $indentation . $vars['rows'][$num][$first_field['key']];
    }
    
    
    /*****************************************
     * IMPLEMENTATION OF REALTIMEEDIT MODULE *
    ******************************************/
    
    if(module_exists('realtimeedit') &&  // check if module exists
       user_access('administer nodes'))  // check permissions
    {
      //get enabled fields
      $realtimeedit_enabled_fields = $style_options['realtimeedit_enabled_fields'];
      
      // get realtimeedit enabled fields sepcification
      $enabled_fields_spec = array();
      if( count($realtimeedit_enabled_fields) > 0 ){
        foreach($realtimeedit_enabled_fields as $enabled_field){
          $enabled_fields[$enabled_field['field']] = true;
        }
      }
  
  
      // prepare all fields
      foreach($fields AS $key => $field_obj){
        if( $enabled_fields[$key] &&               // check if field is marked to be editable
            $field_obj->options['exclude'] == 0 )  // check if field is not exluded from display
        {
          
          // set unique selector
          $css_class = 'realtimeedit-field';
          $selector = 'realtimeedit-field-' . $node_id . '-' . $field_obj->real_field . '-alias-' . $key;
          $wrapper_id = $selector . '-wrapper';
          $ahah_binding = array(
            'url'   => url('realtimeedit/ajax'),
            'event' => 'click',
            'wrapper' => $wrapper_id,
            'selector' => '#' . $selector,
            'effect'   => 'fade',
            'method'   => 'replace',
            'progress' => array('type' => 'throbber'),
          );
          // add js
          drupal_add_js('misc/jquery.form.js');
          drupal_add_js('misc/ahah.js');
          drupal_add_js(array('ahah' => array($selector => $ahah_binding)), 'setting');

          
          // append rendered hidden node id to last rendered field
          // check if field uses link
          if( $field_obj->options['link_to_node'] == 0 ){
            $vars['rows'][$num][$key] = '<span id="' . $wrapper_id . '" class="' . $css_class . '">' . $vars['rows'][$num][$key] . '</span>';
            //$vars['rows'][$num][$key] .= '<a href="#" id="' . $selector . '">edit</span>';
          }
        }
      }
    }
  }
}


/*
 * Get Hierarchy depth by checking the parent id
 * - (1) Look for a parent node id.
 *   (2) Use new parent node as new node id.
 *   (3) When new node id differs from 0 go back to (1)
 * 
 * @param $node_id
 *   The node id we start from
 * @param $result
 *   The results the view returned
 * @param $parent_field_alias
 *   The field_alias of the field that contains the parent node id
 */
function _draggableviews_get_hierarchy_depth($node_id, $result, $parent_field_alias){
  $tmpId = $node_id; // set temporary id
  $depth = -1;       // set depth

  //check if nid of current node exists
  while( $tmpId > 0 /* prevent infinity loop in case of failure */&& $depth < 100 /**/){
    // get index where nid matches tmpId
    $index = _draggableviews_get_result_index_by_nid($tmpId, &$result);
    // get parent id
    $tmpId = $result[$index]->$parent_field_alias;

    $depth++;
  }
    
  return $depth;
}

/*
 * Get index of result array where nid matches
 * - Compare the nid field with the node id we're looking for.
 *   In case of equality return the index of the result array
 * 
 * @param $nid
 *   The node id we're looking for
 * @param $result
 *   The results the view returned
 */
function _draggableviews_get_result_index_by_nid($nid, $result){
  for($i = 0; $i < count($result); $i++){
  	// if current nid matches nid we're looking for, return index
    if($result[$i]->nid == $nid) return $i;
  }
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
function _draggableviews_get_form_element($nid, $field_name, $value, $view){
  // get content field
  //print_r($view->field[$field_name]);
  $content_field = $view->field[$field_name]->content_field;
  // get cck widget type
  $widget_type = !empty($content_field) ? $content_field['widget']['type'] : 'hidden';
  
  switch($widget_type){
  	case 'optionwidgets_select':
  	  return array(
  	    '#type' => 'select',
  	    '#name' => $field_name . '_' . $nid,
  	    '#value' => $value,
  	    '#options' => content_allowed_values($content_field),
  	    '#attributes' => array('class' => $field_name),
  	  );
  	  break;
  	case 'dummy__nodereference_select':
  	  return array(
  	    '#type' => 'nodereference_autocomplete',
        '#default_value' => isset($items[$delta]) ? $items[$delta] : NULL,
        '#value_callback' => 'nodereference_autocomplete_value',
  	    '#attributes' => array('class' => $field_name),
  	  );
  	case 'hidden':
  	default:
      return array(
        '#type' => 'hidden',
        '#name' => $field_name . '_' . $nid,
        '#value' => $value,
        '#attributes' => array('class' => $field_name),
      );
  }
}

?>