<?php

/**
 * @file
 * Views field handler. Contains all relevant Draggableviews
 * options and related logic.
 * Implements the Views Form API.
 */

class draggableviews_handler_field_weight extends views_handler_field_numeric {

  function construct() {
    parent::construct();
  }

 /**
   * {@inheritdoc}
   */
  public function option_definition() {
    $options = parent::option_definition();
    $options['start_from_1'] = array(
      'default' => FALSE,
      'bool' => TRUE,
    );

    return $options;
  }

  /**
   * {@inheritdoc}
   */
  public function options_form(&$form, &$form_state) {
    $form['start_from_1'] = array(
      '#type' => 'checkbox',
      '#title' => t('Start from 1'),
      '#description' => t('If checked, the weight will be displayed as starting from 1.'),
      '#default_value' => $this->options['start_from_1'],
    );
    parent::options_form($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function render($values) {
    $value = $this
      ->get_value($values);

    // Hiding should happen before rounding or adding prefix/suffix.
    if ($this->options['start_from_1']) {
      $value++;
    }

    return $value;
  }
}
