#include "stationary_main.h"

void stationary_main_loop(){
    // TODO: check health

    if (is_currently_receiving){
        receive_mode_hb();
    } else {  // sending

    }

    // TODO: log if needed
}
