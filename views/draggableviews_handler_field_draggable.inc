<?php

/**
* @file
* Views field handler. Contains all relevant Draggableviews
* options and related logic.
* Implements the Views Form API.
*/

class draggableviews_handler_field_draggable extends views_handler_field {

  function option_definition() {
    $options = parent::option_definition();

    $options['draggableviews'] = array(
      'contains' => array(
        'handler' => array('default' => 'draggableviews_native'),
        'save_button_label' => array('default' => 'Save'),
        'ajax' => array('default' => FALSE),
      ),
    );

    // Populate default values of form elements provided by handlers.
    ctools_include('plugins');
    $handlers = ctools_get_plugins('draggableviews', 'handler');
    foreach ($handlers as $handler_id => $handler) {
      // Add handlers form elements.
      if ($definition_function = ctools_plugin_load_function('draggableviews', 'handler', $handler_id, 'option definition callback')) {
        $options['draggableviews']['contains'][$handler_id] = array('default' => $definition_function());
      }
    }
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

    $handler_options = array();
    // Load all plugins type "handler".
    ctools_include('plugins');
    $handlers = ctools_get_plugins('draggableviews', 'handler');
    $handler_forms = array();
    foreach ($handlers as $handler_id => $handler) {
      $handler_options[$handler_id] = filter_xss($handler['label']);
      // Add handlers form elements.
      if ($form_function = ctools_plugin_load_function('draggableviews', 'handler', $handler_id, 'options form callback')) {
        $handler_forms[$handler_id] = $form_function($this);
      }
    }

    $form['draggableviews']['handler'] = array(
      '#type' => 'select',
      '#title' => t('Handler'),
      '#options' => $handler_options,
      '#default_value' => $this->options['draggableviews']['handler'],
    );

    // Add handler's form element as fielset that
    // is active only if handler selected.
    foreach ($handler_forms as $handler_id => $handler_form_element) {
      $form['draggableviews'][$handler_id] = array(
        '#type' => 'fieldset',
        '#title' => $form['draggableviews']['handler']['#options'][$handler_id],
        '#collapsible' => FALSE,
        '#states' => array(
          'visible' => array(
            'select[name="options[draggableviews][handler]"]' => array('value' => $handler_id),
          ),
        ),
      );
      foreach ($handler_form_element as $key => $form_element) {
        $form['draggableviews'][$handler_id][$key] = $form_element;
      }
    }

    $form['draggableviews']['save_button_label'] = array(
      '#type' => 'textfield',
      '#title' => t('Custom Save button label'),
      '#description' => t("Allow to change Save button Label."),
      '#default_value' => $this->options['draggableviews']['save_button_label'],
    );

    $form['draggableviews']['ajax'] = array(
      '#type' => 'checkbox',
      '#title' => t('Ajax'),
      '#description' => t('Use ajax in draggable form.'),
      '#default_value' => $this->options['draggableviews']['ajax'],
    );
  }

  // TODO add validation callback for plugin to ensure all other settings are in place (sort order for native filter for example).
  function options_validate(&$form, &$form_state) {
    // Get name of getter function of plugin.
//    ctools_include('plugins');
//    $validation_function = ctools_plugin_load_function('draggableviews', 'handler', $this->options['draggableviews']['handler'], 'options validation callback');
//    $validation_function($this->view, $form, $form_state);
  }

  function render($values) {
    if (user_access('access draggableviews')) {
      return '<!--form-item-' . $this->options['id'] . '--' . $this->view->row_index . '-->';
    }
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
    $range = count($this->view->result);
    // At this point, the query has already been run, so we can access the results
    // in order to get the base key value (for example, nid for nodes).
    foreach ($this->view->result as $row_index => $row) {
      $entity_id = $this->get_value($row);

      $form[$this->options['id']][$row_index] = array(
        '#tree' => TRUE,
      );

      // Get name of getter function of plugin.
      ctools_include('plugins');
      $getter_function = ctools_plugin_load_function('draggableviews', 'handler', $this->options['draggableviews']['handler'], 'getter callback');

      // Weight field selectbox.
      $form[$this->options['id']][$row_index]['weight'] = array(
        '#type' => 'select',
        '#options' => range(-$range, $range),
        '#attributes' => array('class' => array('draggableviews-weight')),
        '#default_value' => $getter_function($this, $row_index),
      );
      // Item to keep id of the entity.
      $form[$this->options['id']][$row_index]['id'] = array(
        '#type' => 'value',
        '#value' => $this->view->result[$row_index]->{$this->view->base_field},
      );
    }
  }
}