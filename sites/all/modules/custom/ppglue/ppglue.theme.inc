<?php

/**
 * Prev and Next arrows
 */
function theme_ppglue_prevnext($vars) {
    
  $output = $nids = NULL;
  
  if (isset($vars['entity'])) {
    // if ($vars['entity']->type == 'blog_post' && isset($vars['entity']->field_blog_term['und'])) {
      // set defaults
      $prev_text = isset($vars['prev_text']) ? $vars['prev_text'] : "&laquo; Previous"; 
      $next_text = isset($vars['next_text']) ? $vars['next_text'] : "Next &raquo;"; 
      $prev = $next = $vars['entity']->nid;
      // find deepest tid from node
      // $tid = $vars['entity']->field_blog_term['und'][0]['tid'];
      // if (count($vars['entity']->field_blog_term['und'])>1) {
        // $tids = _gs_helper_extract_select_values($vars['entity']->field_blog_term);
        // $tid = array_pop($tids);
      // }
      try {
        // find all nids in database      
        $query = new EntityFieldQuery();
        $query->entityCondition('entity_type', 'node')
          ->entityCondition('bundle', $vars['entity']->type)
          ->propertyCondition('status', 1)
          // ->fieldCondition('field_blog_term', 'tid', $tid)
          ->propertyOrderBy('created', 'DESC');
        $result = $query->execute();
        $nids = array_keys($result['node']);
      } catch(Exception $e) {
        watchdog_exception('ppglue', $e);
      }
      
      // output prev and next arrows
      if (count($nids)>1) {
        // find next and prev nid
        $key = array_search($vars['entity']->nid, $nids);
        $prev = isset($nids[$key - 1]) ? $nids[$key - 1] : $nids[count($nids) - 1];
        $next = isset($nids[$key + 1]) ? $nids[$key + 1] : $nids[0];
        // item list variables
        $variables = array(
          'items'=>array(
            array(
              'data' => l($prev_text,drupal_get_path_alias("node/$prev"),array(
                'html'=>TRUE,
                'attributes' => array(
                  'alt'=>t("Previous post"),
                  'title'=>t("Previous post"),
                ),
              )), 
              'class' => array('prev','prevnext')
            ),
            array(
              'data' => l($next_text,drupal_get_path_alias("node/$next"),array(
                'html'=>TRUE,
                'attributes' => array(
                  'alt'=>t("Next post"),
                  'title'=>t("Next post"),
                ),
              )), 
              'class' => array('next','prevnext')
            ),
          ),
          'attributes' => array(
            'class'=>array('prevnext-list'),
          ),
        );
        $output .= theme('item_list',$variables);
      }
    // }
  }
  
  return $output;
}