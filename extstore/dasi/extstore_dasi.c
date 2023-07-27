/*
 * vim:noexpandtab:shiftwidth=8:tabstop=8:
 *
 * Copyright (C) CEA, 2016
 * Author: Philippe Deniel  philippe.deniel@cea.fr
 *
 * contributeur : Philippe DENIEL   philippe.deniel@cea.fr
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * -------------
 */

/* extstore.c
 * KVSNS: implement a dummy object store inside a POSIX directory
 */

#include <sys/time.h> /* for gettimeofday */
#include <ini_config.h>
#include <iosea/extstore.h>
#include "dasi/api/dasi_c.h"


static char dasi_config[MAXPATHLEN];

static dasi_t *dasi;
static const char type[] = "ensemble";
static char fc_date[9]   = "12345678";
static char fc_time[5]   = "0000";
static char version[5]   = "0001";
const char* param_names[4] = {"p", "t", "u", "v"};

int extstore_attach(extstore_id_t *eid)
{
	return 0;
}

int extstore_create(extstore_id_t eid)
{
	return 0;
}

int extstore_new_objectid(extstore_id_t *eid, 
			  unsigned int seedlen,
			  char *seed)
{
	if (!eid || !seed)
		return -EINVAL;

	/* Be careful about printf format: string with known length */
	eid->len = snprintf(eid->data, KLEN, "obj:%.*s", seedlen, seed);

	return 0;
}

int extstore_init(struct collection_item *cfg_items,
		  struct kvsal_ops *kvsalops)
{
	struct collection_item *item;
	struct stat dasi_config_stat;
	int rc;

	item = NULL;
	rc = get_config_item("dasi_store", "dasi_config",
			     cfg_items, &item);
	if (rc != 0)
		return -rc;

	if (item == NULL)
		return -EINVAL;

	strncpy(dasi_config, get_string_config_value(item, NULL),
		MAXPATHLEN);

	if (stat(dasi_config, &dasi_config_stat)) {
		fprintf(stderr, "Specified path '%s' does not exist\n",
			dasi_config);
		return -ENOENT;
	} else if (!S_ISREG(dasi_config_stat.st_mode)) {
		fprintf(stderr, "Specified path '%s' is not a directory\n",
			dasi_config);
		return -ENOTDIR;
	}

	rc = dasi_open(&dasi, dasi_config) ;
	printf("===> dasi_open rc=%d\n", rc);
	return 0;
}

int extstore_del(extstore_id_t *eid)
{
	return 0;
}

int extstore_write(extstore_id_t *eid,
		   off_t offset,
		   size_t buffer_size,
		   void *buffer,
		   bool *fsal_stable,
		   struct stat *stat)
{
	dasi_key_t *key;
	int rc;
	char data[20];
	char str[MAXPATHLEN];

	snprintf(str, MAXPATHLEN, 
		"type=%s,date=%s,time=%s,version=%s,number=%d,step=%d,level=%d,param=%d", 
		type, fc_date, fc_time, version, 3, 3, 3, 3);
        printf("sre=%s\n", str);	
	
	rc = dasi_new_key_from_string(&key, str);
	fprintf(stderr, "dasi_new_query_from_string rc=%d\n", rc);

	rc = dasi_archive(dasi,
			  key, 
			  (void*)data,
                          (long)strlen(data));
	fprintf(stderr, "dasi_archive rc=%d\n", rc) ;

	rc = dasi_free_key(key);
	fprintf(stderr, "dasi_free_query(param) rc=%d\n", rc) ;

	return 0;
}

int extstore_read(extstore_id_t *eid,
		  off_t offset,
		  size_t buffer_size,
		  void *buffer,
		  bool *end_of_file,
		  struct stat *stat)
{
	dasi_query_t *query;
	dasi_retrieve_t* ret;
	int rc;
	char str[MAXPATHLEN];

	snprintf(str, MAXPATHLEN, 
		"type=%s,date=%s,time=%s,version=%s,number=%d,step=%d,level=%d,param=%d", 
		type, fc_date, fc_time, version, 3, 3, 3, 3);
        printf("sre=%s\n", str);	
	
	rc = dasi_new_query_from_string(&query, str);
	fprintf(stderr, "dasi_new_query_from_string rc=%d\n", rc);

	rc = dasi_retrieve(dasi, query, &ret);
	fprintf(stderr, "dasi_retrieve STR rc=%d\n", rc) ;

	rc = dasi_free_query(query);
	fprintf(stderr, "dasi_free_query(param) STR rc=%d\n", rc) ;

	return 0;
}


int extstore_truncate(extstore_id_t *eid,
		      off_t filesize,
		      bool on_obj_store,
		      struct stat *stat)
{
	return 0;
}

int extstore_getattr(extstore_id_t *eid,
		     struct stat *stat)
{
	return 0;
}

int extstore_archive(extstore_id_t *eid)
{
	return -ENOTSUP;
}

int extstore_restore(extstore_id_t *eid)
{
	return -ENOTSUP;
}

int extstore_release(extstore_id_t *eid)
{
	return -ENOTSUP;
}

int extstore_state(extstore_id_t *eid, char *state)
{
	return -ENOTSUP;
}

int extstore_cp_to(int fd,
		   extstore_id_t *eid, 
		   int iolen,
		   size_t filesize)
{
	return -ENOTSUP;
}

int extstore_cp_from(int fd,
		     extstore_id_t *eid, 
		     int iolen,
		     size_t filesize)
{
	return -ENOTSUP;
}

