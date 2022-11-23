# ELF-Loader - SO Homework 1
## Copyright 2022 Toaca Alexandra Simona

## Implementation details
- I save the old handler for the **SIGSEGV** signal and use it when needed
- The SIGSEGV is verified to come from an unmapped page with the field
**si_code** from **siginfo_t**. **SEGV_MAPPER** means it comes from an
unmapped page. Another example of si_code would be **SEGV_ACCERR**, which
signifies invalid permissions, and is a case when the old_handler is used.
- Another check, which cannot be done through si_code, is whether
the unmapped address is within a segment of the program. If it is not,
that qualifies as a segmentation fault and the old handler is used.
- The access to the executable is done through its file descriptor, *exec_fd*
- I identified **3 cases** of unmapped memory in a segment:
  - the seg fault is within a page full of data -> the copying of
  an entire page is needed
  - the seg fault is within a page that has some data, but the rest
  exists only in the virtual memory and must be zeroed
  -  the seg fault is within a page that exists only within the virtual
  memory (such as .bss) -> map a page anonymously
- The MAP_ANONYMOUS flag zeroes the page mapped with mmap

## What I have learned / Dificulties
- **mmap** does not zero the memory after the end of file (case 2),
although somewhere on StackOverflow someone said it does.
Learned this the hard way, failing the data_bss test.
I used a test function to gather that information:

```
void test_zeroes(void *start_addr, int n) {
  for (int i = 0; i < n; i++) {
    int zero = *(int *)(void *)(start_addr + i);
    printf("%d %d\n", i, zero);
  }
}
```

- it was hard to find clear documentation on how to use the old signal handler

## Thoughts on the homework
- It was a useful homework, but I was somewhat unprepared for it and
had to spend a significant amount of time understanding concepts such
as signals

