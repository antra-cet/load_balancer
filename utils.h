/* Copyright 2021 <Bivolaru Andra> */
#ifndef LOAD_BALANCER_SKEL_UTILS_H_
#define LOAD_BALANCER_SKEL_UTILS_H_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/* useful macro for handling error codes */
#define DIE(assertion, call_description)                                       \
    do {                                                                       \
        if (assertion) {                                                       \
            fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                 \
            perror(call_description);                                          \
            exit(errno);                                                       \
        }                                                                      \
    } while (0)

#endif  // LOAD_BALANCER_SKEL_UTILS_H_
