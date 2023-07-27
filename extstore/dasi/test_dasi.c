#include <stdio.h>
#include <ini_config.h>
#include <iosea/extstore.h>

static struct collection_item *cfg_items;

int main(int argc, char * argv[])
{
	int rc;
	struct collection_item *errors = NULL;


	printf("argc = %d\n", argc) ;

	if(argc <= 1)
		return 0;

	printf("COnfig=%s\n", argv[1]);

	rc = config_from_file("libkvsns", argv[1], &cfg_items,
                              INI_STOP_ON_ERROR, &errors);

        if (rc) {
                fprintf(stderr, "Can't load config %s rc=%d\n",
                        argv[1], rc);

                free_ini_config_errors(errors);
                return -rc;
        }

	rc = extstore_init(cfg_items, NULL);
	fprintf(stderr, "extstore_init: rc=%d\n", rc);

	rc = extstore_write(NULL,
			    0LL,
			    0,
			    NULL,
			    NULL, 
			    NULL);
	fprintf(stderr, "extstore_write: rc=%d\n", rc);
#if 0
	rc = extstore_read(NULL,
			   0LL,
			   0,
			   NULL,
			   NULL, 
			   NULL);
	fprintf(stderr, "extstore_read: rc=%d\n", rc);
#endif
	return 0;
}
