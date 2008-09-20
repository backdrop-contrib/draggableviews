<?php
// $Id$

/**
 * @file
 * Theme functions.
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
    $query = '&'. $query;
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
          'html' => TRUE,
          'attributes' => array('title' => $title),
          'query' => 'order='. urlencode($field) .'&sort='. $initial . $query,
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
  
  $error = FALSE;
  
  $info = _draggableviews_info($view);
  
  // calculate depth of all nodes.
  // If one of the parents of a node is broken,
  // the return will be FALSE
  if (!_draggableviews_calculate_depths($info)) {
  
    drupal_set_message(t('Draggableviews: Detected broken parents.'), 'error');
    
    $error = TRUE;
  }
  
  // Detect order collisions
  // If one order of the same depth is found
  // twice the return will be FALSE.
  if (!_draggableviews_detect_order_collisions($info)) {
  
    drupal_set_message(t('Draggableviews: Detected order collisions.'), 'error');
  
    $error = TRUE;
  }
  
  if ($error) {
    // on error rebuild hierarchy and return
    _draggableviews_rebuild_hierarchy($info);
    
    return;
  }
  
  // loop through all rows the view returns
  foreach ($vars['rows'] as $i => $row) {

    // get node object
    $node = node_load(array('nid' => $result[$i]->nid));
    
    if (isset($info['types'][$node->type])) {
      // set node type specification, if available (e.g. tabledrag-root)
      $vars['tabledrag_type'][$i] = 'tabledrag-'. $info['types'][$node->type];
    }
    
    // Detect ordering errors
    // The nodes order value has to equal with the
    // parents order value. Otherwise the return will be FALSE. 
    if (_draggableviews_check_order($node->nid, $info) == FALSE) {
    
      drupal_set_message(t('Draggableviews: Detected ordering errors.'), 'error');
      
      // on error rebuild hierarchy and return
      _draggableviews_rebuild_hierarchy($info);
      return;
    }
    
    // build indentation (as tabledrag requires)
    $indentation = theme('indentation', $info['nodes'][$node->nid]['depth']);
    
    // Tabledrag needs all concerned fields to be input elements.
    // The values of the input elements will be changed by tabledrag while
    // dragging the rows. As we want to use these values we need to
    // give them names.
    //
    // Concerned fields are
    //  *) the first order field
    //  *) the parent field (if hierarchy used)
    //
    // The remaining fields are not used by tabledrag. But - depending
    // on the depth - one of them will simulate the first order field.
    // (This behavior is based on the fact that tabledrag handles each depth as it was the only one.)
    
    // this for loop should prevent copy->paste'ing code
    for ($modus = 'hierarchy'; $modus !== FALSE ; $modus = ($modus == 'hierarchy' ? 'order' : FALSE)) {
    
      if (isset($info[$modus])) {
        if ($modus == 'hierarchy') {
          // there's only one hierarchy field, so take it
          $field_name = $info['hierarchy']['field']['field_name'];
          
          $value = $info['nodes'][$node->nid][$field_name];
          
          if ($value > 0 && !isset($info['nodes'][$value])) {
            // if parent node doesn't rebuild hierarchy and return
            _draggableviews_rebuild_hierarchy($info);
            return;
          }
          
          // get the field that should be rendered
          // it's still the field name
          // $field_name = $field_name;
          $field = $fields[$field_name]->content_field;
        }
        else {
          // the field to use depends on the depth
          $field_name = $info['order']['fields'][$info['nodes'][$node->nid]['depth']]['field_name'];
          
          $value = $info['nodes'][$node->nid][$field_name];
          
          // get the field that should be rendered
          $field_name = $info['order']['fields'][0]['field_name'];
          $field = $fields[$field_name]->content_field;
        }
        
        // get the form element
        $form_element = _draggableviews_get_form_element($value, $field, $info, array('field_name' => $field_name .'_'. $node->nid, 'class' => $field_name));
        
        // render new form element
        $vars['rows'][$i][$field_name] = drupal_render($form_element);
      }
    }
    
    
    if (isset($info['hierarchy'])) {
      // put indentation in front of first field
      $first_field = each($vars['rows'][$i]);
      $vars['rows'][$i][$first_field['key']] = $indentation . $vars['rows'][$i][$first_field['key']];
      
      // append the node id as a hidden field. This is needed
      // because tabledrag would not know what id to assign when
      // subordinating.
      $hidden_nid = array(
        '#type' => 'hidden',
        '#name' => 'hidden_nid',
        '#value' => $node->nid,
        '#attributes' => array('class' => 'hidden_nid'),
      );
      // append rendered hidden node id to last rendered field
      $vars['rows'][$i][$field_name] .= drupal_render($hidden_nid);
    }
    
    
    if (module_exists('realtimeedit') && user_access('administer nodes')) {
      // IMPLEMENTATION OF REALTIMEEDIT MODULE

      // prepare all fields
      foreach ($info['realtimeedit_enabled_fields'] as $field => $enabled) {
      
        if ($fields[$field]->options['exclude'] == 0 ) {

          // set unique selector
          $css_class = 'realtimeedit-field';
          $selector = 'realtimeedit-field-'. $node_id .'-'. $field_obj->definition['content_field_name'] .'-alias-'. $key;
          $wrapper_id = $selector .'-wrapper';
          $ahah_binding = array(
            'url'   => url('realtimeedit/ajax'),
            'event' => 'click',
            'wrapper' => $wrapper_id,
            'selector' => '#'. $selector,
            'effect'   => 'fade',
            'method'   => 'replace',
            'progress' => array('type' => 'throbber'),
          );
          // add js
          drupal_add_js('misc/jquery.form.js');
          drupal_add_js('misc/ahah.js');
          drupal_add_js(array('ahah' => array($selector => $ahah_binding)), 'setting');
          
          // check if field uses link
          if ($fields[$field]->options['link_to_node'] == 0) {
          
            $vars['rows'][$i][$key] = '<span id="'. $wrapper_id .'" class="'. $css_class .'">'. $vars['rows'][$i][$key] .'</span>';
          }
        }
      }
    }
  }
  
  // output data
  $vars['tabledrag_table_id'] = 'draggableview_'. $view->name;
  
  // prepare tabledrag settings for output
  $vars['tabledrag'] = array();
  if (isset($info['order'])) {
    $vars['tabledrag'][] = array(
      'action'       => 'order',
      'relationship' => 'sibling',
      'source'       => $info['order']['fields'][0]['field_name'],
      'group'        => $info['order']['fields'][0]['field_name'],
      'subgroup'     => $info['order']['fields'][0]['field_name'],
      'hidden'       => !$info['order']['visible'],
      'limit'        => 0,
    );
  }
  if (isset($info['hierarchy'])) {
    $vars['tabledrag'][] = array(
      'action'       => 'match',
      'relationship' => 'parent',
      'source'       => 'hidden_nid',
      'group'        => $info['hierarchy']['field']['field_name'],
      'subgroup'     => $info['hierarchy']['field']['field_name'],
      'hidden'       => !$info['hierarchy']['visible'],
      'limit'        => count($info['order']['fields']),
    );
    // let realtimeedit module know about fields
    drupal_add_js(array('draggableviews' => array('parent' => $info['hierarchy']['field']['field_name'])), 'setting');
  }
}

function template_preprocess_draggableviews_view_draggabletable_form($vars) {
  global $user;
  
  // get view object
  $view_obj = $vars['form']['#parameters'][2];
  $options  = $view_obj->options;
  
  // add javascript
  // BE AWARE: realtimeedit module js has to be loaded first,
  // generated links should be appended in the right order
  
  //include realtimeedit module
  if (module_exists('realtimeedit')) { // check if module exists
    drupal_add_js(drupal_get_path('module', 'realtimeedit') .'/realtimeedit.js');
    drupal_add_css(drupal_get_path('module', 'realtimeedit') .'/styles.css');
  }
  
  // fetch expand information from database
  $result = db_query(db_rewrite_sql("SELECT parent_nid, collapsed FROM {draggableviews_collapsed} WHERE uid = %d"), $user->uid);
  $states = array();
  while ($state = db_fetch_object($result)) {
    $states[$state->parent_nid] = $state->collapsed;
  }
  // check if "expand" links should be shown
  if ($options['tabledrag_expand']['expand_links']) {
  
    drupal_add_js(drupal_get_path('module', 'draggableviews') .'/draggableviews.js');
    
    if (count($states) == 0) {
      // let js know wether child nodes should be expanded or not
      drupal_add_js(array(
                          'draggableviews' => array(
                            'expand_default' =>  $options['tabledrag_expand']['collapsed']
                                                 ? 1 : 0,
                            ),
                          ),
                    'setting');
    }
    else {
      drupal_add_js(array(
                          'draggableviews' => array(
                            'states' => $states,
                            ),
                          ),
                    'setting');
    }
    
    drupal_add_css(drupal_get_path('module', 'draggableviews') .'/styles.css');
  }
  
  
  //theme view
  $sets = $view_obj->render_grouping($view_obj->view->result, $options['grouping']);
  
  $output = '';
  foreach ($sets as $title => $records) {
    $output .= theme($view_obj->theme_functions(), $view_obj->view, $options, $records, $title);
  }
  
  $vars['view'] = $output;

  
  //render submit form
  $vars['submit_form'] = drupal_render($vars['form']);
}