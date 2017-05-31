//
// Created by kveld on 23.05.17.
//

#ifndef ZESTAW10_DG_SERVER_H
#define ZESTAW10_DG_SERVER_H

void parse_args(int argc, char **argv, char *port_num, char *unit_sock_path);
void dispatch_threads(char *port_num, char *unix_sock_path);

#endif //ZESTAW10_DG_SERVER_H
