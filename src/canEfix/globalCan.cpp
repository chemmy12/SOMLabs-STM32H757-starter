
#include "globalCan.h"

#ifdef GLOBAL_CAN

globalCan::globalCan()
{
}


uint16_t globalRecFilter, globalChannelFilter;

bool globalCan::setup(int baudRate,  int group, int channel) 
{
    _group = group;
    _channel = channel;
    globalRecFilter = 1 << _group;
    globalChannelFilter = 1 << _channel;
    // _regp = regp;
    _fromNibble = (_group << 4) + (_channel & 0x0F);
    
    // FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> *canInf = &Can1;
    Can1.begin();
    Can1.setBaudRate(baudRate);
    // Can1.setMaxMB(32);
    Can1.enableFIFO();
    Can1.enableFIFOInterrupt();
    Can1.onReceive(indirectHandleRecvMessage);    // Can1.setMBFilter(REJECT_ALL);

    Can2.begin();
    Can2.setBaudRate(baudRate);
    // Can2.setMaxMB(32);
    Can2.enableFIFO();
    Can2.enableFIFOInterrupt();
    Can2.onReceive(indirectHandleRecvMessage);    // Can1.setMBFilter(REJECT_ALL);


    _msgSend.flags.extended = 0;
    _msgSend.len = 8;
    _msgSend.id = 0;
    _msgSend.seq = 1;
    _msgSend.buf[1] = _fromNibble;

    _sendQHandler = xQueueCreate(100, sizeof(CAN_message_queue));

    xTaskCreate(startCanSendTask, "CanSendTask", 500, this, 5, NULL);

    return true;
}


