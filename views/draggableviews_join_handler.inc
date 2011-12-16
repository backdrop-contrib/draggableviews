<?php
/**
 * Join handler for extra join conditions.
 */
class draggableviews_join_handler extends views_join {
  /**
   * Build the SQL for the join this object represents.
   */
  function build_join($select_query, $table, $view_query) {
    if (empty($this->definition['table formula'])) {
      $right_table = $this->table;
    }
    else {
      $right_table = $this->definition['table formula'];
    }

    if ($this->left_table) {
      $left = $view_query->get_table_info($this->left_table);
      $left_field = "$left[alias].$this->left_field";
    }
    else {
      // This can be used if left_field is a formula or something. It should be used only *very* rarely.
      $left_field = $this->left_field;
    }

    $condition = "$left_field = $table[alias].$this->field";

    // Check whether setting view is set.
    $arguments = array();
    if (!empty($view_query->view->sort['weight']->options['draggableviews_setting_view'])) {
      $condition .= " AND $table[alias].view_name = :view_name";
      $condition .= " AND $table[alias].view_display = :view_display";

      // If it is setting view, set current view name and display name.
      if ($view_query->view->sort['weight']->options['draggableviews_setting_view'] == 'self') {
        $arguments[':view_name'] = $view_query->view->name;
        $arguments[':view_display'] = $view_query->view->current_display;
      }
      else {
        list($setting_view_name, $setting_view_display) = explode(':', $view_query->view->sort['weight']->options['draggableviews_setting_view']);
        $arguments[':view_name'] = $setting_view_name;
        $arguments[':view_display'] = $setting_view_display;
      }
    }

    $select_query->addJoin($this->type, $right_table, $table['alias'], $condition, $arguments);
  }
}