#include "dxl1x.h"

#include <QElapsedTimer>
#include <QDebug>

#define ID				(2)
#define LENGTH				(3)
#define INSTRUCTION			(4)
#define ERRBIT				(4)
#define PARAMETER			(5)
#define DEFAULT_BAUDNUMBER		(1)

dxl1x::dxl1x(QSerialPort *uartInst, unsigned char devNr, bool *reversed) {
    if(!uartInst) {
        err = UartErr_InvalidParam;
    }
    this->uartInst = uartInst;
    this->timeout = 5;
    this->devNr = devNr;
    this->valueTable = (DXL_DATA_t *)malloc(sizeof(DXL_DATA_t) * devNr);
    this->dxl_err = (dxl1x::DXL_ERR *)malloc(sizeof(dxl1x::DXL_ERR) * devNr);
    this->err_ret = (DXL_COMM_ERR *)malloc(sizeof(DXL_COMM_ERR) * devNr);
    this->online = (bool *)malloc(sizeof(bool) * devNr);
    this->reversed = (bool *)malloc(sizeof(bool) * devNr);
    memcpy(this->reversed, reversed, sizeof(bool) * devNr);
    //read_all_dxl_data();
    err = UartErr_Ok;
}

dxl1x::~dxl1x() {
    if(reversed)
        free(reversed);
    if(online)
        free(online);
    if(err_ret)
        free(err_ret);
    if(dxl_err)
        free(dxl_err);
    if(valueTable)
        free(valueTable);
    err = DXL_ERR_OK;
}

void dxl1x::insertChecksum(char *buff) {
    unsigned char chk = 0;
    unsigned char cnt = 0;
    for(; cnt < buff[LENGTH] + 1; cnt ++)
        chk += buff[cnt + 2];
    buff[buff[LENGTH] + 3] = ~chk;
}

bool dxl1x::verifyChecksum(char *buff) {
    unsigned char chk = 0;
    unsigned char cnt = 0;
    for(; cnt < buff[LENGTH] + 1; cnt ++)
        chk += buff[cnt + 2];
    chk = ~chk;
    unsigned char buff_chk = buff[buff[LENGTH] + 3];
    if(buff_chk == chk)
        return true;
    else
        return false;
}

dxl1x::DXL_COMM_ERR dxl1x::ping(unsigned char id, dxl1x::DXL_ERR *dxl_err) {
    *dxl_err = dxl1x::DXL_ERR_OK;
    char tx_buff[6];
    tx_buff[0] = 0xFF;
    tx_buff[1] = 0xFF;
    tx_buff[2] = id;
    tx_buff[3] = 2;
    tx_buff[4] = DXL_PING;
    tx_buff[5] = 0x0;
    insertChecksum(tx_buff);
    uartInst->clear();
    uartInst->write(tx_buff, tx_buff[3] + 4);
    uartInst->flush();
    QElapsedTimer timeoutTimer;
    timeoutTimer.start();
    unsigned int rx_cnt = 0;
    char rx_char;
    char rx_buff[150];
    memset(rx_buff, 0, 150);
    bool preamble_ok = false;
    while(1) {
        uartInst->waitForReadyRead(1);
        if(uartInst->bytesAvailable()) {
            timeoutTimer.start();
            uartInst->getChar(&rx_char);
            if(rx_cnt < 56)
                rx_buff[rx_cnt++] = rx_char;
            else {
                return DXL_COMM_OVERFLOW;
            }
            if(preamble_ok) {
                if(rx_cnt > 3 && rx_cnt != (unsigned char)-1) {
                    if(rx_buff[LENGTH] > 54) {
                        return DXL_COMM_RXCORRUPT;
                    }
                    if(rx_cnt - 4 == (unsigned int)rx_buff[LENGTH]) {
                        if(verifyChecksum(rx_buff)) {
                            *dxl_err = (dxl1x::DXL_ERR)rx_buff[ERRBIT];
                            return DXL_COMM_SUCCESS;
                        } else {
                            return DXL_COMM_CHECKSUM;
                        }
                    }
                }
            }
            preamble_ok = rx_buff[0] == (char)0xFF && rx_buff[1] == (char)0XFF;
        }
        if(timeoutTimer.hasExpired(timeout)) {
            return DXL_COMM_RXTIMEOUT;
        }
    }
}

