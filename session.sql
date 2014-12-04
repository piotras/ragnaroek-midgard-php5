#
# Table structure for table 'session'
#

CREATE TABLE session (
   id int(11) unsigned NOT NULL auto_increment,
   sess_key varchar(255) NOT NULL,
   sess_data text NOT NULL,
   expire timestamp(14),
   person int(11) unsigned DEFAULT '0' NOT NULL,
   sitegroup int(11) unsigned DEFAULT '0' NOT NULL,
   PRIMARY KEY (id),
   KEY sessid (sess_key),
   KEY expire (expire)
);

