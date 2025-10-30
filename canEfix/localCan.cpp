#include "localCan.h"

#ifdef LOCAL_CAN

uint16_t localRecFilter, localChannelFilter;

localCan::localCan()
{
}

bool localCan::setup(int baudRate, int wingId, int procId)
{
    _wingId = wingId;
    _procId = procId;

    localRecFilter = 1 << _wingId;
    localChannelFilter = 1 << _procId;

    _fromNibble = (_wingId << 4) + (_procId & 0x0F);

    Can3.begin();
    Can3.setBaudRate(baudRate);
    Can3.enableFIFO();
    Can3.enableFIFOInterrupt();
    Can3.onReceive(indirectHandleRecvMessageCan3);    // Can1.setMBFilter(REJECT_ALL);

    last_print = millis();
    count_received = count_sent = 0;

    _sendQHandler = xQueueCreate(16, sizeof(CAN_message_queue));
    xTaskCreate(startCan3SendTask, "Can3SendTask", 512, this, 2, NULL);

    return true;
}

// CAN_message_t msg;

void localCan::handleSendCan3() {
    Serial.println("handleLocalSend() called");
    CAN_message_queue msg;
    UBaseType_t uxHighWaterMark;
 
    while (true) {
        if (millis() - last_print > 1000) {
            // Serial.println("Total local messages recived: " + String(count_received) + ", Sent: " + String(count_sent));
            if (count_received + count_sent > 1300) {
                Serial.println("GlobalCan: Too many messages received and sent: " + String(count_received) + ", " + String(count_sent));
            }
            count_received = 0;
            count_sent = 0;
            last_print = millis();
        }

        if (xQueueReceive(_sendQHandler, &msg, pdMS_TO_TICKS(1000)) == pdTRUE) {
            count_sent++;
            msg.data.b[1] = _fromNibble;                                                // Put the "from" just before sending
            CANMSG_Q_TO_CANMSG_T(msg, _msgSend);
            PrintD(DL_VERBOSE, "handleLocalSendCan(): Sending message ID: 0x" + String(_msgSend.id, HEX) +
                    ", tableId=" + _msgSend.buf[2] + ", paramId=" + _msgSend.buf[3]);
            // vTaskDelay(pdMS_TO_TICKS(1));

            if (Can3.write(_msgSend)) {       // TBD Should be changed to && in full setup
                PrintD(DL_VERBOSE, "handleLocalSendCan(): Sent message ID: 0x" + String(_msgSend.id, HEX));
            }
            else {
                PrintD(DL_VERBOSE, "handleLocalSendCan(): Failed to send message ID: 0x" + String(_msgSend.id, HEX)  +
                            ", d0=" + _msgSend.buf[0] + ", d1=" + _msgSend.buf[1] + ", item=" /*+ _regp->getTableDotItem(_msgSend.buf[0], _msgSend.buf[1]) */); // TBD fix the print through other class or....
            }
        }
        else {
            PrintD(DL_WARNING, "localCan() received no data to send for long time.");
        }
        uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
        if (uxHighWaterMark < 100) {
            GregpSetTaskStackBit(STACK_TASK_WARNING::STW_handleSendCan3);
            Serial.println("handleSendCan3: Stack high water mark is low: " + String(uxHighWaterMark));
        }
        else {
            GregpClearTaskStackBit(STACK_TASK_WARNING::STW_handleSendCan3);
        }
    }
}

void localCan::handleCAN3Recv(CAN_message_t *msgp) {
    CAN_message_queue msgq;
    count_received++;    
    // Serial.print("handleLocalCANRecv() message from nibble: ");
    // Serial.println(msgp->buf[1], HEX);

    // Serial.print(" "); Serial.print(msgp->id, HEX);
    // Serial.print(" "); Serial.print(msgp->buf[2], HEX);
    // Serial.print(" "); Serial.print(msgp->buf[3], HEX);
    // Serial.print(" "); Serial.print(msgp->buf[4], HEX);
    // Serial.print(" "); Serial.print(msgp->buf[5], HEX);
    // Serial.print(" "); Serial.print(msgp->buf[6], HEX);
    // Serial.print(" "); Serial.print(msgp->buf[7], HEX);
    // Serial.println();

    // Serial.println("handleCAN3Recv() message ID: 0x" + String(msgp->id, HEX));
    u_int cmd = CC_GET_COMMAND_FROM_MSG_ID(msgp->id);
    if (cmd < CC_MAX_COMMAND) {
        QueueHandle_t q = _recvQHandlers[cmd];
        if (q) {
            BaseType_t *pxHigherPriorityTaskWoken;
            msgq.id = msgp->id; 
            memcpy(msgq.data.b, msgp->buf, 8);
            if (pdFAIL == xQueueSendFromISR((QueueHandle_t) q, &msgq, pxHigherPriorityTaskWoken))
                Serial.print("X" + String(cmd) + ",");
        }
    }
}

void indirectHandleRecvMessageCan3(CAN_message_t *msgp) {
    if (msgp->id & localRecFilter)
        if (msgp->buf[0] & localChannelFilter)
            lcan.handleCAN3Recv(msgp);
}

bool localCan::registerToRecvLocalCan(CanCommand cmd, QueueHandle_t recvQ) {
    if (cmd >= CC_MAX_COMMAND) {
        PrintD(DL_ERROR, "registerToRecvLocalCan(): Invalid cmd ID: " + String(cmd));
        return false;
    }
    if (_recvQHandlers[cmd] != NULL) {
        PrintD(DL_WARNING, "registerToRecvLocalCan(): Already registered cmd ID: " + String(cmd));
        return false;
    }
    PrintD(DL_INFO, "registerToRecvLocalCan(): Registering cmd ID: " + String(cmd));
    _recvQHandlers[cmd] = recvQ;
    return true;
}

localCan lcan;

#endif // LOCAL_CAN