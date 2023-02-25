bytes = np.zeros(801)
from enum import Enum
class Modes(Enum):
    IM_SKIPPER_NOW # -> i will 
    IM_RICO_STATIONARY, # i will State(listen)
                           #  then State(reply), and tell you when
    ,


if bytes[0] == NOT_ADC_DATA:
    # this message doesn't have any data, just communication stuff for us

    if bytes[1] == 1: # 
        state = bytes[2]
    elif bytes[1] == 2:
        mode = Modes(bytes[2])
    elif bytes[1] == 3:
        idx_detected_freq = bytes[2]
        freq_magnitude = readFloat(bytes[3:7])
    elif bytes[1] == 4:
        time_between_Send_and_Receive = readUint16(bytes[3:7])
    elif bytes[1] == 5:
        character_array = readUnitYouFindLineBreak

elif bytes[0] == ADC_DATA:
    save_adc_data(bytes[1:])





main loop should check:
