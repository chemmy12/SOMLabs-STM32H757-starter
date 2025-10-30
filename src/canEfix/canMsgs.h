// msaage types for the CAN bus

#ifndef CANMSGS_H
#define CANMSGS_H


// CAN message types

enum CanCommand {
    CC_HeartBeat,       // TBD
    CC_SetParam,        // Set a parameter
    CC_LogData,         // Log data
    CC_ESC_Base,
    CC_MAX_COMMAND
};

union ubf_data {     // Union Byte Float - This data struct is used for Params and log data
    float f[2];
    uint8_t b[8];
};
struct CAN_message_queue {
    ubf_data    data;
    uint32_t    id;
    uint8_t     bus;
};

#include "msgDebug/msgdebug.h"

// CAN id is built as follows:
// 11 bits. the least significant 2 bits are the address of the wing: 0 for front right, 1 for back right, 2 for back left, 3 for front left
// The next 1 bit (3rd lsb) is the direction: 1 -> to master, 0 -> from master
// The next 1 is 1 in case we do a boadcast to all wings
// The next 3 bits (4-6) are spare
// The next 4 bits (7-10) are the command

#define ADDRESS_WIDTH 4
#define CMD_WIDTH 4
#define CMD_SHIFT 7

#define     CC_GET_COMMAND_FROM_MSG_ID(id)                              ((id >> CMD_SHIFT) & ((1<<CMD_WIDTH)-1))
#define     CC_SET_COMMAND_IN_MSG_ID(id, cmd)                           (id = ((id & ((1<<CMD_SHIFT)-1))) | (cmd << CMD_SHIFT)) 
// #define     CC_CALC_ADDRESS_TO_MSG_ID(id, toMaster, wID)                ((id & ~((1<<ADDRESS_WIDTH)-1-8)) | (toMaster?4:0) | (wID&0x03))
#define     CANMSG_Q_TO_CANMSG_T(msgq, msgt)                            {msgt.id = msgq.id; memcpy(msgt.buf, msgq.data.b, 8);}
#define     CANMSG_T_TO_CANMSG_Q(msgt, msgq)                            {msgq.idd = msgt.id; memcpy(msgq.data.b, msgt.buf, 8);}
// #define     CC_SET_BROADCAST_IN_MSG_ID(id)                              (id |= 8)
// #define     CC_UNSET_BROADCAST_IN_MSG_ID(id)                            (id &= ~8)
// #define     CC_GET_WING_FROM_ID(id)                                     (id & 0x03)



// Another proposal for CAN addressing in CAN Id for "globalCan" bus.
// 11 bits for id. The message command is in the most 4 significant bits.
// The other 7 bits are used for addressing the destinations (Groups).
// See ProcGroup for groups. 1<<PG_MAIN.....
// least significant bit, bit 0: is the master.
// bit 1-4: the wing address. 1 for front right, 2 for back right, 3 for back left, 4 for front left
// bit 5,6: For other processors that are on the global CAN bus (BMS, ESC, .....)
// for broadcasting to multiple: multiple bits can be set.
// First data byte is the destination channel. Each group may have different number for channels....Multiple bits for multiple destinations.
// Second byte is sender ID. most significant nibble for Group number (not flag but value). The least significant nibble is the channel of the sender. (both are numbers and not flags)
// the sender group is 0-6 respective (CanAddress) to the address bits in the ID.
// the third data byte table number
// the fourth data byte is the parameter index in the table
// the last 4 bytes are the parameter value


// #define     CC_CALC_ID(cmd, to)                 ((cmd << CMD_SHIFT) | (1<<to))
inline void CC_CALC_ID(uint32_t &id, uint32_t cmd, uint32_t group) {
    id = (cmd << CMD_SHIFT) | (1<<group);
}
inline void CC_CALC_ID_Flags(uint32_t &id, uint32_t cmd, uint32_t toFlags) {
    id = (cmd << CMD_SHIFT) | (toFlags);
}
inline void CC_CALC_GROUP(uint32_t &id, uint32_t cmd, uint32_t to, uint32_t broadCast) {
    id = (cmd << CMD_SHIFT) | (1<<to) | (broadCast<<2);
}
inline uint16_t CC_GET_ID_BY_CMD_GRP(uint32_t cmd, uint32_t groupVal) {
    return (cmd << CMD_SHIFT) | groupVal;
}
inline void CC_SET_CMD_GRP_CHAN(CAN_message_queue *msgp, uint32_t cmd, uint32_t group, uint32_t chan = 0xff) {
    msgp->id = (cmd << CMD_SHIFT) | (group & 0x7f);
    msgp->data.b[0] = chan;
}
inline void CC_SET_CMD_GRP_CHAN_FROM(CAN_message_queue &msg, uint32_t cmd, uint32_t group, uint32_t chan, uint8_t _group, uint8_t _channel) {
    msg.id = (cmd << CMD_SHIFT) | (group & 0x7f);
    msg.data.b[0] = chan;
    msg.data.b[1] = _group << 4 | _channel;
}


#define PG_WING(w)      (1<<w)
#define PG_WINGS_ALL    (1<<PG_WING1 | 1<<PG_WING2 | 1<<PG_WING3 | 1<<PG_WING4)
#define PG_ALL          (1<<PG_MAIN | 1<<PG_WING1 | 1<<PG_WING2 | 1<<PG_WING3 | 1<<PG_WING4 | 1<<PG_FWD | 1<<PG_ENGINE)
#define PG_MASTER       (1<<PG_MAIN)
#define PG_ALL_CHANS    (0xff)
#define PG_GRP_TO_VAL(group)    (1<<group)

#define CC_SENT_FROM_GROUP(msg)  ((msg.data.b[1] >> 4) & 0x0f)
#define CC_SENT_FROM_CHAN(msg)   (msg.data.b[1] & 0x0f)
#define CC_SENT_TO_CHAN(msg)     (msg.data.b[0]  & 0xff)



#endif // CANMSGS_H