/*
 * settings = structure that contain the uart and TXEN gpio structure settings.
 * buff     = data to be writen.
 * id       = DXL actuator ID.
 * reg      = pointed register from where begin to write the buff data.
 * write_nr = number of bytes to be read.
 */
dxl1x::DXL_COMM_ERR dxl1x::read(unsigned char* buff, unsigned char id, unsigned char reg, unsigned char read_nr, dxl1x::DXL_ERR *dxl_err) {
    *dxl_err = DXL_ERR_OK;
    if(read_nr > 50)
        return DXL_COMM_ERROR;
    if(id == 255)
        return DXL_COMM_ERROR;
    char tx_buff[8];
    char rx_buff[56];
    memset(rx_buff, 0, 56);
    tx_buff[0] = 0xFF;
    tx_buff[1] = 0xFF;
    tx_buff[2] = id;
    tx_buff[3] = 4;
    tx_buff[4] = DXL_READ_DATA;
    tx_buff[5] = reg;
    tx_buff[6] = read_nr;
    tx_buff[7] = 0x0;
    insertChecksum(tx_buff);
    uartInst->clear();
    uartInst->write(tx_buff, tx_buff[3] + 4);
    uartInst->flush();
    QElapsedTimer timeoutTimer;
    timeoutTimer.start();
    char rx_char;
    unsigned char rx_cnt = 0;
    bool preamble_ok = false;
    while(1) {
        uartInst->waitForReadyRead(1);
        if(uartInst->bytesAvailable()) {
            uartInst->getChar(&rx_char);
            timeoutTimer.start();
            if(rx_cnt < 56)
                rx_buff[rx_cnt++] = rx_char;
            else {
                memcpy(buff, rx_buff + 5, read_nr);
                return DXL_COMM_OVERFLOW;
            }
            if(preamble_ok) {
                if(rx_cnt > 3 && rx_cnt != (unsigned char)-1) {
                    if(rx_buff[LENGTH] > 54) {
                        memcpy(buff, rx_buff + 5, read_nr);
                        return DXL_COMM_RXCORRUPT;
                    }
                    if(rx_cnt - 4 == rx_buff[LENGTH]) {
                        if(verifyChecksum(rx_buff)) {
                            memcpy(buff, rx_buff + 5, read_nr);
                            *dxl_err = (dxl1x::DXL_ERR)rx_buff[ERRBIT];
                            return DXL_COMM_SUCCESS;
                        } else {
                            memcpy(buff, rx_buff + 5, read_nr);
                            return DXL_COMM_CHECKSUM;
                        }
                    }
                }
            }
            preamble_ok = rx_buff[0] == (char)0xFF && rx_buff[1] == (char)0XFF;
        }
        if(timeoutTimer.hasExpired(timeout)) {
            memcpy(buff, rx_buff + 5, read_nr);
            return DXL_COMM_RXTIMEOUT;
        }
    }
}

