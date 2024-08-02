#ifndef DXL1X_H
#define DXL1X_H

#include "QtSerialPort/QSerialPort"

class dxl1x
{
public:
//*******************************************************
//* INSTRUCTIONS
//*******************************************************
#define DXL_ERR_MASK_VOLTAGE		(1)
#define DXL_ERR_MASK_ANGLE		(2)
#define DXL_ERR_MASK_OVERHEAT		(4)
#define DXL_ERR_MASK_RANGE		(8)
#define DXL_ERR_MASK_CHECKSUM		(16)
#define DXL_ERR_MASK_OVERLOAD		(32)
#define DXL_ERR_MASK_INSTRUCTION	(64)
//#####################################################
    typedef enum {
        UartErr_Ok = 0,
        UartErr_InvalidParam = -1
    }COM_ERR;
//#####################################################
    typedef enum {
        DXL_PING = 0x01, 		// obtain a status packet
        DXL_READ_DATA, 			// read Control Table values
        DXL_WRITE_DATA, 		// write Control Table values
        DXL_REG_WRITE, 			// write and wait for ACTION
        DXL_ACTION,			// triggers REG_WRITE
        DXL_RESET, 			// set factory defaults
        DXL_SYNC_WRITE = 0x83, 		// control multiple. actuators
        DXL_BULK_READ = 0x92		// read multiple actuators
    }DXL_COMM;
//#####################################################
    typedef enum {
        DXL_ERR_OK = 0,
        DXL_ERR_INPUT_VOLTAGE = 1 << 0,
        DXL_ERR_ANGLE_LIMIT = 1 << 1,
        DXL_ERR_OVERHEAT = 1 << 2,
        DXL_ERR_RANGE = 1 << 3,
        DXL_ERR_CHECKSUM = 1 << 4,
        DXL_ERR_OVERLOAD = 1 << 5,
        DXL_ERR_INSTRUCTION = 1 << 6,
    }DXL_ERR;
//#####################################################
    typedef enum {
        DXL_COMM_SUCCESS = 0,
        DXL_COMM_RXTIMEOUT = -1,
        DXL_COMM_RXCORRUPT = -2,
        DXL_COMM_ERROR = -3,
        DXL_COMM_OVERFLOW = -4,
        DXL_COMM_CHECKSUM = -5,
        DXL_COMM_BUFF_ALLOC = -6,
        DXL_OFFLINE = -7,
        DXL_COM_UNKNOWN = -8
    }DXL_COMM_ERR;
//#####################################################
    enum{
        DXL_MODEL_NUMBER_L, 		// 0x00
        DXL_MODEL_NUMBER_H, 		// 0x01
        DXL_VERSION, 			// 0x02
        DXL_ID, 			// 0x03
        DXL_BAUD_RATE, 			// 0x04
        DXL_RETURN_DELAY_TIME, 		// 0x05
        DXL_CW_ANGLE_LIMIT_L, 		// 0x06
        DXL_CW_ANGLE_LIMIT_H, 		// 0x07
        DXL_CCW_ANGLE_LIMIT_L, 		// 0x08
        DXL_CCW_ANGLE_LIMIT_H, 		// 0x09
        DXL_RESERVED1, 			// 0x0A
        DXL_LIMIT_TEMPERATURE, 		// 0x0B
        DXL_DOWN_LIMIT_VOLTAGE, 	// 0x0C
        DXL_UP_LIMIT_VOLTAGE, 		// 0x0D
        DXL_MDXL_TORQUE_L, 		// 0x0E
        DXL_MDXL_TORQUE_H, 		// 0x0F
        DXL_STATUS_RETURN_LEVEL, 	// 0x10
        DXL_ALARM_LED, 			// 0x11
        DXL_ALARM_SHUTDOWN, 		// 0x12
        DXL_RESERVED2, 			// 0x13
        DXL_DOWN_CALIBRATION_L, 	// 0x14
        DXL_DOWN_CALIBRATION_H, 	// 0x15
        DXL_UP_CALIBRATION_L, 		// 0x16
        DXL_UP_CALIBRATION_H, 		// 0x17
        DXL_TORQUE_ENABLE, 		// 0x18
        DXL_LED, 			// 0x19
        DXL_CW_COMPLIANCE_MARGIN, 	// 0x1A
        DXL_CCW_COMPLIANCE_MARGIN, 	// 0x1B
        DXL_CW_COMPLIANCE_SLOPE, 	// 0x1C
        DXL_CCW_COMPLIANCE_SLOPE, 	// 0x1D
        DXL_GOAL_POSITION_L, 		// 0x1E
        DXL_GOAL_POSITION_H, 		// 0x1F
        DXL_MOVING_SPEED_L, 		// 0x20
        DXL_MOVING_SPEED_H, 		// 0x21
        DXL_TORQUE_LIMIT_L, 		// 0x22
        DXL_TORQUE_LIMIT_H, 		// 0x23
        DXL_PRESENT_POSITION_L, 	// 0x24
        DXL_PRESENT_POSITION_H, 	// 0x25
        DXL_PRESENT_SPEED_L, 		// 0x26
        DXL_PRESENT_SPEED_H, 		// 0x27
        DXL_PRESENT_LOAD_L, 		// 0x28
        DXL_PRESENT_LOAD_H, 		// 0x29
        DXL_PRESENT_VOLTAGE, 		// 0x2A
        DXL_PRESENT_TEMPERATURE, 	// 0x2B
        DXL_REGISTERED_INSTRUCTION, 	// 0x2C
        DXL_RESERVE3, 			// 0x2D
        DXL_MOVING, 			// 0x2E
        DXL_LOCK, 			// 0x2F
        DXL_PUNCH_L, 			// 0x30
        DXL_PUNCH_H 			// 0x31
    };
//#####################################################
    typedef struct DXL_DATA_s{
        unsigned short model_number; 		// 0x00
        unsigned char  version; 		// 0x02
        unsigned char  id; 			// 0x03
        unsigned char  baud_rate; 		// 0x04
        unsigned char  return_delay_time; 	// 0x05
        unsigned short cw_angle_limit; 		// 0x06
        unsigned short ccw_angle_limit; 	// 0x08
        unsigned char  reserved1; 		// 0x0A
        unsigned char  limit_temperature; 	// 0x0B
        unsigned char  down_limit_voltage; 	// 0x0C
        unsigned char  up_limit_voltage; 	// 0x0D
        unsigned short mdxl_torque; 		// 0x0E
        unsigned char  status_return_level; 	// 0x10
        unsigned char  alarm_led; 		// 0x11
        unsigned char  alarm_shutdown; 		// 0x12
        unsigned char  reserved2; 		// 0x13
        unsigned short down_calibration; 	// 0x14
        unsigned short up_calibration; 		// 0x16
        unsigned char  torque_enable; 		// 0x18
        unsigned char  led; 			// 0x19
        unsigned char  cw_compliance_margin; 	// 0x1A
        unsigned char  ccw_compliance_margin; 	// 0x1B
        unsigned char  cw_compliance_slope; 	// 0x1C
        unsigned char  ccw_compliance_slope; 	// 0x1D
        unsigned short goal_position; 		// 0x1E
        unsigned short moving_speed; 		// 0x20
        unsigned short torque_limit; 		// 0x22
        unsigned short present_position; 	// 0x24
        unsigned short present_speed; 		// 0x26
        unsigned short present_load; 		// 0x28
        unsigned char  present_voltage; 	// 0x2A
        unsigned char  present_temperature; 	// 0x2B
        unsigned char  registered_instruction; 	// 0x2C
        unsigned char  reserved3; 		// 0x2D
        unsigned char  moving; 			// 0x2E
        unsigned char  lock; 			// 0x2F
        unsigned short punch; 			// 0x30
    }DXL_DATA_t;
//#####################################################
    typedef struct DXL_SYNK_IND_PACKET_s {
        unsigned char id;
        unsigned char data[23];
    }DXL_SYNK_IND_PACKET_t;
//#####################################################
    typedef struct DXL_BULK_READ_CMD_PACKET_s {
        unsigned char id;
        unsigned char len;
        unsigned char reg;
    }DXL_BULK_READ_CMD_PACKET_t;

