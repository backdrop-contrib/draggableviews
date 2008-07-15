<?php
// $Id$
/**
 * @file views-view-draggabletable.tpl.php
 * Template to display a view as a draggable table.
 *
 * - $header: An array of header labels keyed by field id.
 * - $fields: An array of CSS IDs to use for each field id.
 * - $class: A class or classes to apply to the table, based on settings.
 * - $rows: An array of row items. Each row is an array of content
 *   keyed by field ID.
 * 
 * - $tabledrag: An array of tabledrag settings
 * - $tabledrag_tableId: The table id that drupal_add_tabledrag needs
 * @ingroup views_templates
 */

  //add tabledrag
  if( count($tabledrag) > 0 ){
    foreach($tabledrag as $drag){
      drupal_add_tabledrag($tabledrag_tableId, $drag['action'], $drag['relationship'], $drag['group'], $drag['subgroup'], $drag['source'], $drag['hidden'], $drag['limit']);
    }
  }
?>
<table class="<?php print $class; ?>" id="<?php print $tabledrag_tableId; ?>">
  <thead>
    <tr>
      <?php foreach ($header as $field => $label): ?>
        <th class="views-field views-field-<?php print $fields[$field]; ?>">
          <?php print $label; ?>
        </th>
      <?php endforeach ?>
    </tr>
  </thead>
  <tbody>
    <?php foreach ($rows as $count => $row): ?>
      <tr class="draggable <?php print ($count % 2 == 1) ? 'even' : 'odd';?><? if($tabledrag_type[$count]) print ' '.$tabledrag_type[$count]; ?>">
        <?php foreach ($row as $field => $content): ?>
          <td class="views-field views-field-<?php print $fields[$field]; ?>"><?php
            print $content;
          ?></td>
        <?php endforeach; ?>
      </tr>
    <?php endforeach; ?>
  </tbody>
</table>