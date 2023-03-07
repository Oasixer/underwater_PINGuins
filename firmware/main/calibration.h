#ifndef FYDP_CALIBRATION_H
#define FYDP_CALIBRATION_H

#include "utils.h" // for coord_3d_t
#include "listener.h"
#include "tcp_client.h"

class Calibration {
    public:
        bool is_calibrated;
        coord_3d_t coords[N_ALL_NODES];
        bool has_depths;
        float depths[N_ALL_NODES];
        TcpClient* client;
        Listener* listener;
        Calibration(TcpClient* _client, Listener* _listener);

        void tick();
    private:
        uint16_t calibrating_node_135; // 1, then 3, then 5
        uint16_t received_replies_012; // 0, then 1, then 2 = finished
        bool sending;
};

#endif