    typedef struct DXL_BULK_READ_PACKET_s {
        unsigned char data[56];
        DXL_ERR dxl_err;
        DXL_COMM_ERR com_err;
    }DXL_BULK_READ_PACKET_t;
//#####################################################

    int err;
    unsigned char devNr;
    DXL_DATA_t *valueTable;
    DXL_ERR *dxl_err;
    DXL_COMM_ERR *err_ret;
    QSerialPort *uartInst;
    bool *online;
    bool *reversed;
    unsigned long timeout;

    dxl1x(QSerialPort *uartInst, unsigned char devNr, bool *reversed);
    ~dxl1x();
    DXL_COMM_ERR ping(unsigned char id, dxl1x::DXL_ERR *dxl_err);
    DXL_COMM_ERR read(unsigned char* buff, unsigned char id, unsigned char reg, unsigned char read_nr, dxl1x::DXL_ERR *dxl_err);
    DXL_COMM_ERR write(unsigned char comand_type, unsigned char id, unsigned char reg, unsigned char* send_data, unsigned char write_nr,
                       unsigned char *aditional_info, unsigned char *aditional_info_len, dxl1x::DXL_ERR *dxl_err);
    DXL_COMM_ERR synkWrite(unsigned char reg, DXL_SYNK_IND_PACKET_t *send_data_table, unsigned char write_nr, unsigned char nr_of_pockets);

