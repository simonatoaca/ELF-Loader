/*
 * Loader Implementation
 *
 * 2018, Operating Systems
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "exec_parser.h"
#include "loader.h"

static so_exec_t *exec;
static int exec_fd;
static struct sigaction action, old_action;

int find_segment(void *addr)
{
	if ((uintptr_t)addr < exec->base_addr)
		return -1;

	for (unsigned int curr_segm = 0; curr_segm < exec->segments_no; curr_segm++) {
		so_seg_t segm = exec->segments[curr_segm];

		if ((uintptr_t)addr >= segm.vaddr && (uintptr_t)addr < segm.vaddr + segm.mem_size) {
			return curr_segm;
		}
	}

	return -1;
}

void sigsegv_handler(int signum, siginfo_t *info, void *ucontext)
{
	// Check signal type
	// If it is not SEGV_MAPPER it means invalid permissions or invalid access
	if (info->si_code != SEGV_MAPERR) {
		sigaction(SIGSEGV, &old_action, NULL);
		return;
	}

	// The unmapped address is found in this segment
	int curr_segm = find_segment(info->si_addr);

	// If the address is not in a segment use default handler
	if (curr_segm < 0) {
		sigaction(SIGSEGV, &old_action, NULL);
		return;
	}

	so_seg_t segm = exec->segments[curr_segm];

	unsigned int page_in_segment = ((int)info->si_addr - (int)segm.vaddr) / PAGE_SIZE;
	unsigned int offset_inside_segment = page_in_segment * PAGE_SIZE;

	// Map new page in memory
	void *mapping;
	
	if (offset_inside_segment < segm.file_size) {
		// The current segment position has data
		mapping = mmap((char *)segm.vaddr + offset_inside_segment, PAGE_SIZE,
	 					segm.perm, MAP_FIXED | MAP_PRIVATE, exec_fd, segm.offset + offset_inside_segment);
		if (segm.file_size < segm.mem_size && segm.file_size - offset_inside_segment < PAGE_SIZE) {
			// There is not a full page of data so the rest is zeroed out
			memset(segm.vaddr + offset_inside_segment + segm.file_size % PAGE_SIZE, 0, PAGE_SIZE - segm.file_size % PAGE_SIZE);
		}
	} else {
		// The area is .bss, must be zeroed completely
		mapping = mmap((char *)segm.vaddr + offset_inside_segment, PAGE_SIZE, 
								segm.perm, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	}


	if (mapping == MAP_FAILED) {
		perror("Mapping failed");
		exit(-1);
	}
}

int so_init_loader(void)
{
	// handler
	memset(&action, 0, sizeof(action));
	action.sa_flags = SA_SIGINFO | SA_RESETHAND;
	action.sa_sigaction = sigsegv_handler;
	if (sigaction(SIGSEGV, &action, &old_action)) {
		perror("Failed sigaction\n");
		exit(-1);
	}

	return 0;
}

int so_execute(char *path, char *argv[])
{
	exec_fd = open(path, O_RDONLY);
	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	so_start_exec(exec, argv);
	return -1;
}
