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
    $query->fields('n', array('nid', 'vid', 'type', 'title', 'uid', 'status', 'created', 'changed', 'comment', 'promote', 'moderate', 'sticky'));
    $query->condition('n.type', 'blog_article', '=');
    $query->join('node_revisions', 'nr', 'n.vid = nr.vid');
    $query->addField('nr', 'body');
    $query->addField('nr', 'teaser');
    $query->addField('nr', 'format');
    $query->orderBy('n.changed');
  	
  	$source_fields = array(
      'nid' => t('The node ID of the article'),
      'url_alias' => t('The url alias of the node.'),
      'title' => t('Headline.'),
      'tag_id_list'=> t('Comma sepparated list of tag ids'),
      'format' => t('Body format'),
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
    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping('uid')->defaultValue(1);
    $this->addFieldMapping('revision')->defaultValue(TRUE);
    $this->addFieldMapping('revision_uid', 'uid')->defaultValue(1);
    $this->addFieldMapping('created', 'created');
    $this->addFieldMapping('changed', 'changed');
    $this->addFieldMapping('status', 'status');
    $this->addFieldMapping('promote', 'promote');
    $this->addFieldMapping('sticky', 'sticky');
    $this->addFieldMapping('comment', 'comment');
    $this->addFieldMapping('path', 'url_alias');
    $this->addFieldMapping('language')->defaultValue(LANGUAGE_NONE);
    
    // terms
    $this->addFieldMapping('field_tags', 'tag_id_list')->separator(',');
    $this->addFieldMapping('field_tags:source_type')->defaultValue('tid');
    $this->addFieldMapping('field_tags:create_term')->defaultValue(FALSE);

  	// body
    $this->addFieldMapping('body', 'body');
    $this->addFieldMapping('body:summary', 'teaser');
    $this->addFieldMapping('body:format')->defaultValue('full_html');
    $this->addFieldMapping('body:language')->defaultValue(LANGUAGE_NONE);
  	
  	// Unmapped source fields
    $this->addUnmigratedSources(array('body','teaser','nid','vid','type',
			'moderate','teaser','created','format'));
  	
  	// Unmapped destination fields
  	$this->addUnmigratedDestinations(array('is_new','nid','log','tnid',
  		'field_image',
  		'field_image:file_class',
			'field_image:language',
			'field_image:destination_dir',
			'field_image:destination_file',
			'field_image:file_replace',
			'field_image:preserve_files',
			'field_image:source_dir',
			'field_image:alt',
			'field_image:title'));
  	
  	// Disable application of pathauto during migration
    // $this->addFieldMapping('pathauto')->defaultValue(0);
  }
  
  public function prepareRow($current_row) {

		// comma sepparated list of terms
		$q = Database::getConnection('default', 'd6')->select('term_node', 'tn');
		$q->condition('tn.nid',$current_row->nid);
		$q->addField('tn', 'tid');
		$result = $q->execute()->fetchCol();
		$current_row->tag_id_list = ppmig_mappedterm($result);
		
		// url alias
		$q = Database::getConnection('default', 'd6')->select('url_alias', 'ua');
		$q->condition('ua.src',"node/{$current_row->nid}");
		$q->addField('ua', 'dst');
		$current_row->url_alias = $q->execute()->fetchfield();
		
    return TRUE;
  }

  public function complete($entity, stdClass $row) {
		
  }
}