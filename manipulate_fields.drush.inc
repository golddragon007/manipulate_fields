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
    'types' => array(
      'description' => dt('Comma separated list of the entity/field types'),
      'value' => 'required',
      'example-value' => array(
        'drush mf lock --types=all' => dt('Lock all the fields.'),
        'drush mf --types=all' => dt('Manipulate all the fields.'),
        'drush mf --types=node' => dt('Manipulate only the fields related to node.'),
        'drush mf --types=node,field_collection' => dt('Manipulate the fields related to the node and field_collection.'),
      ),
    ),
  );

  if (function_exists('entity_get_info')) {
    $entity_types = drush_manipulate_fields_manipulate_fields_get_entity_type_options();

    foreach ($entity_types as $machine_name => $entity_type_name) {
      $options[$machine_name] = dt('Modify the !type_name entity/field type', array('!type_name' => $entity_type_name));
    }
  }

  $items = array();
  $items['manipulate-fields'] = array(
    'description' => dt('Lock/Unlock any type of fields.'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
    'arguments' => array(
      'locked' => dt('Manipulation type, can be lock and unlock. If not present it will be asked later.'),
    ),
    'options' => $options,
    'examples' => array(
      'drush mf unlock' => dt('Unlocking selected type of fields/entities.'),
      'drush mf lock' => dt('Locking selected type of fields/entities.'),
      'drush mf lock --node' => dt("Locking fields which relates to nodes (doesn't lock fields which are inside a field collection)."),
      'drush mf lock --node --field_collection' => dt('Locking fields which relates to nodes and field_collection.'),
      'drush mf lock --all' => dt('Locks all the fields.'),
      'drush mf lock --types=all' => dt('Lock all the fields.'),
      'drush mf --types=all' => dt('Manipulate all the fields.'),
      'drush mf --types=node' => dt('Manipulate only the fields related to node.'),
      'drush mf --types=node,field_collection' => dt('Manipulate the fields related to the node and field_collection.'),
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
    return dt('Lock/Unlock the selected entity/field types.
- If this module was installed as a drupal module, you can use directly the machine names of the entity types.
- If it was installed as a Drush module, you need to use --types= option to define the entity types.');
  }
}

/**
 * Callback for the lock-fields command.
 *
 * @param string $locked
 *   Argument from drush command, which says if it's locked or not or not
 *   present.
 *
 * @return bool
 *   Returns false if user cancelled the command otherwise true.
 */
function drush_manipulate_fields($locked = NULL) {
  $entity_choices = drush_manipulate_fields_manipulate_fields_get_entity_types();

  $locked_int = drush_manipulate_fields_manipulate_fields_get_locked_value($locked);

  if ($entity_choices === FALSE || $locked_int === FALSE) {
    return drush_user_abort();
  }

  if (in_array('all', $entity_choices, FALSE)) {
    // If we update all.
    db_query('
      UPDATE field_config
      SET locked = :locked
    ', array(':locked' => $locked_int));

    drush_log(dt('Field manipulation done.'), LogLevel::SUCCESS);
    drush_manipulate_fields_manipulate_fields_clear_entity_cache($entity_choices);
    return TRUE;
  }

  if (in_array('node', $entity_choices, FALSE)) {
    // If we update node, than we need to update all type of fields which
    // connected to a node bundle.
    db_query('
      UPDATE field_config fc
      INNER JOIN field_config_instance fci ON fci.field_name = fc.field_name
      INNER JOIN node_type nt ON nt.type = fci.bundle
      SET fc.locked = :locked
    ', array(':locked' => $locked_int));

    drush_log(dt('Field manipulation done for node.'), LogLevel::SUCCESS);
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
    ', array(':locked' => $locked_int, ':entity_types' => $entity_choices));

    drush_log(dt('Field manipulation done for !types.', array('!types' => implode(', ', $entity_choices))), LogLevel::SUCCESS);
    drush_manipulate_fields_manipulate_fields_clear_entity_cache($entity_choices);
  }

  return TRUE;
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
    drush_log(dt('All entity caches was successfully cleared.'), LogLevel::SUCCESS);
    return;
  }

  foreach ($entity_choices as $entity_type_machine_name) {
    if (db_table_exists('cache_entity_' . $entity_type_machine_name)) {
      cache_clear_all('*', 'cache_entity_' . $entity_type_machine_name, TRUE);
    }
  }
  drush_log(dt('The following entity caches was successfully cleared: !types', array('!types' => implode(', ', $entity_choices))), LogLevel::SUCCESS);
}

/**
 * Returns the operation if we should lock or unlock a field.
 *
 * @param string $locked
 *   Argument from drush command, which says if it's locked or not or not
 *   present.
 *
 * @return int|bool
 *   1 if we lock, 0 if we unlock. If it's false, operation was cancelled.
 */
function drush_manipulate_fields_manipulate_fields_get_locked_value($locked) {
  if ($locked === 'lock') {
    return 1;
  }
  if ($locked === 'unlock') {
    return 0;
  }

  $lock_options = array(
    'lock' => dt('Lock fields'),
    'unlock' => dt('Unlock fields'),
  );
  $locked_choice = drush_choice($lock_options, dt('What do you want to do?'));
  if ($locked_choice === FALSE) {
    return FALSE;
  }

  if ($locked_choice === 'lock') {
    return 1;
  }

  return 0;
}

/**
 * Returns the entity/field types which we should lock or unlock.
 *
 * @return array|false
 *   Returns and array with the entity/field types which should be modified.
 *   If it's false, the action was cancelled.
 */
function drush_manipulate_fields_manipulate_fields_get_entity_types() {
  $entity_type_options = drush_manipulate_fields_manipulate_fields_get_entity_type_options();

  $command_line_options = [];
  foreach ($entity_type_options as $entity_type_option_key => $entity_type_option) {
    if (drush_get_option($entity_type_option_key)) {
      $command_line_options[] = $entity_type_option_key;
    }
  }

  if ($types = drush_get_option('types')) {
    $command_line_options = array_unique(array_merge(explode(',', $types), $command_line_options));
  }

  if (count($command_line_options) > 0) {
    return $command_line_options;
  }

  $entity_choice = drush_choice($entity_type_options, dt('What entity type do you want to manipulate?'));
  if ($entity_choice === FALSE) {
    return FALSE;
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
    'all' => dt('All types [all]'),
    'node' => dt('Node [node]'),
  );
  $used_fields_query = db_query("
    SELECT entity_type FROM field_config_instance fci
      LEFT JOIN node_type nt ON nt.type = fci.bundle AND fci.entity_type != 'profile2' 
    WHERE nt.type IS NULL
    GROUP BY entity_type
  ");
  while ($row = $used_fields_query->fetchAssoc()) {
    $entity_type_options[$row['entity_type']] = isset($entities[$row['entity_type']]['label']) ? dt('!entity_name [!entity_machine_name]', array('!entity_name' => $entities[$row['entity_type']]['label'], '!entity_machine_name' => $row['entity_type'])) : $row['entity_type'];
  }

  return $entity_type_options;
}
