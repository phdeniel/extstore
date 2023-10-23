Configuration of the test bed of extstore/DASI 
==============================================

Test bed via kvsns_shell
------------------------
Configuration file is referenced by envvar KVSNS_CONFIG
    export KVSNS_CONFIG=/home/phil/DEV/iosea-namespace/extstore/extstore/dasi/kvsns.ini

This configuration file should contain an explicit reference to the dasi.yaml fil
which contains the configuration/schema of the DASI library


[dasi_store]
	dasi_config = /home/phil/DEV/iosea-namespace/extstore/extstore/dasi/dasi.yml
    
The extstore_lib should point to the right dasi extstore library
    extstore_lib = /home/phil/DEV/iosea-namespace/b/extstore/extstore/dasi/libextstore_dasi.so 

__RUN the TEST__
Go to the kvsns_shell build directory
cd /home/phil/DEV/iosea-namespace/b/kvsns-base/kvsns_shell

make links

Cleanup the KVS and then create the namespace

ALWAYS REBUILD FROM /home/phil/DEV/iosea-namespace/b

Create stuffs
1076	 ./ns_mkdir a
1077	 ./ns_cd a
1078	 ./ns_ls
1079	 ./ns_mkdir b 
1080	 ./ns_cd b
1081	 ./ns_mkdir c
1082	 ./ns_cd c
1083	 ./ns_pwd
1084	 ./ns_ls

Copy something
 ./kvsns_cp Makefile kvsns://a/b/c/file 

__FIRST TEST via kvsns_shell__
Create the tree
    for i in type=ensemble date=12345678 time=0000 version=0001 number=3 step=3 level=3; do 
        ./ns_mkdir $i; 
        ./ns_cd $i ;  
    done

The last level is a file
    ./kvsns_cp Makefile kvsns://type=ensemble/date=12345678/time=0000/version=0001/number=3/step=3/level=3/param=3

Dans l'autre sens on fait la lecture avec 
    ./kvsns_cp  kvsns://type=ensemble/date=12345678/time=0000/version=0001/number=3/step=3/level=3/param=3 retour 


Voir le contenud es records dans DASI
-------------------------------------

Set the envvar for easier CLI
export DASICONF=/home/phil/DEV/iosea-namespace/extstore/extstore/dasi/dasi.yml

    dasi-put --config==$DASICONF type=ensemble,date=12345678,time=0000,version=0001,number=3,step=3,level=3,param=3 toto1 
    dasi-get --config==$DASICONF type=ensemble,date=12345678,time=0000,version=0001,number=3,step=3,level=3,param=3 toto2

Faire un record et ensuite l'attacher au FS
-------------------------------------------
Ecrire toto1
     dasi-put --config==$DASICONF type=ensemble,date=12345678,time=0000,version=0001,number=3,step=3,level=3,param=6 toto1 

Relire toto1 dans toto52 pour vérifier
	 dasi-get --config==$DASICONF type=ensemble,date=12345678,time=0000,version=0001,number=3,step=3,level=3,param=6 toto52

Attacher le record DASI, l'object ID a peu d'importance puisque c'est le chemin qui compte
    ./kvsns_attach -p /type=ensemble/date=12345678/time=0000/version=0001/number=3/step=3/level=3/param=6  -o 12345678

Ensuite je peux relire le truc avec kvsns_cp
    ./kvsns_cp kvsns://type=ensemble/date=12345678/time=0000/version=0001/number=3/step=3/level=3/param=6  toto56