dxl1x::DXL_COMM_ERR dxl1x::write(unsigned char comand_type, unsigned char id, unsigned char reg, unsigned char* send_data, unsigned char write_nr,
                         unsigned char *aditional_info, unsigned char *aditional_info_len, dxl1x::DXL_ERR *dxl_err) {
    *dxl_err = DXL_ERR_OK;
    if(write_nr > 50 || write_nr + reg > 50 || id == 255)
        return dxl1x::DXL_COMM_ERROR;
    char tx_buff[256];// = (char *)malloc(write_nr + 7);
    /*if(!tx_buff) {
        return DXL_COMM_BUFF_ALLOC;
    }*/
    tx_buff[0] = 0xFF;
    tx_buff[1] = 0xFF;
    tx_buff[2] = id;
    unsigned char rx_cnt;
    QElapsedTimer timeoutTimer;
    timeoutTimer.start();
    uartInst->clear();
    if(write_nr) {
        tx_buff[3] = write_nr + 3;
        tx_buff[4] = comand_type;
        tx_buff[5] = reg;
        memcpy(tx_buff + 6, send_data, write_nr);
        insertChecksum(tx_buff);
        uartInst->write(tx_buff, tx_buff[3] + 7);
    } else {
        tx_buff[3] = 2;
        tx_buff[4] = comand_type;
        insertChecksum(tx_buff);
        uartInst->write(tx_buff, 6);
    }
    uartInst->flush();
    //free(tx_buff);
    rx_cnt = 0;
    char rx_char;
    char rx_buff[54];
    memset(rx_buff, 0, 54);
    bool preamble_ok = false;
    while(1) {
        uartInst->waitForReadyRead(1);
        if(uartInst->bytesAvailable()) {
            uartInst->getChar(&rx_char);
            timeoutTimer.start();
            if(rx_cnt < 56)
                rx_buff[rx_cnt++] = rx_char;
            else {
                return dxl1x::DXL_COMM_OVERFLOW;
            }
            if(preamble_ok) {
                if(rx_cnt > 3 && rx_cnt != (unsigned char)-1) {
                    if(rx_buff[LENGTH] > 54) {
                        return dxl1x::DXL_COMM_RXCORRUPT;
                    }
                    if(rx_cnt - 4 == rx_buff[LENGTH]) {
                        if(verifyChecksum(rx_buff)) {
                            if(rx_cnt - 6) {
                                memcpy(aditional_info, rx_buff + 5, rx_cnt - 6);
                            }
                            *dxl_err = (dxl1x::DXL_ERR)rx_buff[ERRBIT];
                            *aditional_info_len = rx_cnt - 6;
                            return dxl1x::DXL_COMM_SUCCESS;
                        } else {
                            return dxl1x::DXL_COMM_CHECKSUM;
                        }
                    }
                }
            }
            preamble_ok = rx_buff[0] == (char)0xFF && rx_buff[1] == (char)0XFF;
        }
        if(timeoutTimer.hasExpired(timeout)) {
            return dxl1x::DXL_COMM_RXTIMEOUT;
        }
    }
}


/*
 * settings = structure that contain the uart and TXEN gpio structure settings.
 * id       = DXL actuator ID.
 * reg      = pointed register from where begin to write the buff data.
 * send_data_table     = table of pockets data to be send.
 * write_nr = number of bytes on each packet.
 * nr_of_pockets = number of pockets to be send.
 */
dxl1x::DXL_COMM_ERR dxl1x::synkWrite(unsigned char reg, DXL_SYNK_IND_PACKET_t *send_data_table, unsigned char write_nr, unsigned char nr_of_pockets) {
    if(write_nr > 50 || write_nr == 0 || write_nr + reg > 50)
        return DXL_COMM_ERROR;
    char tx_buff[(54*256)+7];
    tx_buff[0] = 0xFF;
    tx_buff[1] = 0xFF;
    tx_buff[2] = 0xFE;
    tx_buff[3] = ((write_nr + 1) * nr_of_pockets) + 4;
    tx_buff[4] = DXL_SYNC_WRITE;
    tx_buff[5] = reg;
    tx_buff[6] = write_nr;
    unsigned char tx_buff_cnt = 7;
    unsigned char pocket_cnt = 0;
    for(; pocket_cnt < nr_of_pockets; pocket_cnt++) {
        tx_buff[tx_buff_cnt++] = send_data_table[pocket_cnt].id;
        unsigned char back[2] = {send_data_table[pocket_cnt].data[0], send_data_table[pocket_cnt].data[1]};
        if(reg == DXL_GOAL_POSITION_L && reversed[send_data_table[pocket_cnt].id-1] == true) {
            unsigned short tmp = 1023 - (send_data_table[pocket_cnt].data[0] + (send_data_table[pocket_cnt].data[1] << 8));
            send_data_table[pocket_cnt].data[0] = tmp;
            send_data_table[pocket_cnt].data[1] = tmp >> 8;
        }
        unsigned char data_cnt = 0;
        for(; data_cnt < write_nr; data_cnt++) {
            if(tx_buff_cnt > 254) {
                return DXL_COMM_OVERFLOW;
            }
            tx_buff[tx_buff_cnt++] = send_data_table[pocket_cnt].data[data_cnt];
        }
        send_data_table[pocket_cnt].data[0] = back[0];
        send_data_table[pocket_cnt].data[1] = back[1];
    }
    tx_buff_cnt++;
    insertChecksum(tx_buff);
    uartInst->clear();
    uartInst->write(tx_buff, tx_buff_cnt);
    uartInst->flush();
    return DXL_COMM_SUCCESS;
}

