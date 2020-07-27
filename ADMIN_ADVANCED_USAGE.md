# Advanced Options

## Server Command Line Options

### Viewing Contents from Web Pages

The command line arguments are `d /path/to/view-remote-dir`. Included is [an example for Pinterest](scripts/remote-dir-viewers/pinterest.com) - I think you may need to supply your own cookies, however.

This command line argument will enable web users to request the site via `Filesystem Dir` - they can type `pinterest [QUERY]`, and the script will place the pins from that query into the file page, as though they were files on the local filesystem.

# Advanced Configuration

## Multiple Servers on the Same Database with Different Permissions

Simply rename all the core tables:

	RENAME TABLE tag TO _tag;
	RENAME TABLE era TO _era;
	RENAME TABLE file TO _file;
	RENAME TABLE dir TO _dir;
	RENAME TABLE device TO _device;

and create security views (that emulate row-level security) (based on https://mariadb.com/resources/blog/protect-your-data-row-level-security-in-mariadb-10-0/):

	CREATE TABLE IF NOT EXISTS permission (
		id BIGINT UNSIGNED NOT NULL PRIMARY KEY,
		name VARCHAR(32) NOT NULL UNIQUE KEY
	);
	CREATE TABLE IF NOT EXISTS user2permissions (
		user VARBINARY(32) NOT NULL PRIMARY KEY,
		permissions BIGINT UNSIGNED NOT NULL
	);
	CREATE SQL SECURITY DEFINER
	VIEW tag
	AS
		SELECT *
		FROM _tag
		WHERE id NOT IN (
			SELECT t2pt.tag
			FROM user u
			JOIN user2blacklist_tag u2ht ON u2ht.user=u.id
			JOIN tag2parent_tree t2pt ON t2pt.parent=u2ht.tag
			WHERE u.name=SESSION_USER()
		)
	;
	CREATE SQL SECURITY DEFINER
	VIEW file
	AS
		SELECT *
		FROM _file
		WHERE id NOT IN (
			SELECT f2t.file
			FROM user u
			JOIN user2blacklist_tag u2ht ON u2ht.user=u.id
			JOIN tag2parent_tree t2pt ON t2pt.parent=u2ht.tag
			JOIN file2tag f2t ON f2t.tag=t2pt.tag
			WHERE u.name=SESSION_USER()
		)
	;
	CREATE SQL SECURITY DEFINER
	VIEW dir
	AS
		SELECT d.*
		FROM _dir d
		JOIN user2permissions u2p ON u2p.user=SESSION_USER()
		WHERE u2p.permissions & d.permissions = d.permissions
	;
	CREATE SQL SECURITY DEFINER
	VIEW device
	AS
		SELECT d.*
		FROM _device d
		JOIN user2permissions u2p ON u2p.user=SESSION_USER()
		WHERE u2p.permissions & d.permissions = d.permissions
	;

The client servers should be given permission to access these views, and any permissions to the underlying tables (now prefixed with underscores) revoked.
