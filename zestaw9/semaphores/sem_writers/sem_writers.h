//
// Created by kveld on 19.05.17.
//

#ifndef ZESTAW9_SEM_WRITERS_H
#define ZESTAW9_SEM_WRITERS_H

#include <stdbool.h>

void *write_to_array(void *arg);
void modify(int *shared_array, int cnt, bool verbose);

#endif //ZESTAW9_SEM_WRITERS_H