/*
 * settings = structure that contain the uart and TXEN gpio structure settings.
 * id       = DXL actuator ID.
 * reg      = pointed register from where begin to write the buff data.
 * send_data     = data to be send.
 * write_nr = number of bytes to be send.
 * aditional_info = buffer to receive additional info from DXL.
 * aditional_info_len = return number of bytes on aditional_info buffer.
 * dxl_err = the error byte received from DXL.
 */
dxl1x::DXL_COMM_ERR dxl1x::dataWrite(unsigned char id, unsigned char reg, unsigned char* send_data, unsigned char write_nr, unsigned char *aditional_info, unsigned char *aditional_info_len, dxl1x::DXL_ERR *dxl_err) {
    unsigned char back[2];
    if(reg == DXL_GOAL_POSITION_L && reversed[id-1] == true && write_nr >= 2) {
        back[0] = send_data[0];
        back[1] = send_data[1];
        unsigned short tmp = 1023 - (send_data[0] + (send_data[1] << 8));
        send_data[0] = tmp;
        send_data[1] = tmp >> 8;
    }
    DXL_COMM_ERR res =  write(DXL_WRITE_DATA, id, reg, send_data, write_nr, aditional_info, aditional_info_len, dxl_err);
    if(reg == DXL_GOAL_POSITION_L && reversed[id-1] == true && write_nr >= 2) {
        send_data[0] = back[0];
        send_data[1] = back[1];
    }
    return res;
}

/*
 * settings = structure that contain the uart and TXEN gpio structure settings.
 * id       = DXL actuator ID.
 * reg      = pointed register from where begin to write the buff data.
 * send_data     = data to be send.
 * write_nr = number of bytes to be send.
 * aditional_info = buffer to receive additional info from DXL.
 * aditional_info_len = return number of bytes on aditional_info buffer.
 * dxl_err = the error byte received from DXL.
 */
dxl1x::DXL_COMM_ERR dxl1x::regWrite(unsigned char id, unsigned char reg, unsigned char* send_data, unsigned char write_nr, unsigned char *aditional_info, unsigned char *aditional_info_len, dxl1x::DXL_ERR *dxl_err) {
    return write(DXL_REG_WRITE, id, reg, send_data, write_nr, aditional_info, aditional_info_len, dxl_err);
}

/*
 * settings = structure that contain the uart and TXEN gpio structure settings.
 * id       = DXL actuator ID.
 * dxl_err = the error byte received from DXL.
 */
dxl1x::DXL_COMM_ERR dxl1x::reset(unsigned char id, dxl1x::DXL_ERR *dxl_err) {
    unsigned char aditional_info[54];
    unsigned char aditional_info_len;
    return write(DXL_RESET, id, 0, NULL, 0, aditional_info, &aditional_info_len, dxl_err);
}

dxl1x::DXL_COMM_ERR dxl1x::changeId(unsigned char oldId, unsigned char newId, dxl1x::DXL_ERR *dxl_err) {
    unsigned char data_send[1];
    unsigned char aditional_info[54];
    unsigned char aditional_info_len = 0;
    data_send[0] = newId;
    return write(DXL_WRITE_DATA, oldId, DXL_ID,data_send, 1, aditional_info, &aditional_info_len, dxl_err);
}

