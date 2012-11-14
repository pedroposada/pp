<?php

/**
 * Migration class for Section terms
 */
class TermArticleMigration extends PPMigration {
  public function __construct() {
    parent::__construct();
    // Human-friendly description of your migration process. Be as detailed as you like.
    $this->description = t('Migrate blog tags.');

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'tid' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'D6 Unique Term ID',
          'alias' => 'td',
        )
      ),
      MigrateDestinationTerm::getKeySchema()
    );

    // We are getting data from tables in the Drupal default database - first,
    // set up a query for this data.
    $query = Database::getConnection('default', 'd6')->select('term_data', 'td');
    $query->fields('td', array('tid', 'vid', 'name', 'description', 'weight'));
    $query->condition('td.vid', 1, '='); // "Blog" vocabulary id
    $query->join('term_hierarchy', 'th', 'td.tid = th.tid');
    $query->addField('th', 'parent');
    $query->orderBy('th.parent', 'ASC');
    $query->orderBy('td.tid', 'ASC');
	
		// document source fields
		$source_fields = array(
	    'tid' => t('Term ID.'),
	    'vid' => t('Vocabulary ID.'),
	    'name' => t('Name of term.'),
	    'description' => t('Description of term.'),
	    'weight' => t('Sort order for term.'),
	    'parent' => t('Term id of the parent term.'),
	  );
	
		// use our own count query
    $count_query = Database::getConnection('default', 'd6')->select('term_data', 'td');
    $count_query->condition('td.vid', 1, '='); // "Secondary Sections" vocabulary id
    $count_query->addExpression('COUNT(td.tid)', 'cnt');

    // Create a MigrateSource object, which manages retrieving the input data.
    $this->source = new MigrateSourceSQL($query, $source_fields, $count_query, array('map_joinable' => FALSE));

    // Set up our destination - term in this case. 
    // Takes one parameter; in this case the machine name of the vocabulary.
    $this->destination = new MigrateDestinationTerm('tags');

    // Assign mappings TO destination fields FROM source fields.
    $this->addFieldMapping('name', 'name');
    $this->addFieldMapping('description', 'description');
    $this->addFieldMapping('format')->defaultValue('plain_text');
    $this->addFieldMapping('weight', 'weight');
    $this->addFieldMapping('parent', 'parent')->sourceMigration($this->getMachineName());

    // Unmapped source fields
    $this->addUnmigratedSources(array('vid'));

    // Unmapped destination fields
    $this->addUnmigratedDestinations(array('path', 'parent_name'));
	
  }

  public function prepareRow($current_row) {
  	
		// drupal 7 top level terms
    // if ($current_row->parent == 0) {
      	// unset($current_row->parent);
    // }else{
		// // check if parent is in map-table; if not then skip row
	  	// $dest = db_select('migrate_map_termsection', 'ts')
	      // ->fields('ts', array('destid1'))
	      // ->condition('ts.sourceid1', $current_row->parent, '=')
		  // ->execute()->fetchObject();
		// return is_object($dest) && !empty($dest->destid1) ? TRUE : FALSE;
    // }

		return TRUE;
	
  }
  
  public function prepare($entity, stdClass $row) {
  	
  }
  
}