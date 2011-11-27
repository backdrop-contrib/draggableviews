<?php

/**
* @file
* Views field handler. Contains all relevant VBO options and related logic.
* Implements the Views Form API.
*/

class draggableviews_handler_field_draggable extends views_handler_field {

  function option_definition() {
    $options = parent::option_definition();

    $options['draggableviews'] = array(
      'contains' => array(
        'save_button_label' => array('default' => 'Save'),
      ),
    );

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    // Remove not needed settings options.
    $form['alter']['#access'] = FALSE;
    $form['style_settings']['#access'] = FALSE;
    $form['empty_field_behavior']['#access'] = FALSE;

    $form['draggableviews'] = array(
      '#type' => 'fieldset',
      '#title' => t('Draggable Views'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );

    $form['draggableviews']['save_button_label'] = array(
      '#type' => 'textfield',
      '#title' => t('Custom Save button label'),
      '#description' => t("Allow to change Save button Label."),
      '#default_value' => $this->options['draggableviews']['save_button_label'],
    );

//    $form['vbo']['display_type'] = array(
//      '#type' => 'radios',
//      '#title' => t('Display operations as'),
//      '#default_value' => $this->options['vbo']['display_type'],
//      '#options' => array(
//        t('Dropdown selectbox with Submit button'),
//        t('Each action as a separate button'),
//      ),
//    );
//    $form['vbo']['enable_select_all_pages'] = array(
//      '#type' => 'checkbox',
//      '#title' => t('Enable "Select all items on all pages"'),
//      '#default_value' => $this->options['vbo']['enable_select_all_pages'],
//      '#description' => t('Check this box to enable the ability to select all items on all pages.'),
//    );
//    $form['vbo']['force_single'] = array(
//      '#type' => 'checkbox',
//      '#title' => t('Force single'),
//      '#default_value' => $this->options['vbo']['force_single'],
//      '#description' => t('Check this box to restrict selection to a single value.'),
//    );
//    $form['vbo']['display_result'] = array(
//      '#type' => 'checkbox',
//      '#title' => t('Display processing result'),
//      '#description' => t('Check this box to let Drupal display a message with the result of processing the selected items.'),
//      '#default_value' => $this->options['vbo']['display_result'],
//    );
//    $form['vbo']['merge_single_action'] = array(
//      '#type' => 'checkbox',
//      '#title' => t('Merge single action\'s form with the row selection view'),
//      '#description' => t('In case only one action is selected *and* this action is configurable, display its action form along with the row selection view.'),
//      '#default_value' => $this->options['vbo']['merge_single_action'],
//    );
//    $form['vbo']['entity_load_capacity'] = array(
//      '#type' => 'textfield',
//      '#title' => t('Number of entities to load at once'),
//      '#description' => t("Improve execution performance at the cost of memory usage. Set to '1' if you're having problems."),
//      '#default_value' => $this->options['vbo']['entity_load_capacity'],
//    );
//    // Display operations and their settings.
//    $form['vbo']['operations'] = array(
//      '#tree' => TRUE,
//      '#type' => 'fieldset',
//      '#title' => t('Selected operations'),
//      '#collapsible' => TRUE,
//      '#collapsed' => FALSE,
//    );
//    foreach ($this->get_operations_options() as $key => $label) {
//      $options = isset($this->options['vbo']['operations'][$key]) ? $this->options['vbo']['operations'][$key] : array();
//
//      $dom_id = 'edit-options-vbo-operations-' . str_replace(array('_', ':'), array('-', ''), $key);
//      $form['vbo']['operations'][$key]['selected'] = array(
//        '#type' => 'checkbox',
//        '#title' => $label,
//        '#default_value' => !empty($options['selected']),
//      );
//      if (!$this->all_operations[$key]['aggregate']) {
//        $form['vbo']['operations'][$key]['use_queue'] = array(
//          '#type' => 'checkbox',
//          '#title' => t('Enqueue the operation instead of executing it directly'),
//          '#default_value' => !empty($options['use_queue']),
//          '#dependency' => array(
//            $dom_id . '-selected' => array(1),
//          ),
//        );
//      }
//      $form['vbo']['operations'][$key]['skip_confirmation'] = array(
//        '#type' => 'checkbox',
//        '#title' => t('Skip confirmation step'),
//        '#default_value' => !empty($options['skip_confirmation']),
//        '#dependency' => array(
//          $dom_id . '-selected' => array(1),
//        ),
//      );
//      $show_label = isset($options['label']) ? $options['label'] : '';
//      $form['vbo']['operations'][$key]['override_label'] = array(
//        '#type' => 'checkbox',
//        '#title' => t('Override label'),
//        '#default_value' => $show_label !== '',
//        '#dependency' => array(
//          $dom_id . '-selected' => array(1),
//        ),
//      );
//      $form['vbo']['operations'][$key]['label'] = array(
//        '#type' => 'textfield',
//        '#title' => t('Provide label'),
//        '#title_display' => 'invisible',
//        '#default_value' => $show_label,
//        '#dependency' => array(
//          $dom_id . '-selected' => array(1),
//          $dom_id . '-override-label' => array(1),
//        ),
//        '#dependency_count' => 2,
//      );
//      $form_function = $this->all_operations[$key]['callback'] . '_views_bulk_operations_form';
//      if (function_exists($form_function)) {
//        $settings = isset($options['settings']) ? $options['settings'] : array();
//        $form_settings = call_user_func($form_function, $settings);
//        $form['vbo']['operations'][$key]['settings'] = array(
//          '#type' => 'fieldset',
//          '#title' => t('Operation settings'),
//          '#collapsible' => TRUE,
//          '#dependency' => array(
//            $dom_id . '-selected' => array(1),
//          ),
//        );
//        $form['vbo']['operations'][$key]['settings'] += $form_settings;
//      }
//    }
  }

  function render($values) {
    return '<!--form-item-' . $this->options['id'] . '--' . $this->view->row_index . '-->';
  }

  /**
   * The form which replaces the placeholder from render().
   */
  function views_form(&$form, &$form_state) {
    // The view is empty, abort.
    if (empty($this->view->result)) {
      return;
    }

    // Pass view object to submit handler.
    $form['view'] = array(
      '#type' => 'value',
      '#value' => $this->view,
    );

    $form[$this->options['id']] = array(
      '#tree' => TRUE,
    );
    // At this point, the query has already been run, so we can access the results
    // in order to get the base key value (for example, nid for nodes).
    foreach ($this->view->result as $row_index => $row) {
      $entity_id = $this->get_value($row);

      $form[$this->options['id']][$row_index] = array(
        '#tree' => TRUE,
      );
      // Weight field selectbox.
      $form[$this->options['id']][$row_index]['weight'] = array(
        '#type' => 'select',
        '#options' => range(-10, 10),
        '#attributes' => array('class' => array('draggableviews-weight')),
      );
      // Item to keep id of the entity.
      $form[$this->options['id']][$row_index]['id'] = array(
        '#type' => 'value',
        '#value' => $this->view->result[$row_index]->{$this->view->base_field},
      );
    }
  }

}
