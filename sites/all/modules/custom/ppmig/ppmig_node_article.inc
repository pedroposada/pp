<?php

/**
 * Migration class for Articles Nodes
 */
class NodeArticleMigration extends PPMigration {
  public function __construct() {
  	parent::__construct();
  	$this->description = t('Migrate PP blog nodes');
  	$this->dependencies = array('TermArticle');
  	
  	$this->map = new MigrateSQLMap($this->machineName,
      array(
        'nid' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'D6 Unique Node ID',
          'alias' => 'n',
        )
      ),
      MigrateDestinationNode::getKeySchema()
    );
  	
  	// We are getting data from tables in the Drupal default database - first,
    // set up a query for this data.
    $query = Database::getConnection('default', 'd6')->select('node', 'n');
    $query->fields('n', array('nid', 'vid', 'type', 'language', 'title', 'uid', 'status', 'created', 'changed', 'comment', 'promote', 'moderate', 'sticky', 'tnid', 'translate'));
    $query->condition('n.type', 'content_article', '=');
    $query->join('node_revisions', 'nr', 'n.vid = nr.vid');
    $query->addField('nr', 'body');
    $query->addField('nr', 'teaser');
    $query->addField('nr', 'format');
    $query->join('users', 'u', 'n.uid = u.uid');
    $query->addField('u', 'name');
  	$query->join($excludeblog,'eb','eb.nid = n.nid');
    $query->orderBy('n.changed');
  	
  	$source_fields = array(
      'nid' => t('The node ID of the article'),
      'url_alias' => t('The url alias of the node.'),
      'title' => t('Headline.'),
      'sub_headline' => t('Sub Headline.'), 
      'section_termids' => t('Term from Scondary Section vocabulary.'),
      'crosspostingterms' => t('Term from Scondary Section vocabulary.'),
      'tag_name_list' => t('Terms from Topics vocabulary.'),
      'byline' => t('Users related to this article.'),
      'gallerynid' => t('Node id of the gallery.'),
      'sidebar_title' => t('Title of the legacy sidebar node.'),
      'sidebar_body' => t('Text from the legacy sidebar node.'),
      'pollnid' => t('Node id of the poll.'),
      'body_value' => t('Body of article.'),
      'created_originally' => t('Activation date or created date, whichever is older.'),
      // 'scheduler_publish_on' => t('Activation date field.'),
    );
  	
  	$this->highwaterField = array(
      'name' => 'changed', 		// Column to be used as highwater mark
      'alias' => 'n',           // Table alias containing that column
      'type' => 'int',          // By default, highwater marks are assumed to be lexicographically
                                // sortable (e.g., '2011-05-19 17:53:12'). To properly
                                // deal with integer highwater marks (such as UNIX
                                // timestamps), indicate so here.
    );
  	
  	// Create a MigrateSource object, which manages retrieving the input data.
    $this->source = new MigrateSourceSQL($query, $source_fields, NULL, array('map_joinable' => FALSE));
  	
  	// Set up our destination - node in this case.
    $this->destination = new MigrateDestinationNode('article');
  	
  	// Assign mappings TO destination fields FROM source fields.
    // $this->addFieldMapping('is_new')->defaultValue(TRUE); // sets node id same as D6 nid
    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping('uid', 'uid')->sourceMigration('User')->defaultValue(1);
    $this->addFieldMapping('revision')->defaultValue(TRUE);
    $this->addFieldMapping('revision_uid', 'uid')->sourceMigration('User')->defaultValue(1);
    $this->addFieldMapping('created', 'created_originally');
    $this->addFieldMapping('changed', 'changed');
    $this->addFieldMapping('status', 'status');
    $this->addFieldMapping('promote', 'promote');
    $this->addFieldMapping('sticky', 'sticky');
    $this->addFieldMapping('comment', 'comment');
    $this->addFieldMapping('path', 'url_alias');
    $this->addFieldMapping('language')->defaultValue(LANGUAGE_NONE);
    $this->addFieldMapping('field_subheadline', 'sub_headline');
    
    $this->addFieldMapping('field_section', 'section_termids')
      ->separator(',')
      ->sourceMigration('TermSection')
      ->arguments(array('source_type' => 'tid'))
      ->defaultValue(448);

    $this->addFieldMapping('field_secondarysections', 'crosspostingterms')
      ->separator(',')
      ->sourceMigration('TermSection')
      ->arguments(array('source_type' => 'tid'))
      ->defaultValue(448);
    
    $this->addFieldMapping('field_tags', 'tag_name_list')
     ->separator(',')
  	 ->arguments(array('create_term' => TRUE));
    
    $this->addFieldMapping('field_byline', 'byline')
      ->separator(',')
      ->defaultValue(NULL);
    
    $this->addFieldMapping('field_gallery', 'gallerynid')
  	  ->sourceMigration('NodeGallery')
      ->defaultValue(NULL);
    
    $this->addFieldMapping('field_poll', 'pollnid')
    	->sourceMigration('NodePoll')
    	->defaultValue(NULL);

    // $this->addFieldMapping('scheduler', 'scheduler_publish_on')
    	// ->defaultValue(NULL);
  	
  	// body
  	$body_arguments = MigrateTextFieldHandler::arguments(array('source_field' => 'body_teaser'), array('source_field' => 'full_html_with_paging'));
      $this->addFieldMapping('body', 'body_value')->arguments($body_arguments);
  	
  	// legacy sidebar
  	$sidebar_arguments = MigrateTextFieldHandler::arguments(array('source_field' => 'sidebar_teaser'), array('source_field' => 'sidebar_format'));
      $this->addFieldMapping('field_legacy_sidebar', 'sidebar_body')->arguments($sidebar_arguments);
      $this->addFieldMapping('field_legacy_sidebar_title', 'sidebar_title');
  	
