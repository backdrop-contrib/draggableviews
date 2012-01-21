/**
 * @file
 * Adds draggable functionality to the html list display of the view.
 */

(function ($) {
 Drupal.behaviors.draggableViews = {
  attach: function (context, settings) {
    $('.views-form .' + Drupal.settings.draggableviews_row_class + ':not(.draggableviews-processed)', context)
    // Add class for theming.
    .addClass('draggableviews-processed')
    // Add sortable effect.
    .sortable({
      update: function(event, ui) {
        $( ".draggableviews-weight" ).each(function (i, Val) {
          $(this).val(i);
        });
        if (!$(this).hasClass('draggableviews-changed')) {
          $('<div class="draggableviews-changed-warning messages warning">' + Drupal.t('Changes made in this list will not be saved until the form is submitted.') + '</div>')
            .insertBefore($(this).parents('.views-form')).hide().fadeIn('slow');
          $(this).addClass('draggableviews-changed');
        }
      },
      containment: 'parent',
      cursor: 'move'
    });
  }
 }
})(jQuery);
