THIS IS NOT A MODULE. YOU CAN'T ENABLE IT. FOLLOW THE INSTRUCTIONS.

This module helps to lock and unlock already existing fields directly inside the database, so you don't need to manually rewrite inside the feature files for example. This module also supports custom entities, locking or unlocking multiple type of entities/fields in one shot.

When you need this module?
If you use locked fields in production and you need to unlock them in an easy way.

Does it work with multisite installation?
Normally yes, but you need to use correct drush alias or cd into the site/ directory.

Can I unlock/lock multiple entity/field types?
Yes, if you give as an option for the command like: drush mf --node --profile2 --lock . In this case all the fields related to node and profile2 will be locked.

How can I lock/unlock every kind of fields?
Use drush mf --all --lock or drush mf --all --unlock command.

Do I need to clear cache after this?
Normally not, the module clears all the related entity caches which are affected by the manipulation.

How To Use Manipulate Fields With Drush
1. You can just drush @none dl manipulate_fields-7.x and drush will download it into your .drush folder. (Alternately, you can obtain the package another way and copy the folder into .drush yourself.)
2. Make a backup of your database (it's required for security reasons, without this, it would hard to find out an issue/bug).
3. Clear Drush own cache with drush cc drush
4. On a multisite install, either use correct drush alias, or cd into the site you're manipulating, as in cd sites/mymultisite
5. Run drush mf or drush @sitename mf if you want questions, or:
6. Run drush @sitename mf --[entity_machine_name] --[lock|unlock] to run without questioning

This package comes with no guarantee explicit or implicit.

There's no reason it should do any harm to your install. But there can be lots of things wrong with a system, and the registry problem is not the fix for everything.