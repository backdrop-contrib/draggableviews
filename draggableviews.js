//check js availability
if (Drupal.jsEnabled) {
  //start at onload-event
  $(document).ready(draggableviews_load);
}

//load editfield-plugin
function draggableviews_load(){
  $(".tabledrag-root").each( function(i) {
    var nid = $(this).find('td > .hidden_nid').attr('value');
    $(this).find('td:first').each( function(i) {
      //$(this).append('<a class="draggableviews-expand" href="#" onclick="javascript:draggableviews_collapse('+nid+')">collapse</a>');
      $(this).append('<div class="draggableviews-expand" href="#"></div>').children('.draggableviews-expand').bind('click', function(){draggableviews_collapse(nid);});
    });
    
    // apply collapsed/expanded state
    if (Drupal.settings.draggableviews.states) {
      if (Drupal.settings.draggableviews.states[nid]) {
        // when list should be collapsed..
        if (Drupal.settings.draggableviews.states[nid] == 1) {
          // ..collapse list
          draggableviews_collapse(nid);
          
          // and set hidden field
          draggableviews_set_state_field(nid, true);
        }
      } 
    }
  });
  
  // collapse all by default if set
  if( Drupal.settings.draggableviews.expand_default &&
      Drupal.settings.draggableviews.expand_default == 1 )
  {
    draggableviews_collapse_all();
  }
}

function draggableviews_expand(parent_id){
  // show elements
  draggableviews_show(parent_id);
  
  // swap link to collapse link
  $("tr:has(td > .hidden_nid[value="+parent_id+"])").each( function (i){
    $(this).find('.draggableviews-collapse').each( function (i){
      $(this).unbind('click');
      $(this).attr('class', 'draggableviews-expand');
      $(this).bind('click', function(){ draggableviews_collapse(parent_id); });
      // set state as value of a hidden field
      draggableviews_set_state_field(parent_id, false);
    });
  });
}

// show recursively
function draggableviews_show(parent_id) {
  $("tr:has(td > ." + Drupal.settings.draggableviews.parent + "[value="+parent_id+"])").each( function (i){
    $(this).show();
    var nid = $(this).find('td > .hidden_nid').attr('value');
    if (nid) {
      draggableviews_show(nid);
    }
  });
}

function draggableviews_collapse(parent_id){
  // hide elements
  draggableviews_hide(parent_id);
  
  // swap link to expand link
  $("tr:has(td > .hidden_nid[value=" + parent_id + "])").each( function (i){
    $(this).find('.draggableviews-expand').each( function (i){
      //$(this).unbind('onclick');
      $(this).unbind('click');
      $(this).attr('class', 'draggableviews-collapse');
      $(this).bind('click', function(){ draggableviews_expand(parent_id); });
      
      // set state as value of a hidden field
      draggableviews_set_state_field(parent_id, true);
    });
  });
}

// hide recursively
function draggableviews_hide(parent_id) {
  $("tr:has(td > ." + Drupal.settings.draggableviews.parent + "[value="+parent_id+"])").each( function (i){
    $(this).hide();
    var nid = $(this).find('td > .hidden_nid').attr('value');
    if (nid) {
      draggableviews_hide(nid);
    }
  });
}

function draggableviews_collapse_all() {
  // hide elements
  $("tr:has(td > ." + Drupal.settings.draggableviews.parent + "[value<>0])").each( function (i) {
    $(this).hide();
  });
  
  // swap links to expand links
  $("tr:has(td > .hidden_nid)").each( function (i){
    var parent_id = $(this).find('td > .hidden_nid').attr('value');
    
    $(this).find('.draggableviews-expand').each( function (i){
      // set new action and class
      $(this).unbind('click');
      $(this).attr('class', 'draggableviews-collapse');
      $(this).bind('click', function(){ draggableviews_expand(parent_id); });
      
      // set collapsed/expanded state
      draggableviews_set_state_field(parent_id, true);
    });
  });

  //  $(this).bind('click', function(){ draggableviews_expand(1); });
  //});
}

// save state of expanded/collapsed fields in a hidden field
function draggableviews_set_state_field(parent_id, state) {
  //build field name
  var field_name = 'draggableviews_collapsed_' + parent_id;
  
  $("input[name='hidden_nid'][value='" + parent_id + "']")
  .parent().each( function(i){
    var replaced = false;
    
    // "check" if field already exists (..by just selecting it)
    $(this).find('input[name="' + field_name + '"]').each( function(i){
      $(this).attr('value', state?'1':'0');
      replaced = true;
    });
    // if no field existed yet -> create it
    if (replaced == false) {
      $(this).append('<input type="hidden" name="' + field_name + '" value="' + (state?'1':'0') + '" />');
    }
  });
}