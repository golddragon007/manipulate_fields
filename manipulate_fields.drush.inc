<?php

/**
 * @file
 * Devel module drush integration for manipulate-fields command.
 */

use Drush\Log\LogLevel;

/**
 * Implements hook_drush_command().
 *
 * Returns an associative array describing your command(s).
 *
 * @see drush_parse_command()
 */
function manipulate_fields_drush_command() {
  $options = array(
    'lock' => 'Locks the selected type of fields',
    'unlock' => 'Unlocks the selected type of fields',
  );

  $entity_types = drush_manipulate_fields_manipulate_fields_get_entity_type_options();

  foreach ($entity_types as $machine_name => $entity_type_name) {
    $options[$machine_name] = "Modify the $entity_type_name entity/field type";
  }

  $items = array();
  $items['manipulate-fields'] = array(
    'description' => 'Lock/Unlock any type of fields.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_DATABASE,
    'options' => $options,
    'examples' => array(
      'drush mf --lock' => 'Locking selected type of fields/entities.',
      'drush mf --node --lock' => "Locking fields which relates to nodes (doesn't lock fields which are inside a field collection).",
      'drush mf --node --field_collection --lock' => 'Locking fields which relates to nodes and field_collection.',
      'drush mf --all --lock' => 'Locks all the fields.',
    ),
    'aliases' => array('mf'),
  );

  return $items;
}

/**
 * Implements hook_drush_help().
 */
function manipulate_fields_drush_help($section) {
  if ($section === 'drush:manipulate-fields') {
    return dt('Lock/Unlock the content type fields');
  }
}

/**
 * Callback for the lock-fields command.
 */
function drush_manipulate_fields_manipulate_fields() {
  $entity_choices = drush_manipulate_fields_manipulate_fields_get_entity_types();

  $locked = drush_manipulate_fields_manipulate_fields_get_locked_value();

  if (in_array('all', $entity_choices, FALSE)) {
    // If we update all.
    db_query('
      UPDATE field_config
      SET locked = :locked
    ', array(':locked' => $locked));

    drush_log('Field manipulation done.', LogLevel::SUCCESS);
    drush_manipulate_fields_manipulate_fields_clear_entity_cache($entity_choices);
    return;
  }

  if (in_array('node', $entity_choices, FALSE)) {
    // If we update node, than we need to update all type of fields which
    // connected to a node bundle.
    db_query('
      UPDATE field_config fc
      INNER JOIN field_config_instance fci ON fci.field_name = fc.field_name
      INNER JOIN node_type nt ON nt.type = fci.bundle
      SET fc.locked = :locked
    ', array(':locked' => $locked));

    drush_log('Field manipulation done for node.', LogLevel::SUCCESS);
    drush_manipulate_fields_manipulate_fields_clear_entity_cache(array('node'));

    // Remove the node from the list to not modify later on.
    unset($entity_choices[array_search('node', $entity_choices, FALSE)]);
  }

  if (count($entity_choices) > 0) {
    // Every other case, we just update the specific entity type.
    db_query('
      UPDATE field_config fc
      INNER JOIN field_config_instance fci ON fci.field_name = fc.field_name
      SET fc.locked = :locked
      WHERE fci.entity_type IN (:entity_types)
    ', array(':locked' => $locked, ':entity_types' => $entity_choices));

    drush_log('Field manipulation done for ' . implode(', ', $entity_choices) . '.', LogLevel::SUCCESS);
    drush_manipulate_fields_manipulate_fields_clear_entity_cache($entity_choices);
  }
}

/**
 * Helper function to clear entity caches, only the necessary ones.
 *
 * @param array $entity_choices
 *   An array of clearable entity caches. (value: entity machine name).
 */
function drush_manipulate_fields_manipulate_fields_clear_entity_cache($entity_choices) {
  if (in_array('all', $entity_choices, FALSE)) {
    $entity_types = drush_manipulate_fields_manipulate_fields_get_entity_type_options();
    foreach ($entity_types as $entity_type_machine_name => $entity_type_name) {
      if (db_table_exists('cache_entity_' . $entity_type_machine_name)) {
        cache_clear_all('*', 'cache_entity_' . $entity_type_machine_name, TRUE);
      }
    }
    drush_log('All entity caches was successfully cleared.', LogLevel::SUCCESS);
    return;
  }

  foreach ($entity_choices as $entity_type_machine_name) {
    if (db_table_exists('cache_entity_' . $entity_type_machine_name)) {
      cache_clear_all('*', 'cache_entity_' . $entity_type_machine_name, TRUE);
    }
  }
  drush_log('The following entity caches was successfully cleared: ' . implode(', ', $entity_choices), LogLevel::SUCCESS);
}

/**
 * Returns the operation if we should lock or unlock a field.
 *
 * @return int
 *   1 if we lock, 0 if we unlock.
 */
function drush_manipulate_fields_manipulate_fields_get_locked_value() {
  if (drush_get_option('lock')) {
    return 1;
  }
  if (drush_get_option('unlock')) {
    return 0;
  }

  $lock_options = array(
    'lock' => 'Lock fields',
    'unlock' => 'Unlock fields',
  );
  $locked_choice = drush_choice($lock_options, dt('What do you want to do?'));
  if ($locked_choice === FALSE) {
    drush_log('Cancelled.', LogLevel::CANCEL);
    exit();
  }

  if ($locked_choice === 'lock') {
    return 1;
  }

  return 0;
}

/**
 * Returns the entity/field types which we should lock or unlock.
 *
 * @return array
 *   Returns and array with the entity/field types which should be modified.
 */
function drush_manipulate_fields_manipulate_fields_get_entity_types() {
  $entity_type_options = drush_manipulate_fields_manipulate_fields_get_entity_type_options();

  $command_line_options = [];
  foreach ($entity_type_options as $entity_type_option_key => $entity_type_option) {
    if (drush_get_option($entity_type_option_key)) {
      $command_line_options[] = $entity_type_option_key;
    }
  }

  if (count($command_line_options) > 0) {
    return $command_line_options;
  }

  $entity_choice = drush_choice($entity_type_options, dt('What entity type do you want to manipulate?'));
  if ($entity_choice === FALSE) {
    drush_log('Cancelled.', LogLevel::CANCEL);
    exit();
  }

  return array($entity_choice);
}

/**
 * Returns the types of the selectable entity/field types.
 *
 * @return array
 *   An array with the selectable entity/field types, key is machine name, value
 *   is the human readable name.
 */
function drush_manipulate_fields_manipulate_fields_get_entity_type_options() {
  $entities = entity_get_info();
  $entity_type_options = array(
    'all' => 'All types',
    'node' => 'Node',
  );
  $used_fields_query = db_query("
    SELECT entity_type FROM field_config_instance fci
      LEFT JOIN node_type nt ON nt.type = fci.bundle AND fci.entity_type != 'profile2' 
    WHERE nt.type IS NULL
    GROUP BY entity_type
  ");
  while ($row = $used_fields_query->fetchAssoc()) {
    $entity_type_options[$row['entity_type']] = isset($entities[$row['entity_type']]['label']) ? $entities[$row['entity_type']]['label'] : $row['entity_type'];
  }

  return $entity_type_options;
}
