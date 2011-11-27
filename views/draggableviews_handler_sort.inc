<?php
/**
 * Sort handler for ordering by thread
 */
class draggableviews_handler_sort extends views_handler_sort {
  function query() {
    $this->ensure_my_table();
    $this->query->add_orderby($this->table_alias, $this->real_field);
  }
}
