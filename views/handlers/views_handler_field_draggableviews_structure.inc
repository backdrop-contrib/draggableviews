<?php


/**
 * A handler to provide proper displays for time units (i.e. calendar weeks).
 */
class views_handler_field_draggableviews_structure extends views_handler_field {
  function init(&$view, &$data) {
    parent::init($view, $data);
  }

  function click_sort($order) {
    $this->query->add_orderby($this->table, $this->field, $order, $this->field_alias);
  }
}