//
// Created by kveld on 18.05.17.
//

#ifndef ZESTAW9_WRITER_H
#define ZESTAW9_WRITER_H

#include <stdbool.h>

void *write_to_array(void *arg);
void modify(int *shared_array, int cnt, bool verbose);

#endif //ZESTAW9_WRITER_H
