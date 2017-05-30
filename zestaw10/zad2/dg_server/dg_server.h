//
// Created by kveld on 23.05.17.
//

#ifndef ZESTAW10_DG_SERVER_H
#define ZESTAW10_DG_SERVER_H

struct client_info {

};

void parse_args(int argc, char **argv, char *port_num, char *unit_sock_path);
void dipatch_threads(char *port_num, char *unix_sock_path);

#endif //ZESTAW10_DG_SERVER_H
