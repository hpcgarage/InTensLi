/******************************************************************************\
 *                                                                            *
 * Copyright (c) 2012 Marat Dukhan                                            *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 * claim that you wrote the original software. If you use this software       *
 * in a product, an acknowledgment in the product documentation would be      *
 * appreciated but is not required.                                           *
 *                                                                            *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 * misrepresented as being the original software.                             *
 *                                                                            *
 * 3. This notice may not be removed or altered from any source               *
 * distribution.                                                              *
 *                                                                            *
\******************************************************************************/

#include <timer.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

timer::timer() {
	int cgt_result = clock_gettime(CLOCK_MONOTONIC, &this->creation_time);
	if (cgt_result == -1) {
		fprintf(stderr, "Failed to query the time from system: error code %d\n", errno);
		exit(EXIT_FAILURE);
	}
}

timer::~timer() {
}

double timer::get_ms() {
	struct timespec current_time;
	int cgt_result = clock_gettime(CLOCK_MONOTONIC, &current_time);
	if (cgt_result == -1) {
		fprintf(stderr, "Failed to query the time from system: error code %d\n", errno);
		exit(EXIT_FAILURE);
	}
	return double((current_time.tv_sec * 1000000000ll + current_time.tv_nsec) - (creation_time.tv_sec * 1000000000ll + creation_time.tv_nsec)) / 1.0e+6; // ns to ms
}
