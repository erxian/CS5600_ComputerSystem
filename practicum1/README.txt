success allocate page 0
success allocate page 1
success allocate page 2
success allocate page 3
Page success: find page 0 in physical memory
success write data to page 0
Page success: find page 1 in physical memory
success write data to page 1
Page success: find page 2 in physical memory
success write data to page 2
Page success: find page 3 in physical memory
success write data to page 3

Now The Physical Memory is FULL, Let's Allocate a New Page!!

no free frame in physical memory for page 4, need page replacemnt
remove page 0 from FIFO queue, it's frame num is 0
success allocate page 4
Page success: find page 4 in physical memory
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
no free frame in physical memory for page 3, need page replacemnt
remove page 4 from FIFO queue, it's frame num is 0
Data in page3: Page 3 is here!

Let's Read data from page4, some replacement will occur!!
Page Fault: page 4 not in physical memory
no free frame in physical memory for page 4, need page replacemnt
remove page 0 from FIFO queue, it's frame num is 1
Data in page4: The last page, page 4.