    DXL_COMM_ERR dataWrite(unsigned char id, unsigned char reg, unsigned char* send_data, unsigned char write_nr, unsigned char *aditional_info, unsigned char *aditional_info_len, dxl1x::DXL_ERR *dxl_err);
    DXL_COMM_ERR regWrite(unsigned char id, unsigned char reg, unsigned char* send_data, unsigned char write_nr, unsigned char *aditional_info, unsigned char *aditional_info_len, dxl1x::DXL_ERR *dxl_err);

    DXL_COMM_ERR reset(unsigned char id, dxl1x::DXL_ERR *dxl_err);
    DXL_COMM_ERR changeId(unsigned char oldId, unsigned char newId, dxl1x::DXL_ERR *dxl_err);
    DXL_COMM_ERR setLimitTemperature(unsigned char id, unsigned char temperature, dxl1x::DXL_ERR *dxl_err);
    DXL_COMM_ERR setUpLimitVoltage(unsigned char id, unsigned char voltage, dxl1x::DXL_ERR *dxl_err);
    DXL_COMM_ERR setDnLimitVoltage(unsigned char id, unsigned char voltage, dxl1x::DXL_ERR *dxl_err);
    DXL_COMM_ERR setTorqueEnable(unsigned char id, bool enable, dxl1x::DXL_ERR *dxl_err);
    DXL_COMM_ERR setCwComplianceMargin(unsigned char id, unsigned char margin, dxl1x::DXL_ERR *dxl_err);
    DXL_COMM_ERR setCcwComplianceMargin(unsigned char id, unsigned char margin, dxl1x::DXL_ERR *dxl_err);
    DXL_COMM_ERR setCwComplianceSlope(unsigned char id, unsigned char slope, dxl1x::DXL_ERR *dxl_err);
    DXL_COMM_ERR setCcwComplianceSlope(unsigned char id, unsigned char slope, dxl1x::DXL_ERR *dxl_err);
    DXL_COMM_ERR setGoalPosition(unsigned char id, unsigned short position, dxl1x::DXL_ERR *dxl_err);
    DXL_COMM_ERR setMovingSpeed(unsigned char id, unsigned short speed, dxl1x::DXL_ERR *dxl_err);
    DXL_COMM_ERR setTorqueLimit(unsigned char id, unsigned short torque, dxl1x::DXL_ERR *dxl_err);
    DXL_COMM_ERR setPunch(unsigned char id, unsigned short punch, dxl1x::DXL_ERR *dxl_err);

    unsigned char getLimitTemperature(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err);
    unsigned char getUpLimitVoltage(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err);
    unsigned char getDnLimitVoltage(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err);
    bool getTorqueEnable(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err);
    unsigned char getCwComplianceMargin(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err);
    unsigned char getCcwComplianceMargin(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err);
    unsigned char getCwComplianceSlope(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err);
    unsigned char getCcwComplianceSlope(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err);
    unsigned short getGoalPosition(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err);
    unsigned short getMovingSpeed(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err);
    unsigned short getTorqueLimit(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err);
    unsigned short getPresentPosition(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err);
    unsigned short getPresentSpeed(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err);
    unsigned short getPresentLoad(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err);
    unsigned char getPresentVoltage(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err);
    unsigned char getPresentTemperature(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err);
    unsigned short getPunch(unsigned char id, DXL_COMM_ERR *status, dxl1x::DXL_ERR *dxl_err);
    /*
 * settings = structure that contain the uart and TXEN gpio structure settings.
 */
    DXL_COMM_ERR action();

    QString toHumanReadable(DXL_DATA_t *valueTable);
    QString showComError(DXL_COMM_ERR err);
    QString showDxlError(DXL_ERR err);
private:
    void insertChecksum(char *buff);
    bool verifyChecksum(char *buff);
};

#endif // DXL1X_H
