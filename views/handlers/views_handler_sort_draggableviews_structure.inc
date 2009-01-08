<?php
/**
 * A sort handler that properly takes non-completed targets into account.
 */
class views_handler_sort_draggableviews_structure extends views_handler_sort {
  function query() {
    $this->ensure_my_table();
    $order = strtolower($this->options['order']);

    $this->query->add_orderby($this->table_alias, $this->real_field, $order);
  }
}