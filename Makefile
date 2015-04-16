LIBRARY=libmydb.so
EXE=./runner/test_speed
WDIR=./workloads

all:
	gcc my_functions.c mydb.c  -std=c99 -shared -fPIC -o libmydb.so

sophia:
	make -C sophia/
	
clean:
	rm $(LIBRARY)
	rm $(WDIR)/*.yours

test1:
	$(EXE) $(WDIR)/workload.lat $(LIBRARY)
	diff $(WDIR)/workload.lat.out $(WDIR)/workload.lat.out.yours

test2:
	$(EXE) $(WDIR)/workload.old $(LIBRARY)
	diff $(WDIR)/workload.old.out $(WDIR)/workload.old.out.yours

test3:
	$(EXE) $(WDIR)/workload.uni $(LIBRARY)
	diff $(WDIR)/workload.uni.out $(WDIR)/workload.uni.out.yours

test4:
	$(EXE) $(WDIR)/rwd-workloads $(LIBRARY)
	diff $(WDIR)/rwd-workloads.out $(WDIR)/rwd-workloads.out.yours

test:
	make test1
	make test2
	make test3
	make test4




