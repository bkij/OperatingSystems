#ifndef UTIL_H
#define UTIL_H

void err_exit(const char *why);
long long timestamp();

#define ARR_LEN(x) sizeof(x)/sizeof(x[0])

#endif