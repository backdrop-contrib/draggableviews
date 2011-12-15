<?php
/**
 * Sort handler for ordering by width
 */
class draggableviews_handler_sort extends views_handler_sort {
  function query() {
    $this->ensure_my_table();
    $this->query->add_orderby($this->table_alias, $this->real_field);
  }

  function option_definition() {
    $options = parent::option_definition();

    // This handler invokes few times for one view,
    // in the first time the $this->view->name is empty,
    // so we need this check.
    if (is_object($this->view)) {
      $options['draggableviews_setting_view'] = array('default' => $this->view->name);
    }
    else {
      $options['draggableviews_setting_view'] = array('default' => NULL);
    }

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    // Check whether current views display doesn't have draggableviews field.
    // If it has, it means that this is setting view so we should set
    // option draggableviews_setting_view to 'self'
    $view_clone = clone $this->view;
    $view_clone->build($view_clone->current_display);
    $options = 'self';
    // Check whether field exists. If not it is displaying view so lets show
    // all options of other setting views.
    if (!isset($view_clone->field['draggableviews'])) {
      // Get list of all enabled views.
      $views_list = views_get_enabled_views();
      $options = array();

      // Convert list of objects to options for the form.
      foreach ($views_list as $view_name => $view_object) {
        // Skip current view.
        if ($view_name == $this->view->name) {
          continue;
        }
        foreach ($view_object->display as $display_name => $display) {
          if ($display_name == 'default') {
            continue;
          }
          // Clone view and build it so we can see all the fields.
          $view_clone = clone $view_object;
          $view_clone->build($display_name);

          // If draggableviews field attached, show this view in options.
          if (isset($view_clone->field['draggableviews'])) {
            $options[$view_name . ':' . $display_name] = $view_object->human_name . ' (' . $display->display_title . ')';
          }
        }
      }
    }

    // If it is setting view.
    if (!is_array($options)) {
      $form['draggableviews_setting_view'] = array(
        '#type' => 'value',
        '#value' => $options,
      );
    }
    else {
      $form['draggableviews_setting_view'] = array(
        '#type' => 'select',
        '#title' => t('Display sort as'),
        '#default_value' => $this->options['draggableviews_setting_view'],
        '#options' => $options,
        '#description' => t('Please choose the view and display that sets the order.')
      );
      // If there is no setting views available, show error message.
      if (empty($options)) {
        drupal_set_message(t('First you should create a view that sets sorting order.'), 'error');
      }
    }
  }
}