dxl1x::DXL_COMM_ERR dxl1x::setLimitTemperature(unsigned char id, unsigned char temperature, dxl1x::DXL_ERR *dxl_err) {
    unsigned char data_send[1];
    unsigned char aditional_info[16];
    unsigned char aditional_info_len = 0;
    data_send[0] = (unsigned char)temperature;
    return dataWrite(id,DXL_LIMIT_TEMPERATURE, data_send, 1, aditional_info, &aditional_info_len, dxl_err);
}

dxl1x::DXL_COMM_ERR dxl1x::setUpLimitVoltage(unsigned char id, unsigned char voltage, dxl1x::DXL_ERR *dxl_err) {
    unsigned char data_send[1];
    unsigned char aditional_info[16];
    unsigned char aditional_info_len = 0;
    data_send[0] = (unsigned char)voltage;
    return dataWrite(id,DXL_UP_LIMIT_VOLTAGE, data_send, 1, aditional_info, &aditional_info_len, dxl_err);
}

dxl1x::DXL_COMM_ERR dxl1x::setDnLimitVoltage(unsigned char id, unsigned char voltage, dxl1x::DXL_ERR *dxl_err) {
    unsigned char data_send[2];
    unsigned char aditional_info[16];
    unsigned char aditional_info_len = 0;
    data_send[0] = (unsigned char)voltage;
    return dataWrite(id,DXL_DOWN_LIMIT_VOLTAGE, data_send, 1, aditional_info, &aditional_info_len, dxl_err);
}

dxl1x::DXL_COMM_ERR dxl1x::setTorqueEnable(unsigned char id, bool enable, dxl1x::DXL_ERR *dxl_err) {
    unsigned char data_send[1];
    unsigned char aditional_info[16];
    unsigned char aditional_info_len = 0;
    data_send[0] = (unsigned char)enable;
    return dataWrite(id,DXL_TORQUE_ENABLE, data_send, 1, aditional_info, &aditional_info_len, dxl_err);
}

dxl1x::DXL_COMM_ERR dxl1x::setCwComplianceMargin(unsigned char id, unsigned char margin, dxl1x::DXL_ERR *dxl_err) {
    unsigned char data_send[1];
    unsigned char aditional_info[16];
    unsigned char aditional_info_len = 0;
    data_send[0] = margin;
    return dataWrite(id, DXL_CW_COMPLIANCE_MARGIN, data_send, 1, aditional_info, &aditional_info_len, dxl_err);
}

dxl1x::DXL_COMM_ERR dxl1x::setCcwComplianceMargin(unsigned char id, unsigned char margin, dxl1x::DXL_ERR *dxl_err) {
    unsigned char data_send[1];
    unsigned char aditional_info[16];
    unsigned char aditional_info_len = 0;
    data_send[0] = margin;
    return dataWrite(id,DXL_CCW_COMPLIANCE_MARGIN, data_send, 1, aditional_info, &aditional_info_len, dxl_err);
}

dxl1x::DXL_COMM_ERR dxl1x::setCwComplianceSlope(unsigned char id, unsigned char slope, dxl1x::DXL_ERR *dxl_err) {
    unsigned char data_send[1];
    unsigned char aditional_info[16];
    unsigned char aditional_info_len = 0;
    data_send[0] = slope;
    return dataWrite(id,DXL_CW_COMPLIANCE_SLOPE, data_send, 1, aditional_info, &aditional_info_len, dxl_err);
}

dxl1x::DXL_COMM_ERR dxl1x::setCcwComplianceSlope(unsigned char id, unsigned char slope, dxl1x::DXL_ERR *dxl_err) {
    unsigned char data_send[1];
    unsigned char aditional_info[16];
    unsigned char aditional_info_len = 0;
    data_send[0] = slope;
    return dataWrite(id,DXL_CCW_COMPLIANCE_SLOPE, data_send, 1, aditional_info, &aditional_info_len, dxl_err);
}

