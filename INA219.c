#include "main.h"
#include "INA219.h"
#include <FreeRTOS.h>
#include "task.h"
#include "cmsis_os2.h"
#include "i2c.h"

static osMutexId_t i2c1_mut_id = NULL;
static osMutexId_t i2c2_mut_id = NULL;
const osMutexAttr_t i2c1_mutex_attr = {"i2c1Mutex", osMutexPrioInherit, NULL, 1};
const osMutexAttr_t i2c2_mutex_attr = {"i2c2Mutex", osMutexPrioInherit, NULL, 1};

struct calibCoef high_calib = {INA219_HIGH_CALIBREG, INA219_HIGH_CURR_DIV, INA219_HIGH_CONFIG, INA219_POW_DIV};
struct calibCoef low_calib  = {INA219_LOW_CALIBREG, INA219_LOW_CURR_DIV, INA219_LOW_CONFIG, INA219_POW_DIV};


uint16_t Read16(INA219_t *ina219, uint8_t Register) {
    uint8_t Value[2];
    if (ina219->ina219_i2c == &hi2c1) {
        osMutexAcquire(i2c1_mut_id, osWaitForever);
        HAL_I2C_Mem_Read(ina219->ina219_i2c, (ina219->Address << 1), Register, 1, Value, 2, 1000);
        osMutexRelease(i2c1_mut_id);
    } else {
        osMutexAcquire(i2c2_mut_id, osWaitForever);
        HAL_I2C_Mem_Read(ina219->ina219_i2c, (ina219->Address << 1), Register, 1, Value, 2, 1000);
        osMutexRelease(i2c2_mut_id);
    }

    return ((Value[0] << 8) | Value[1]);
}
HAL_StatusTypeDef Write16(INA219_t *ina219, uint8_t Register, uint16_t Value) {
    uint8_t addr[2];
    addr[0] = (Value >> 8) & 0xff; // upper byte
    addr[1] = (Value >> 0) & 0xff; // lower byte

    if (ina219->ina219_i2c == &hi2c1) {
        osMutexAcquire(i2c1_mut_id, osWaitForever);
        return HAL_I2C_Mem_Write(ina219->ina219_i2c, (ina219->Address << 1), Register, 1, (uint8_t *) addr, 2,
                                 1000);
        osMutexRelease(i2c1_mut_id);
    } else {
        osMutexAcquire(i2c2_mut_id, osWaitForever);
        return HAL_I2C_Mem_Write(ina219->ina219_i2c, (ina219->Address << 1), Register, 1, (uint8_t *) addr, 2,
                                 1000);
        osMutexRelease(i2c2_mut_id);
    }
}

void initI2cMutex()
{
	i2c1_mut_id = osMutexNew(&i2c1_mutex_attr);
	i2c2_mut_id = osMutexNew(&i2c2_mutex_attr);
}
void INA219_Reset(INA219_t *ina219)
{
	Write16(ina219, INA219_REG_CONFIG, INA219_CONFIG_RESET);
}
void INA219_isOnBus(INA219_t *ina219)
{
	if (HAL_I2C_IsDeviceReady(ina219->ina219_i2c, (ina219->Address << 1), 3, 2) == HAL_OK)
	{
		ina219->states.is_on_bus = 1;
        return;
    }
        ina219->states.is_on_bus = 0;
}
float INA219_ReadCurrent(INA219_t *ina219) {
    float   result  = (int16_t)Read16(ina219, INA219_REG_CURRENT);
    return result /= ina219->calib->ina219_currentDivider_mA;
}
HAL_StatusTypeDef INA219_setCalibration(INA219_t *ina219, uint16_t CalibrationData)
{
	return Write16(ina219, INA219_REG_CALIBRATION, CalibrationData);
}
HAL_StatusTypeDef INA219_setConfig(INA219_t *ina219, uint16_t Config)
{
	return Write16(ina219, INA219_REG_CONFIG, Config);
}
uint8_t INA219_Init(INA219_t *ina219, I2C_HandleTypeDef *i2c, uint8_t Address) {
    ina219->ina219_i2c = i2c;
    ina219->Address    = Address;

    uint8_t ina219_isReady = HAL_I2C_IsDeviceReady(i2c, (Address << 1), 3, 2);

    if (ina219_isReady == HAL_OK) {
        INA219_Reset(ina219);
        int8_t initStatus = 0;
        initStatus |= INA219_setCalibration(ina219, ina219->calib->ina219_calibrationValue);
        initStatus |= INA219_setConfig(ina219, ina219->calib->ina219_config);
        if (initStatus == HAL_OK) {
            ina219->states.is_inited = 1;
            return 1;
        }
        return 0;
    } else {
        ina219->states.is_inited = 0;
        return 0;
    }
}

float getPower_mW(INA219_t *ina219, float corrCoef) {
    float valueDec = Read16(ina219, INA219_REG_POWER);
    valueDec *= ina219->calib->ina219_powerMultiplier_mW;
    return valueDec + corrCoef;
}

float INA219_ReadBusVoltage(INA219_t *ina219, float corrCoef) {
    uint16_t result = (Read16(ina219, INA219_REG_BUSVOLTAGE)>> 3) * 4;
    return (float)result + corrCoef;
}

float INA219_MeasComp(float current) {
    return (current * INA219_SHUNT_RESISTANCE)/1000;
}