  	// Unmapped source fields
    $this->addUnmigratedSources(array('body','teaser','format','nid','vid','type','language','moderate','tnid','translate','teaser','format','name','created'));
  	
  	// Unmapped destination fields
  	$this->addUnmigratedDestinations(array('scheduler','field_mediacollection','field_files','is_new','nid','field_related_articles','field_footer','field_socialmedia_teaser','field_socialmedia_title'));
  	
  	// Disable application of pathauto during migration
    $this->addFieldMapping('pathauto')->defaultValue(0);
  }
  
  public function prepareRow($current_row) {

  	// load article fields   	
  	$query = Database::getConnection('default', 'd6')->select('content_type_content_article', 'ctca');
  	$query->fields('ctca');
  	$query->condition('ctca.nid', $current_row->nid, '=');
  	$query->join('content_field_body', 'b', ' b.nid = ctca.nid');
  	$query->addField('b', 'field_body_value');
  	$query->addField('b', 'field_body_format');
  	$articlefields = $query->execute()->fetchObject();
  	
  	// body
  	$current_row->body_value = strlen($current_row->body) ? $current_row->body : $articlefields->field_body_value;
  	$current_row->body_teaser = strlen($current_row->teaser) ? $current_row->teaser : $articlefields->field_body_value;
  	$current_row->body_format = strlen($current_row->format) ? 
  		gs_migrate_get_text_format($current_row->format) : gs_migrate_get_text_format($articlefields->field_body_format);
  
  	// gallery; set gallery node id in field_gallery
  	$subquery = Database::getConnection('default','d6')->select('term_data','td');
  	$subquery->fields('td',array('tid'));
  	$subquery->join('term_node','tn','tn.tid = td.tid');
  	$subquery->addField('tn','nid');
  	$subquery->condition('td.vid',6,'='); // "Gallery" vid
  	$subquery->condition('tn.nid',$current_row->nid,'=');
  	$gallerytermid = $subquery->execute()->fetchObject();
  	if (is_object($gallerytermid)) {
  		$current_row->gallerynid = $gallerytermid->tid;
  	}
  
  	// byline field; user reference field, multiple values; content_field_contributor
  	$contributors = Database::getConnection('default', 'd6')->select('content_field_contributor','cfc');
  	$contributors->fields('cfc',array('field_contributor_uid'));
  	$contributors->condition('cfc.nid',$current_row->nid,'=');
  	if ($result = $contributors->execute()->fetchCol()) {
      $current_row->byline = gs_migrate_mappeduids($result);
    }
	
  	// Set the text format for the node.
    $current_row->format = gs_migrate_get_text_format($current_row->format);
  	
  	// Set the url alias for the node.
    $current_row->url_alias = gs_migrate_get_url_alias($current_row->nid);
  	
  	// set Sub-headline
  	$current_row->sub_headline = text_summary($articlefields->field_subheadline_value,NULL,255);
  	
  	// find topics associated with article, migrate tags
  	// $tvid = Topics vocabulary id = 41
  	$current_row->tag_name_list = gs_migration_get_terms($current_row->vid, 41); // comma separated term names
  	
  	// term section
  	// $current_row->section_termids = gs_migration_term_get_parents($articlefields->field_primary_section_value);
  	$current_row->section_termids = $articlefields->field_primary_section_value;
    
    // cross posting terms
  	// $tvid = Secondary Section vocabulary id = 1
    $current_row->crosspostingterms = gs_migration_get_leave_terms($current_row->vid, 1, 'migrate_map_termsection'); // comma separated leave term ids
  	
  	// sidebar content
  	// d7 name: field_legacy_sidebar; 
  	// $articlefields->field_sidebar_nid; 
  	// d6 table and fields: content_type_sidebar, field_sidebar_title_value
  	// d6 body table: node_revisions, body, teaser
  	if ($articlefields->field_sidebar_nid) {
  		$query = Database::getConnection('default', 'd6')->select('node', 'n');
  		$query->condition('n.nid',$articlefields->field_sidebar_nid,'=');
	    $query->join('node_revisions', 'nr', 'n.vid = nr.vid');
	    $query->addField('nr', 'body');
	    $query->addField('nr', 'teaser');
	    $query->addField('nr', 'format');
      $query->join('content_type_sidebar', 'cts', 'cts.nid = nr.nid AND cts.vid = nr.vid');
      $query->addField('cts','field_sidebar_title_value');
  		$sidebar = $query->execute()->fetchObject();
  		
  		$current_row->sidebar_title = $sidebar->field_sidebar_title_value;
  		$current_row->sidebar_body = $sidebar->body;
  		$current_row->sidebar_teaser = $sidebar->teaser;
  		$current_row->sidebar_format = gs_migrate_get_text_format($sidebar->format);
  	}
  	
  	// poll reference
  	if (isset($articlefields->field_poll_nid)) {
  	  $current_row->pollnid = $articlefields->field_poll_nid;
  	}
    
    // created and activation dates
    $query = Database::getConnection('default', 'd6')->select('content_field_activation_date', 'ad');
    $query->addField('ad', 'field_activation_date_value');
    $query->condition('ad.nid',$current_row->nid,'=');
    $activation_date = $query->execute()->fetchField();
    if ($activation_date = strtotime($activation_date)) {
      // created originally
      $current_row->created_originally = $current_row->created > $activation_date ? $activation_date : $current_row->created;
      // activation date
      // $current_row->scheduler_publish_on = $activation_date;
    } else {
      // created
      $current_row->created_originally = $current_row->created;
    }

    return TRUE;
  }

  public function complete($entity, stdClass $row) {
		
  }
}