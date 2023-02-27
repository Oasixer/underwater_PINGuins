#ifndef FYDP_ROV_MAIN_H
#define FYDP_ROV_MAIN_H

#include "tcp_client.h"
void rov_main_setup(TcpClient& client);
void rov_main_loop(TcpClient& client);

#endif
