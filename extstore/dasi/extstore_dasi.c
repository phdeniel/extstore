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
	char sbuf[5];
	char data[20];

	rc = dasi_new_key(&key);
	fprintf(stderr, "dasi_new_key rc=%d\n", rc) ;

	rc = dasi_key_set(key, "type", type);
	fprintf(stderr, "dasi_set_key(type) rc=%d\n", rc) ;

	rc = dasi_key_set(key, "version", version);
	fprintf(stderr, "dasi_set_key(version) rc=%d\n", rc) ;

	rc = dasi_key_set(key, "date", fc_date);
	fprintf(stderr, "dasi_set_key(date) rc=%d\n", rc) ;

	rc = dasi_key_set(key, "time", fc_time);
	fprintf(stderr, "dasi_set_key(time) rc=%d\n", rc) ;

	snprintf(sbuf, sizeof(sbuf), "%d", 3);
	rc = dasi_key_set(key, "number", sbuf);
	fprintf(stderr, "dasi_set_key(number) rc=%d\n", rc) ;

	snprintf(sbuf, sizeof(sbuf), "%d", 3);
	rc = dasi_key_set(key, "step", sbuf);
	fprintf(stderr, "dasi_set_key(step) rc=%d\n", rc) ;

	snprintf(sbuf, sizeof(sbuf), "%d", 3);
	rc = dasi_key_set(key, "level", sbuf);
	fprintf(stderr, "dasi_set_key(level) rc=%d\n", rc) ;

	rc = dasi_key_set(key, "param", param_names[0]);
	fprintf(stderr, "dasi_set_key(param) rc=%d\n", rc) ;

	snprintf(data, sizeof(data), "%.10lg\n", (double)42); 
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
	char sbuf[5];

	rc = dasi_new_query(&query);
	fprintf(stderr, "dasi_new_key rc=%d\n", rc) ;

	rc = dasi_query_append(query, "type", type);
	fprintf(stderr, "dasi_query_append(type) rc=%d\n", rc) ;

	rc = dasi_query_append(query, "date", fc_date);
	fprintf(stderr, "dasi_query_append(date) rc=%d\n", rc) ;

	rc = dasi_query_append(query, "time", fc_time);
	fprintf(stderr, "dasi_query_append(time) rc=%d\n", rc) ;

	rc = dasi_query_append(query, "version", version);
	fprintf(stderr, "dasi_query_append(version) rc=%d\n", rc) ;

	snprintf(sbuf, sizeof(sbuf), "%d", 3);
	rc = dasi_query_append(query, "number", sbuf);
	fprintf(stderr, "dasi_query_append(number) rc=%d\n", rc) ;

	snprintf(sbuf, sizeof(sbuf), "%d", 3);
	rc = dasi_query_append(query, "step", sbuf);
	fprintf(stderr, "dasi_query_append(step) rc=%d\n", rc) ;

	snprintf(sbuf, sizeof(sbuf), "%d", 3);
	rc = dasi_query_append(query, "level", sbuf);
	fprintf(stderr, "dasi_query_append(level) rc=%d\n", rc) ;

	snprintf(sbuf, sizeof(sbuf), "%d", 3);
	rc = dasi_query_append(query, "param", sbuf);
	fprintf(stderr, "dasi_query_append(param) rc=%d\n", rc) ;

	rc = dasi_retrieve(dasi, query, &ret);
	fprintf(stderr, "dasi_retrieve rc=%d\n", rc) ;

	rc = dasi_free_query(query);
	fprintf(stderr, "dasi_free_query(param) rc=%d\n", rc) ;

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

