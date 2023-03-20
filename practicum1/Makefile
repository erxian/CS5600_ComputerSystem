all : single_thread_test multi_thread_test 

single_thread_test: single_thread_test.c 
	gcc -o single_thread_test pm_vheap.c single_thread_test.c

multi_thread_test: multi_thread_test.c
	gcc -o multi_thread_test pm_vheap.c multi_thread_test.c -lpthread

clean:
	rm single_thread_test
	rm multi_thread_test