dxl1x::DXL_COMM_ERR dxl1x::setGoalPosition(unsigned char id, unsigned short position, dxl1x::DXL_ERR *dxl_err) {
    unsigned char data_send[2];
    unsigned char aditional_info[16];
    unsigned char aditional_info_len = 0;
    data_send[0] = (unsigned char)position;
    data_send[1] = (unsigned char)(position >> 8);
    return dataWrite(id,DXL_GOAL_POSITION_L, data_send, 2, aditional_info, &aditional_info_len, dxl_err);
}

dxl1x::DXL_COMM_ERR dxl1x::setMovingSpeed(unsigned char id, unsigned short speed, dxl1x::DXL_ERR *dxl_err) {
    unsigned char data_send[2];
    unsigned char aditional_info[16];
    unsigned char aditional_info_len = 0;
    data_send[0] = (unsigned char)speed;
    data_send[1] = (unsigned char)(speed >> 8);
    return dataWrite(id,DXL_MOVING_SPEED_L, data_send, 2, aditional_info, &aditional_info_len, dxl_err);
}

dxl1x::DXL_COMM_ERR dxl1x::setTorqueLimit(unsigned char id, unsigned short torque, dxl1x::DXL_ERR *dxl_err) {
    unsigned char data_send[2];
    unsigned char aditional_info[16];
    unsigned char aditional_info_len = 0;
    data_send[0] = (unsigned char)torque;
    data_send[1] = (unsigned char)(torque >> 8);
    return dataWrite(id,DXL_TORQUE_LIMIT_L, data_send, 2, aditional_info, &aditional_info_len, dxl_err);
}

dxl1x::DXL_COMM_ERR dxl1x::setPunch(unsigned char id, unsigned short punch, dxl1x::DXL_ERR *dxl_err) {
    unsigned char data_send[2];
    unsigned char aditional_info[16];
    unsigned char aditional_info_len = 0;
    data_send[0] = (unsigned char)punch;
    data_send[1] = (unsigned char)(punch >> 8);
    return dataWrite(id,DXL_PUNCH_L, data_send, 2, aditional_info, &aditional_info_len, dxl_err);
}



unsigned char dxl1x::getLimitTemperature(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err) {
    unsigned char temperature = 0;
    *status = read(&temperature, id, DXL_LIMIT_TEMPERATURE, 1, dxl_err);
    return temperature;
}

unsigned char dxl1x::getUpLimitVoltage(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err) {
    unsigned char voltage = 0;
    *status = read(&voltage, id, DXL_UP_LIMIT_VOLTAGE, 1, dxl_err);
    return voltage;
}

unsigned char dxl1x::getDnLimitVoltage(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err) {
    unsigned char voltage = 0;
    *status = read(&voltage, id, DXL_DOWN_LIMIT_VOLTAGE, 1, dxl_err);
    return voltage;
}

bool dxl1x::getTorqueEnable(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err) {
    //unsigned char tmp = 0;
    unsigned char enable = 0;
    *status =  read(&enable, id, DXL_DOWN_LIMIT_VOLTAGE, 1, dxl_err);
    return enable == 1 ? true : false;
}

unsigned char dxl1x::getCwComplianceMargin(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err) {
    unsigned char voltage = 0;
    *status = read(&voltage, id, DXL_CW_COMPLIANCE_MARGIN, 1, dxl_err);
    return voltage;
}

unsigned char dxl1x::getCcwComplianceMargin(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err) {
    unsigned char voltage = 0;
    *status = read(&voltage, id, DXL_CCW_COMPLIANCE_MARGIN, 1, dxl_err);
    return voltage;
}

unsigned char dxl1x::getCwComplianceSlope(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err) {
    unsigned char voltage = 0;
    *status = read(&voltage, id, DXL_CW_COMPLIANCE_SLOPE, 1, dxl_err);
    return voltage;
}

unsigned char dxl1x::getCcwComplianceSlope(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err) {
    unsigned char voltage = 0;
    *status = read(&voltage, id, DXL_CCW_COMPLIANCE_SLOPE, 1, dxl_err);
    return voltage;
}

