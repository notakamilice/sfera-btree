all:
	gcc mydb.c  -std=c99 -shared -fPIC -o libmydb.so

sophia:
	make -C sophia/




