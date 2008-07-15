//check js availability
if (Drupal.jsEnabled) {
  //start at onload-event
  $(document).ready(draggableviews_load);
}

//load editfield-plugin
function draggableviews_load(){
    $(".tabledrag-root").each( function(i) {
      var milestone_id = $(this).find('td > .hidden_nid').attr('value');
      $(this).find('td:first').each( function(i) {
        $(this).append('<a class="draggableviews-expand" href="#" onclick="javascript:draggableviews_collapse('+milestone_id+')">collapse</a>');
      });
    });
}

function draggableviews_expand(milestone_id){
  // hide elements
  $("tr:has(td > .field_parent_nid[value="+milestone_id+"])").each( function (i){
    $(this).show();
  });
  
  // swap link to collapse link
  $("tr:has(td > .hidden_nid[value="+milestone_id+"])").each( function (i){
    $(this).find('.draggableviews-expand').each( function (i){
      $(this).attr('onclick', 'draggableviews_collapse('+milestone_id+')');
      this.firstChild.data='collapse';
    });
  });
}

function draggableviews_collapse(milestone_id){
  // hide elements
  $("tr:has(td > .field_parent_nid[value="+milestone_id+"])").each( function (i){
    $(this).hide();
  });
  
  // swap link to expand link
  $("tr:has(td > .hidden_nid[value="+milestone_id+"])").each( function (i){
    $(this).find('.draggableviews-expand').each( function (i){
      $(this).attr('onclick', 'draggableviews_expand('+milestone_id+')');
      this.firstChild.data='expand';
    });
  });
}