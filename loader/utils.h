/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef UTILS_H_
#define UTILS_H_

#include <errno.h>

#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);				\
		}							\
	} while (0)
#endif
