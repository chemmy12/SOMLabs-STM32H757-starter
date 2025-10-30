#pragma once

#include "config.h"

#ifdef GLOBAL_CAN

#define HEX 16
#include <FlexCAN_T4.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "can/canMsgs.h"
#include <array>
#include "msgDebug/msgdebug.h"
#include "enums.h"
extern void GregpSetTaskStackBit(STACK_TASK_WARNING b);
extern void GregpClearTaskStackBit(STACK_TASK_WARNING b);
extern void GregpSetQueueFullBit(QUEU_FULL_WARNING b);
extern void GregpClearQueueFullBit(QUEU_FULL_WARNING b);

#define FLEXCAN0_MB_NUM 0
#define FLEXCAN_RX_EXT_ID 1

// #define CAN_STATISTICS

class globalCan {
    public:
        globalCan();
        bool setup(int baudRate, int group, int channel);
        bool registerToRecvCan(CanCommand cmd, QueueHandle_t recvQ);
        // bool registerToSendCan(QueueHandle_t sendQ);
        QueueHandle_t   getSendQ()      { return _sendQHandler; }

        // TBD: Make two task. one for send and one for receive and two register functions
        
        void handleCANRecv(const CAN_message_t *msg);
        // bool sendCanMessage(CAN_message_t msg);

        static void startCanSendTask(void* pvParameters)  {
            globalCan* taskHandler = static_cast<globalCan*>(pvParameters);
            taskHandler->handleSendCan();
        }
        void handleSendCan();
        uint32_t getCanMsgRate(int id) { return canMsgRate[id]; }
        

    private:
        FlexCAN_T4<CAN_CHA, RX_SIZE_256, TX_SIZE_16> Can1;
        FlexCAN_T4<CAN_CHB, RX_SIZE_256, TX_SIZE_16> Can2;

        std::array<QueueHandle_t, CC_MAX_COMMAND> recvQHandlers;
        // std::vector<QueueHandle_t> sendQHandlers;
        QueueHandle_t   _sendQHandler;

        int _group;
        int _channel;
        uint8_t _fromNibble;
        // Reg *_regp;
        CAN_message_t       _msgSend;
        CAN_message_queue   _msgRecvq;
        
        // Error tracking
        CAN_error_t _can1_error;
        CAN_error_t _can2_error;

        uint32_t last_print;
        uint16_t count_received[2];
        uint16_t count_sent[2];
        uint32_t canMsgRate[2];

};

void indirectHandleRecvMessage(const CAN_message_t *msg);
extern globalCan gcan;


#endif // GLOBAL_CAN