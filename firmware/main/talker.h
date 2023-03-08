#ifndef FYDP_TALKER_H
#define FYDP_TALKER_H

#include "configurations.h"

class Talker {
    public:
        Talker(config_t* _config);
        uint64_t begin_yell_5ms(uint16_t frequency_id);
        bool yell();
    private:
        uint16_t freq;
        uint64_t ts_start;
        config_t* config;
};

#endif