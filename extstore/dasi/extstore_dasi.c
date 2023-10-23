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
const char* param_names[4] = {"p", "t", "u", "v"};

static int extstore_pathtag2dasikey(char *pathtag, char *dasikey)
{
#define DELTA 2
	int pos;

	if (!pathtag || !dasikey)
		return -EINVAL;

	/* turn '/' into ',' */
	/* start at 1 to avoid double slash at the beginning of the pathtag */

	pos=DELTA;
	while (pathtag[pos] != '\0') {
		if (pathtag[pos] == '/')
			dasikey[pos-DELTA] = ',';
		else
			dasikey[pos-DELTA] = pathtag[pos];

		pos += 1;
  	}
	dasikey[pos] = '\0';

	printf("---->>>> pathtag=#%s#, dasikey=#%s#\n", pathtag, dasikey);
	return 0; 
}

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
		printf("Specified path '%s' does not exist\n",
			dasi_config);
		return -ENOENT;
	} else if (!S_ISREG(dasi_config_stat.st_mode)) {
		printf("Specified path '%s' is not a directory\n",
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
	char strkey[MAXPATHLEN];

	printf("extstore_write: eid->pathtag=%s\n", eid->pathtag); 

	rc = extstore_pathtag2dasikey(eid->pathtag, strkey);
	if (rc)
		return rc;

	rc = dasi_new_key_from_string(&key, strkey);
	printf("dasi_new_key_from_string rc=%d\n", rc);

	rc = dasi_archive(dasi,
			  key, 
			  buffer,
                          (long)buffer_size);
	printf("dasi_archive rc=%d\n", rc) ;

	rc = dasi_free_key(key);
	printf("dasi_free_query(param) rc=%d\n", rc) ;

	rc = dasi_flush(dasi);
	printf("dasi_flush rc=%d\n", rc);

	return (int)buffer_size;
}


int extstore_getattr(extstore_id_t *eid,
		     struct stat *stat)
{
	dasi_query_t *query;
	dasi_retrieve_t* retrieve;
	long dlen;
	long rcount = 0;
	int rc;
	char strkey[MAXPATHLEN];

	if( !eid || !stat)
		return -EINVAL;

	/* KVSNS has been modified to handle the pathtag 
	 * eid->pathtag should contain this tag */
	printf("extstore_read: eid->pathtag=%s\n", eid->pathtag); 

	rc = extstore_pathtag2dasikey(eid->pathtag, strkey);
	if (rc)
		return rc;

	rc = dasi_new_query_from_string(&query, strkey);
	printf("dasi_new_query_from_string rc=%d\n", rc);

	rc = dasi_retrieve(dasi, query, &retrieve);
	printf("dasi_retrieve STR rc=%d\n", rc) ;

	rc = dasi_retrieve_count(retrieve, &rcount);
	printf("dasi_retrieve_count STR rc=%d rcount=%ld\n", rc, rcount) ;
	
	while ((rc = dasi_retrieve_next(retrieve)) == DASI_SUCCESS) {
		rc = dasi_retrieve_attrs(retrieve, 
					 NULL, 
					 NULL, 
					 NULL, 
					 &dlen) ;
		printf("dasi_retrieve_attrs rc=%d\n", rc) ;
	}

	rc = dasi_free_query(query);
	printf("dasi_free_query(param) STR rc=%d\n", rc) ;

	rc = dasi_free_retrieve(retrieve);
	printf("dasi_free_retrieve(param) STR rc=%d\n", rc) ;

	stat->st_size = dlen;
	stat->st_mtime = time(NULL);
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
	dasi_retrieve_t* retrieve;
	long dlen;
	long rcount = 0;
	int rc;
	char strkey[MAXPATHLEN];

	/* KVSNS has been modified to handle the pathtag 
	 * eid->pathtag should contain this tag */
	printf("extstore_read: eid->pathtag=%s\n", eid->pathtag); 

	rc = extstore_pathtag2dasikey(eid->pathtag, strkey);
	if (rc)
		return rc;

	rc = dasi_new_query_from_string(&query, strkey);
	printf("dasi_new_query_from_string rc=%d\n", rc);

	rc = dasi_retrieve(dasi, query, &retrieve);
	printf("dasi_retrieve STR rc=%d\n", rc) ;

	rc = dasi_retrieve_count(retrieve, &rcount);
	printf("dasi_retrieve_count STR rc=%d rcount=%ld\n", rc, rcount) ;
	
	while ((rc = dasi_retrieve_next(retrieve)) == DASI_SUCCESS) {
		rc = dasi_retrieve_attrs(retrieve, 
					 NULL, 
					 NULL, 
					 NULL, 
					 &dlen) ;
		printf("dasi_retrieve_attrs rc=%d\n", rc) ;

		rc = dasi_retrieve_read(retrieve, buffer, &dlen);
		printf("dasi_retrieve_read rc=%d\n", rc) ;

		printf("----->>>> BUFFER: #%s#\n", (char *)buffer);
	}

	rc = dasi_free_query(query);
	printf("dasi_free_query(param) STR rc=%d\n", rc) ;

	rc = dasi_free_retrieve(retrieve);
	printf("dasi_free_retrieve(param) STR rc=%d\n", rc) ;

	return dlen;
}


int extstore_truncate(extstore_id_t *eid,
		      off_t filesize,
		      bool on_obj_store,
		      struct stat *stat)
{
	return 0;
}

#if 0
int extstore_getattr__(extstore_id_t *eid,
		     struct stat *stat)
{
	dasi_query_t *query;
	dasi_retrieve_t* retrieve;
	dasi_key_t *key;
	long dlen;
	int rc;
	char strkey[MAXPATHLEN];

	if (!eid || !stat)
		return -EINVAL;

	/* KVSNS has been modified to handle the pathtag 
	 * eid->pathtag should contain this tag */
	printf("extstore_getattr: eid->pathtag=%s\n", eid->pathtag); 

	rc = extstore_pathtag2dasikey(eid->pathtag, strkey);
	if (rc)
		return rc;
	printf("extstore_getattr: strkey: %s\n", strkey);

	rc = dasi_new_query_from_string(&query, strkey);
	printf("dasi_new_query_from_string rc=%d\n", rc);

	rc = dasi_retrieve(dasi, query, &retrieve);
	printf("dasi_retrieve rc=%d\n", rc) ;

	rc = dasi_new_key(&key);
	printf("dasi_new_key rc=%d\n", rc) ;

	do {
		rc = dasi_retrieve_attrs(retrieve, 
					 &key,
					 NULL,
					 NULL,
					 &dlen) ;
		printf(
			"dasi_retrieve_attrs STR rc=%d\n",
			rc) ;
		printf("======> iter  extstore_getattrs, dlen=%ld\n", dlen);
	} while((rc = dasi_retrieve_next(retrieve)) == DASI_SUCCESS) ;

	/* Here rc should be DASI_ITERATION_COMPLETE */
	rc = dasi_free_retrieve(retrieve);
	printf("dasi_free_retrieve rc=%d\n", rc) ;

	stat->st_size = dlen;
	printf("======> extstore_getattrs, dlen=%ld\n", dlen);

	rc = dasi_free_query(query);
	printf("dasi_free_query(param) STR rc=%d\n", rc) ;

	return 0;
}
#endif

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

