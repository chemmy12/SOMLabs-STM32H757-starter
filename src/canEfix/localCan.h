#pragma once

#include "config.h"

#ifdef LOCAL_CAN

#include "Arduino.h"
#include "FlexCAN_T4.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "can/canMsgs.h"

extern void GregpSetTaskStackBit(STACK_TASK_WARNING b);
extern void GregpClearTaskStackBit(STACK_TASK_WARNING b);
extern void GregpSetQueueFullBit(QUEU_FULL_WARNING b);
extern void GregpClearQueueFullBit(QUEU_FULL_WARNING b);

#include "msgDebug/msgdebug.h"
// #include "reg/reg.h"

class localCan {
    public:
        localCan();
        bool setup(int baudRate, int wingId, int procId);
        bool registerToRecvLocalCan(CanCommand cmd, QueueHandle_t recvQ);           // TBD: Handle commands - need maybe to change the array into a lookup table
        // bool registerToSendCan(QueueHandle_t sendQ);
        QueueHandle_t   getSendQ()      { return _sendQHandler; }

        // TBD: Make two task. one for send and one for receive and two register functions
        
        void handleCAN3Recv(CAN_message_t *msg);
        // bool sendCanMessage(CAN_message_t msg);

        static void startCan3SendTask(void* pvParameters)  {
            localCan* taskHandler = static_cast<localCan*>(pvParameters);
            taskHandler->handleSendCan3();
        }
        

    private:
        FlexCAN_T4<CAN_L, RX_SIZE_256, TX_SIZE_16> Can3;
        void handleSendCan3();

        std::array<QueueHandle_t, CC_MAX_COMMAND> _recvQHandlers;
        QueueHandle_t   _sendQHandler;

        int _wingId;
        int _procId;
        uint8_t _fromNibble;

        uint32_t last_print;
        uint16_t count_received;
        uint16_t count_sent;    

        CAN_message_t       _msgSend;
        CAN_message_queue   _msgRecvq;
};

void indirectHandleRecvMessageCan3(CAN_message_t *msg);
extern localCan lcan;


#endif // LOCAL_CAN