bool globalCan::registerToRecvCan(CanCommand cmd, QueueHandle_t recvQ)
{
    if (cmd >= CC_MAX_COMMAND) {
        PrintD(DL_ERROR, "registerToRecvGlobalCan(): Invalid cmd ID: " + String(cmd));
        return false;
    }
    if (recvQHandlers[cmd] != NULL) {
        PrintD(DL_WARNING, "registerToRecvGlobalCan(): Already registered cmd ID: " + String(cmd));
        return false;
    }
    PrintD(DL_INFO, "registerToRecvGlobalCan(): Registering cmd ID: " + String(cmd));
    recvQHandlers[cmd] = recvQ;
    return true;
}

 void globalCan::handleSendCan()
 {
    CAN_message_queue msg;
    while(1) {
        vTaskDelay(pdUS_TO_TICKS(500));
        if (millis() - last_print > 1000) {
            if (count_received[0] + count_sent[0] > 1500) Serial.println("High Global Can Message Rate - Can1 Recived: " + String(count_received[0]) + ", Sent: " + String(count_sent[0]));
            if (count_received[1] + count_sent[1] > 1500) Serial.println("High Global Can Message Rate - Can2 Recived: " + String(count_received[1]) + ", Sent: " + String(count_sent[1]));
            // Serial.println("Total global messages recived on Can1: " + String(count_received[0]) + ", Sent: " + String(count_sent[0]));
            // Serial.println("Total global messages recived on Can2: " + String(count_received[1]) + ", Sent: " + String(count_sent[1]));
            canMsgRate[0] = count_sent[0] + count_received[0];
            canMsgRate[1] = count_sent[1] + count_received[1];
            count_received[0] = count_received[1] = 0;
            count_sent[0] = count_sent[1] = 0;
            last_print = millis();
        }


        if (xQueueReceive(_sendQHandler, &msg, pdMS_TO_TICKS(1000)) == pdTRUE) {

            // vTaskDelay(pdMS_TO_TICKS(1));
            // msg.data.b[1] = _fromNibble;                                                // Put the "from" just before sending
            CANMSG_Q_TO_CANMSG_T(msg, _msgSend);
            // PrintD(DL_VERBOSE, "handleGlobalSendCan(): Sending message ID: 0x" + String(_msgSend.id, HEX) +
            //         ", tableId=" + _msgSend.buf[2] + ", paramId=" + _msgSend.buf[3] + ", data=" + msg.data.f[1]);
            // Serial.print("handleGlobalSendCan(): Sending message ID: 0x" + String(_msgSend.id, BIN) +
            //         ", buf[0]=" + _msgSend.buf[0] + ", buf[1]=" + _msgSend.buf[1] +
            //         ", tableId=" + _msgSend.buf[2] + ", paramId=" + _msgSend.buf[3] + ", data=" + msg.data.f[1]);
            // float dataP = msg.data.f[1];
            // uint8_t* bytes = (uint8_t*)&dataP;
            // for (int i = 0; i < sizeof(float); i++) {
            //     Serial.print(" ");
            //     if (bytes[i] < 0x10) Serial.print("0"); // Add leading zero for values less than 0x10
            //     Serial.print(bytes[i], HEX);
            // }
            // Serial.println();

            // Serial.print("0x" + String(_msgSend.id, HEX) +
            //             " " + _msgSend.buf[2] + " " + _msgSend.buf[3] + " " + msg.data.f[1]);
            // float dataP = msg.data.f[1];
            // uint8_t* bytes = (uint8_t*)&dataP;
            // for (int i = 0; i < sizeof(float); i++) {
            //     Serial.print(" ");
            //     if (bytes[i] < 0x10) Serial.print("0"); // Add leading zero for values less than 0x10
            //     Serial.print(bytes[i], HEX);
            // }
            // Serial.println();
// #ifdef MASTER_PROC
//             if (_msgSend.id == 0x90 && _msgSend.buf[2] == 0x19 && _msgSend.buf[3] == 0x01) {
//                 Serial.print("handleGlobalSendCan(): Sending message ID: 0x" + String(_msgSend.id, HEX) + 
//                         ", tableId=" + _msgSend.buf[2] + ", paramId=" + _msgSend.buf[3] + ", data=" + msg.data.f[1] + ", ");
//                 float dataP = msg.data.f[1];
//                 uint8_t* bytes = (uint8_t*)&dataP;
//                 for (int i = 0; i < sizeof(float); i++) {
//                     Serial.print(" ");
//                     if (bytes[i] < 0x10) Serial.print("0"); // Add leading zero for values less than 0x10
//                     Serial.print(bytes[i], HEX);
//                 }
//                 Serial.println();
//             }
// #endif

            // Use the local copy for all write operations
            int r1 = Can1.write(_msgSend);
            if (r1 >= 0) count_sent[0]++;
            int r2 = Can2.write(_msgSend);
            if (r2 >= 0) count_sent[1]++;

            if (r1 <= 0 || r2 <= 0) { 
                PrintD(DL_VERBOSE, "handleGlobalSendCan(): Failed to send message ID: 0x" + String(_msgSend.id, HEX)  +
                            ", d0=" + _msgSend.buf[0] + ", d1=" + _msgSend.buf[1] + ", item=" /*+ _regp->getTableDotItem(_msgSend.buf[0], _msgSend.buf[1]) */); // TBD fix the print through other class or....
                bool c1_err = Can1.error(_can1_error, false);
                bool c2_err = Can2.error(_can2_error, false);
            }
        }
        else {
            PrintD(DL_WARNING, "globalCan() received no data to send for long time.");
        }
#ifdef PRINT_WM_Q_STATS
        UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
        if (uxHighWaterMark < 100) {
            GregpSetTaskStackBit(STACK_TASK_WARNING::STW_handleSendCan);
            Serial.println("handleSendCan: Stack high water mark is low: " + String(uxHighWaterMark));
        }
        else {
            GregpClearTaskStackBit(STACK_TASK_WARNING::STW_handleSendCan);
        }

        int16_t Q_space = uxQueueSpacesAvailable(_sendQHandler);
        if (Q_space < 10) {
            GregpSetQueueFullBit(QUEU_FULL_WARNING::QFW_handleSendCan);
            Serial.println("handleSendCan: Queue space is low: " + String(Q_space));
        }
        else {
            GregpClearQueueFullBit(QUEU_FULL_WARNING::QFW_handleSendCan);
        }
#endif
    }
 }

