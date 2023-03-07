#include "calibration.h"
#include "positions.h"

Calibration::Calibration(TcpClient* _client, Listener* _listener){
    is_calibrated = false;
    has_depths = false;
    calibrating_node_135 = 1;
    received_replies_012 = 0;
    sending = false;

    client = _client;
    listener = _listener;
}