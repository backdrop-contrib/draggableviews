/**
 * @file
 * Adds draggable functionality to the html list display of the view.
 */

(function ($) {
 Drupal.behaviors.draggableViews = {
  attach: function (context, settings) {
    var list = $(".views-form ul." + Drupal.settings.draggableviews_row_class);
    // Add class for theming.
    list.addClass('sortable');
    // Add sortable effect.
    list.sortable({
      update: function(event, ui) {
        $( ".draggableviews-weight" ).each(function (i, Val) {
          $(this).val(i);
        });
      },
      containment: 'parent',
      cursor: 'move'
    });
  }
 }
})(jQuery);