// CAN_message_t msg;

void globalCan::handleCANRecv(const CAN_message_t *msgp) {
    // Read all available messages from the FIFO and push them into the queue
    // Serial.print("handleGlobalCANRecv() message from nibble: ");
    // Serial.println(msgp->buf[1], HEX);
    // Serial.print("handleGlobalCANRecv() message ID: "); Serial.println(msgp->id, HEX);

    // if (!((1<<_channel) & msgp->buf[0]))      // Check if the message is for our channel
    //     return;

    // bool c1_err = Can1.error(_can1_error, true);
    // bool c2_err = Can2.error(_can2_error, true);

// #ifdef SENDER
//     static float dataP;
//     memcpy(&dataP, &msgp->buf[4], 4);
//     Serial.println(dataP);
    // Serial.print(" RX: ");
    // Serial.print(Can1.getRXQueueCount());
    // Serial.print(", ");
    // Serial.print(Can2.getRXQueueCount());
    // Serial.print(", TX: ");
    // Serial.print(Can1.getTXQueueCount());
    // Serial.print(", ");
    // Serial.println(Can2.getTXQueueCount());

    // bool c1_err = Can1.error(_can1_error, true);
    // bool c2_err = Can2.error(_can2_error, true);

    // if (msgp->id == 0x90 && msgp->buf[2] == 0x19 && msgp->buf[3] == 0x01) {
        // Serial.print("Recivied message ID: 0x" + String(msgp->id, HEX) + ", bus=" + msgp->bus +
        //         ", tableId=" + msgp->buf[2] + ", paramId=" + msgp->buf[3] + ", data=" + dataP + ", ");
        // uint8_t* bytes = (uint8_t*)&dataP;
        // for (int i = 0; i < sizeof(float); i++) {
        //     Serial.print(" ");
        //     if (bytes[i] < 0x10) Serial.print("0"); // Add leading zero for values less than 0x10
        //     Serial.print(bytes[i], HEX);
        // }
        // Serial.println();
    // }
// #endif
   
    CAN_message_queue msgq;
    if( msgp->bus == CAN_CHA_ID) count_received[0]++;
    else if (msgp->bus == CAN_CHB_ID) count_received[1]++;
    else {
        PrintD(DL_ERROR, "handleCANRecv(): Invalid bus ID: " + String(msgp->bus));
        return;
    }

    u_int cmd;
    if(msgp->id > ESC_NUMBER::ESC1 && msgp->id < (ESC_NUMBER::ESC4 + ESC_CAN_MSG_ID::ESC_CAN_MSG_MAX)) cmd = CC_ESC_Base;
    else cmd = CC_GET_COMMAND_FROM_MSG_ID(msgp->id);

    if (cmd < CC_MAX_COMMAND) {
        QueueHandle_t q = recvQHandlers[cmd];
        if (q) {
            BaseType_t *pxHigherPriorityTaskWoken;
            msgq.id = msgp->id; 
            msgq.bus = msgp->bus;
            memcpy(msgq.data.b, msgp->buf, 8);
            if (pdFAIL == xQueueSendFromISR((QueueHandle_t) q, &msgq, pxHigherPriorityTaskWoken))
                Serial.print("X" + String(cmd) + ",");
        }
    }
}

void indirectHandleRecvMessage(const CAN_message_t *msgp) {
    
#if defined(ESC_TASK) || defined(MASTER_LOGGER)
    gcan.handleCANRecv(msgp);
    // Serial.println("indirectHandleRecvMessage() called");
#else
     if (msgp->id & globalRecFilter)
         if (msgp->buf[0] & globalChannelFilter)
            gcan.handleCANRecv(msgp);
#endif
}

globalCan gcan;

#endif // GLOBAL_CAN