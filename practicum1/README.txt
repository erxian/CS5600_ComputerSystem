# DESCRIPTION

# This program-managed virtual heap was designed and implemented in pairs,
# Members: Zengping Xu, Mingxin Zhang 
# In our implementation, only when reading or writting data to page, the page need be in physical memory.
# Therefore the detailed workflow for a virtual heap could be:
# (1) Program allocate a page in virtual memory(a virtual page) for one thread, mark allocated page as valid,
#     and load it to the physical memory.
# (2) Program write some data to allocated page, check if the requested page is in physical memory. If in
#     physical memory, just memcpy data to that page frame. If not, page fault occurs, find a free frame in
#     physical memory or replace one using the page replacement algorithm (FIFO)
# (3) Read data from page, similar approach with write data, check if the requested page is in physical
#     memory. If page hit in physical memory, then read data. If page fault, then load page from disk.
# Based on above design, we built a head file pm_vheap.h that contains the declarations of the functions,
# data structures of the pm_vheap module. We also provided source file pm_vheap.c that contains the actual
# implementation of the functions, data structures, and algorithms declared in the pm_vheap.h header file.
# It includes the code for allocating, deallocating, reading, and writing pages, as well as managing the page
# table and physical memory. 


# TEST CASES

# We provide two types test cases: single thread test and multi-thread test, the single thread test
# mainly for functional test, which gives a more straight forward insight of how it works. While the
# multi thread test is used to demonstrate that our heap design works for multi-thread environment
# and it's thread safe.

########################## SINGLE-THREAD #############################
# SINGLE-THREAD TEST APPROACH

$ make clean
$ make
$ ./single_thread_test  // expected output shows below

success allocate page 0
success allocate page 1
success allocate page 2
success allocate page 3
success write data to page 0
success write data to page 1
success write data to page 2
success write data to page 3

Now The Physical Memory is FULL, Let's Allocate a New Page!!

no free frame in physical memory for page 4, need page replacemnt
remove page 0 from FIFO queue, it's frame num is 0
success allocate page 4
success write data to page 4

Let's Read data from page0, some replacement will occur!!
Page Fault: page 0 not in physical memory
no free frame in physical memory for page 0, need page replacemnt
remove page 1 from FIFO queue, it's frame num is 1
Data in page0: Hello, World!

Let's Read data from page1, some replacement will occur!!
Page Fault: page 1 not in physical memory
no free frame in physical memory for page 1, need page replacemnt
remove page 2 from FIFO queue, it's frame num is 2
Data in page1: This is page 1.

Let's Read data from page2, some replacement will occur!!
Page Fault: page 2 not in physical memory
no free frame in physical memory for page 2, need page replacemnt
remove page 3 from FIFO queue, it's frame num is 3
Data in page2: Another page, page 2.

Let's Read data from page3, some replacement will occur!!
Page Fault: page 3 not in physical memory
remove page 4 from FIFO queue, it's frame num is 0
Data in page3: Page 3 is here!

Let's Read data from page4, some replacement will occur!!
Page Fault: page 4 not in physical memory
no free frame in physical memory for page 4, need page replacemnt
remove page 0 from FIFO queue, it's frame num is 1
Data in page4: The last page, page 4.

free page0, since page0 is not in physical memory, warrning occurs
Warning: trying to free an invalid page 0
success free page1
success free page2
success free page3
success free page4


########################## MULTI-THREAD #############################
# MULTI-THREAD TEST APPROACH

# I create 5 threads concurrently executing, each thread will call
# allocate_page(), write_data(), read_data() and free_page().

$ ./multi_thread_test // each run the output will be vary, but similar to below pattern

Worker 0 started
Worker 2 started
Worker 1 started
Worker 3 started
Worker 4 started
success allocate page 0 for worker 0
Start to write data to page 0
success allocate page 1 for worker 4
Start to write data to page 1
success allocate page 2 for worker 1
Start to write data to page 2
success write data to page 2

Start to read data from page 2
success allocate page 3 for worker 3
Start to write data to page 3
success write data to page 3

Start to read data from page 3
success write data to page 0

Start to read data from page 0
Read Data in page3: Hello, World! from worker 3

no free frame in physical memory for page 4, need page replacemnt
remove page 0 from FIFO queue, it's frame num is 0
success write data to page 1

Start to read data from page 1
success allocate page 4 for worker 2
Start to write data to page 4
success write data to page 4

Start to read data from page 4
Read Data in page4: Hello, World! from worker 2

Read Data in page2: Hello, World! from worker 1

success free page 2 from worker 1
success free page 4 from worker 2
Page Fault: page 0 not in physical memory
Read Data in page0: Hello, World! from worker 0

success free page 0 from worker 0
success free page 3 from worker 3
Read Data in page1: Hello, World! from worker 4

success free page 1 from worker 4