unsigned short dxl1x::getGoalPosition(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err) {
    unsigned char position[2] = {0, 0};
    *status = read(position, id, DXL_GOAL_POSITION_L, 2, dxl_err);
    return position[0] + (position[1] << 8);
}

unsigned short dxl1x::getMovingSpeed(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err) {
    unsigned char speed[2] = {0, 0};
    *status = read(speed, id, DXL_MOVING_SPEED_L, 2, dxl_err);
    return speed[0] + (speed[1] << 8);
}

unsigned short dxl1x::getTorqueLimit(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err) {
    unsigned char torque[2] = {0, 0};
    *status = read(torque, id, DXL_TORQUE_LIMIT_L, 2, dxl_err);
    return torque[0] + (torque[1] << 8);
}

unsigned short dxl1x::getPresentPosition(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err) {
    unsigned char position[2] = {0, 0};
    *status = read(position, id, DXL_PRESENT_POSITION_L, 2, dxl_err);
    return position[0] + (position[1] << 8);
}

unsigned short dxl1x::getPresentSpeed(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err) {
    unsigned char speed[2] = {0, 0};
    *status = read(speed, id, DXL_PRESENT_SPEED_L, 2, dxl_err);
    return speed[0] + (speed[1] << 8);
}

unsigned short dxl1x::getPresentLoad(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err) {
    unsigned char load[2] = {0, 0};
    *status = read(load, id, DXL_PRESENT_LOAD_L, 2, dxl_err);
    return load[0] + (load[1] << 8);
}

unsigned char dxl1x::getPresentVoltage(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err) {
    unsigned char voltage = 0;
    *status = read(&voltage, id, DXL_PRESENT_VOLTAGE, 1, dxl_err);
    return voltage;
}

unsigned char dxl1x::getPresentTemperature(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err) {
    unsigned char temperature = 0;
    *status = read(&temperature, id, DXL_PRESENT_TEMPERATURE, 1, dxl_err);
    return temperature;
}

unsigned short dxl1x::getPunch(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err) {
    unsigned char load[2] = {0, 0};
    *status = read(load, id, DXL_PUNCH_L, 2, dxl_err);
    return load[0] + (load[1] << 8);
}


/*
 * settings = structure that contain the uart and TXEN gpio structure settings.
 */
dxl1x::DXL_COMM_ERR dxl1x::action() {
    char tx_buff[6];
    tx_buff[0] = 0xFF;
    tx_buff[1] = 0xFF;
    tx_buff[2] = 0xFE;
    tx_buff[3] = 2;
    tx_buff[4] = DXL_ACTION;
    insertChecksum(tx_buff);
    uartInst->clear();
    uartInst->write(tx_buff, 6);
    uartInst->flush();
    return DXL_COMM_SUCCESS;
}

