THIS CAN BE ENABLED AS A MODULE, BUT CAN BE USED GLOBALLY TOO. FOLLOW THE INSTRUCTIONS.

This module helps to lock and unlock already existing fields directly inside the database, so you don't need to manually rewrite inside the feature files for example. This module also supports custom entities, locking or unlocking multiple type of entities/fields in one shot.

When you need this module?
If you use locked fields in production and you need to unlock them in an easy way.

Does it work with multisite installation?
Normally yes, but you need to use correct drush alias or cd into the site/ directory.

What's the difference if I install on site or globally?
If you install on site you will have the machine names as an options and you can use directly without the '--types=' string.

What's the possible types?
If you don't know and you installed globally than run first without the '--types=' string and it will list the possible options.

Can I use the types option in case of locally installed version?
Yes.

Can I use the entity types as an option in globally installed version?
No.

Can I unlock/lock multiple entity/field types?
Yes, if you give as an option for the command like: drush mf lock --node --profile2 (local) or drush mf lock --types=node,profile2 (global). In this case all the fields related to node and profile2 will be locked.

How can I lock/unlock every kind of fields?
GLOBALLY: Use drush mf lock --types=all or drush mf unlock --types=all command.
LOCALLY: Use drush mf lock --all or drush mf unlock --all command.

Do I need to clear cache after this?
Normally not, the module clears all the related entity caches which are affected by the manipulation.

How To Use Manipulate Fields With Drush GLOBALLY
1. You can just drush @none dl manipulate_fields-7.x and drush will download it into your .drush folder. (Alternately, you can obtain the package another way and copy the folder into .drush yourself.)
2. Make a backup of your database (it's required for security reasons, without this, it would hard to find out an issue/bug).
3. Clear Drush own cache with drush cc drush
4. On a multisite install, either use correct drush alias, or cd into the site you're manipulating, as in cd sites/mymultisite
5a. Run drush mf or drush @sitename mf if you want questions, or:
5b. Run drush @sitename mf [lock|unlock] --types=[entity_machine_name] to run without questioning

How To Use Manipulate Fields With Drush LOCALLY
1. You can just drush en manipulate_fields-7.x and drush will download and enable it. (Alternately, you can obtain the package another way and copy the modules folder.)
2. Make a backup of your database (it's required for security reasons, without this, it would hard to find out an issue/bug).
3. Clear Drush own cache with drush cc drush
4. On a multisite install, either use correct drush alias, or cd into the site you're manipulating, as in cd sites/mymultisite
5a. Run drush mf or drush @sitename mf if you want questions, or:
5b. Run drush @sitename mf [lock|unlock] --[entity_machine_name] to run without questioning

This package comes with no guarantee explicit or implicit.

There's no reason it should do any harm to your install. But there can be lots of things wrong with a system, and the registry problem is not the fix for everything.