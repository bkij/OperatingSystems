//
// Created by kveld on 23.05.17.
//

#ifndef ZESTAW10_COMMON_H
#define ZESTAW10_COMMON_H


#define MAX_PATH_LEN 108
#define MAX_CLIENTS 16
#define CLIENT_NAME_LEN 32

#define ERRNO_EXIT(S) do { perror(S); exit(EXIT_FAILURE); } while(1)

#endif //ZESTAW10_COMMON_H