QString dxl1x::toHumanReadable(DXL_DATA_t *valueTable) {
    QString text;
    text.append("\nDXL model nr              = " + QString::number(valueTable->model_number));
    text.append("\nDXL version               = " + QString::number(valueTable->version));
    text.append("\nDXL ID                    = " + QString::number(valueTable->id));
    text.append("\nDXL baud rate             = " + QString::number(valueTable->baud_rate));
    text.append("\nDXL return delay time     = " + QString::number(valueTable->return_delay_time));
    text.append("\nDXL cw angle limit        = " + QString::number(valueTable->cw_angle_limit));
    text.append("\nDXL ccw angle limit       = " + QString::number(valueTable->ccw_angle_limit));
    text.append("\nDXL limit temperature     = " + QString::number(valueTable->limit_temperature));
    text.append("\nDXL down limit voltage    = " + QString::number(valueTable->down_limit_voltage));
    text.append("\nDXL up limit voltage      = " + QString::number(valueTable->up_limit_voltage));
    text.append("\nDXL mdxl torque limit     = " + QString::number(valueTable->mdxl_torque));
    text.append("\nDXL status return level   = " + QString::number(valueTable->status_return_level));
    text.append("\nDXL alarm led             = " + QString::number(valueTable->alarm_led));
    text.append("\nDXL alarm shutdown        = " + QString::number(valueTable->alarm_shutdown));
    text.append("\nDXL down calibration      = " + QString::number(valueTable->down_calibration));
    text.append("\nDXL up calibration        = " + QString::number(valueTable->up_calibration));
    text.append("\nDXL torque enable         = " + QString::number(valueTable->torque_enable));
    text.append("\nDXL led                   = " + QString::number(valueTable->led));
    text.append("\nDXL cw compliance margin  = " + QString::number(valueTable->cw_compliance_margin));
    text.append("\nDXL ccw compliance margin = " + QString::number(valueTable->ccw_compliance_margin));
    text.append("\nDXL cw compliance slope   = " + QString::number(valueTable->cw_compliance_slope));
    text.append("\nDXL ccw compliance slope  = " + QString::number(valueTable->ccw_compliance_slope));
    text.append("\nDXL goal position         = " + QString::number(valueTable->goal_position));

    text.append("\nDXL moving speed          = ");
    int tmp_value = valueTable->moving_speed;
    if(tmp_value > 1023)
        tmp_value = -(tmp_value & 0x3FF);
    text.append(QString::number(tmp_value));

    text.append("\nDXL torque limit          = " + QString::number(valueTable->torque_limit));
    text.append("\nDXL present position      = " + QString::number(valueTable->present_position));

    text.append("\nDXL present speed         = ");
    tmp_value = valueTable->present_speed;
    if(tmp_value > 1023)
        tmp_value = -(tmp_value & 0x3FF);
    text.append(QString::number(tmp_value));

    text.append("\nDXL present load          = ");
    tmp_value = valueTable->present_load;
    if(tmp_value > 1023)
        tmp_value = -(tmp_value & 0x3FF);
    text.append(QString::number(tmp_value));

    text.append("\nDXL present voltage       = " + QString::number(valueTable->present_voltage));
    text.append("\nDXL present temperature   = " + QString::number(valueTable->present_temperature));
    text.append("\nDXL registered instruction= " + QString::number(valueTable->registered_instruction));
    text.append("\nDXL moving                = " + QString::number(valueTable->moving));
    text.append("\nDXL lock                  = " + QString::number(valueTable->lock));
    text.append("\nDXL punch                 = " + QString::number(valueTable->punch));
    return text;
}

QString dxl1x::showComError(DXL_COMM_ERR err) {
    switch(err) {
    case DXL_COMM_SUCCESS:
        return "COM: OK";
    case DXL_COMM_RXTIMEOUT:
        return "COM: RX_TIMEOUT";
    case DXL_COMM_RXCORRUPT:
        return "COM: RX_CORRUPT";
    case DXL_COMM_ERROR:
        return "COM: COM_ERR";
    case DXL_COMM_OVERFLOW:
        return "COM: OVERFLOW";
    case DXL_COMM_CHECKSUM:
        return "COM: CHECKSUM";
    case DXL_COMM_BUFF_ALLOC:
        return "COM: BUFF_ALLOC";
    case DXL_OFFLINE:
        return "COM: OFFLINE";
    default:
        return "COM: UNKNOWN";
    }
}

QString dxl1x::showDxlError(DXL_ERR err) {
    QString res = err ? "DXL ERR: " : "";
    if(err) {
        if(err & DXL_ERR_INPUT_VOLTAGE) {
            res += "INPUT_VOLTAGE, ";
        }
        if(err & DXL_ERR_ANGLE_LIMIT) {
            res += "ANGLE_LIMIT, ";
        }
        if(err & DXL_ERR_OVERHEAT) {
            res += "OVERHEAT, ";
        }
        if(err & DXL_ERR_RANGE) {
            res += "RANGE, ";
        }
        if(err & DXL_ERR_CHECKSUM) {
            res += "CHECKSUM, ";
        }
        if(err & DXL_ERR_OVERLOAD) {
            res += "OVERLOAD, ";
        }
        if(err & DXL_ERR_INSTRUCTION) {
            res += "INSTRUCTION, ";
        }
    } else {
        res += "DXL ERR: NONE";
    }
    return res;
}

