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

void initI2cMutex()
{
	i2c1_mut_id = osMutexNew(&i2c1_mutex_attr);
	i2c2_mut_id = osMutexNew(&i2c2_mutex_attr);
}

uint16_t Read16(INA219_t *ina219, uint8_t Register)
{

	uint8_t Value[2];

if(ina219->ina219_i2c == &hi2c1){
	osMutexAcquire(i2c1_mut_id, osWaitForever);
	HAL_I2C_Mem_Read(ina219->ina219_i2c, (ina219->Address << 1), Register, 1, Value, 2, 1000);
	osMutexRelease(i2c1_mut_id);
	}
	else{
	osMutexAcquire(i2c2_mut_id, osWaitForever);
	HAL_I2C_Mem_Read(ina219->ina219_i2c, (ina219->Address << 1), Register, 1, Value, 2, 1000);
	osMutexRelease(i2c2_mut_id);
	}

	return ((Value[0] << 8) | Value[1]);
}

HAL_StatusTypeDef Write16(INA219_t *ina219, uint8_t Register, uint16_t Value)
{
	uint8_t addr[2];
	addr[0] = (Value >> 8) & 0xff;  // upper byte
	addr[1] = (Value >> 0) & 0xff; // lower byte

	if(ina219->ina219_i2c == &hi2c1){
	osMutexAcquire(i2c1_mut_id, osWaitForever);
	return HAL_I2C_Mem_Write(ina219->ina219_i2c, (ina219->Address << 1), Register, 1, (uint8_t *)addr, 2, 1000);
	osMutexRelease(i2c1_mut_id);
	}
	else{
	osMutexAcquire(i2c2_mut_id, osWaitForever);
	return HAL_I2C_Mem_Write(ina219->ina219_i2c, (ina219->Address << 1), Register, 1, (uint8_t *)addr, 2, 1000);
	osMutexRelease(i2c2_mut_id);
	}

}

uint16_t INA219_ReadBusVoltage(INA219_t *ina219)
{
	uint16_t result = Read16(ina219, INA219_REG_BUSVOLTAGE);

	return ((result >> 3  ) * 4);

}

int16_t INA219_ReadCurrent_raw(INA219_t *ina219)
{
	int16_t result = Read16(ina219, INA219_REG_CURRENT);

	return result;
}

float INA219_ReadCurrent(INA219_t *ina219)
{
	float result = INA219_ReadCurrent_raw(ina219);
	result /= ina219_currentDivider_mA;
	return result;
}
float INA219_ReadPower(INA219_t *ina219)
{
	int16_t result = Read16(ina219, INA219_REG_POWER);

	float valueDec = result * ina219_powerMultiplier_mW;

	return valueDec;
}

uint16_t INA219_ReadShuntVolage(INA219_t *ina219)
{
	uint16_t result = Read16(ina219, INA219_REG_SHUNTVOLTAGE);

	return (result * 0.01 );
}

void INA219_Reset(INA219_t *ina219)
{
	Write16(ina219, INA219_REG_CONFIG, INA219_CONFIG_RESET);
}

HAL_StatusTypeDef INA219_setCalibration(INA219_t *ina219, uint16_t CalibrationData)
{
	return Write16(ina219, INA219_REG_CALIBRATION, CalibrationData);
}

uint16_t INA219_getConfig(INA219_t *ina219)
{
	uint16_t result = Read16(ina219, INA219_REG_CONFIG);
	return result;
}

HAL_StatusTypeDef INA219_setConfig(INA219_t *ina219, uint16_t Config)
{
	return Write16(ina219, INA219_REG_CONFIG, Config);
}

void INA219_setCalibration_32V_2A(INA219_t *ina219)
{
	uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V |
	             INA219_CONFIG_GAIN_8_320MV | INA219_CONFIG_BADCRES_12BIT |
	             INA219_CONFIG_SADCRES_12BIT_1S_532US |
	             INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;

	ina219_calibrationValue = 4096;
	ina219_currentDivider_mA = 10; // Current LSB = 100uA per bit (1000/100 = 10)
	ina219_powerMultiplier_mW = 2; // Power LSB = 1mW per bit (2/1)

	INA219_setCalibration(ina219, ina219_calibrationValue);
	INA219_setConfig(ina219, config);
}

HAL_StatusTypeDef INA219_setCalibration_32V_1A(INA219_t *ina219)
{
	uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V |
	                    INA219_CONFIG_GAIN_8_320MV | INA219_CONFIG_BADCRES_12BIT |
	                    INA219_CONFIG_SADCRES_12BIT_1S_532US |
	                    INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;

	ina219_calibrationValue = 10240;
	ina219_currentDivider_mA = 25;    // Current LSB = 40uA per bit (1000/40 = 25)
	ina219_powerMultiplier_mW = 0.8f; // Power LSB = 800uW per bit

	// INA219_setCalibration(ina219, ina219_calibrationValue);
	// INA219_setConfig(ina219, config);
	return (INA219_setCalibration(ina219, ina219_calibrationValue) && INA219_setConfig(ina219, config));
}

HAL_StatusTypeDef INA219_setCalibration_16V_400mA(INA219_t *ina219)
{
	uint16_t config = INA219_CONFIG_BVOLTAGERANGE_16V |
					  INA219_CONFIG_GAIN_1_40MV |
					  INA219_CONFIG_SADCRES_12BIT_128S_69MS|
					  INA219_CONFIG_SADCRES_12BIT_128S_69MS |
					  INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;

	ina219_calibrationValue = 27768;//27306
	ina219_currentDivider_mA = 64;    // Current LSB = 50uA per bit (1000/50 = 20)//64 my setting
	ina219_powerMultiplier_mW = 0.4f; // Power LSB = 1mW per bit

	return (INA219_setCalibration(ina219, ina219_calibrationValue) && INA219_setConfig(ina219, config));
}


uint8_t INA219_Init(INA219_t *ina219, I2C_HandleTypeDef *i2c, uint8_t Address)
{
	ina219->ina219_i2c = i2c;
	ina219->Address = Address;

	ina219_currentDivider_mA = 0;
	ina219_powerMultiplier_mW = 0;

	uint8_t ina219_isReady = HAL_I2C_IsDeviceReady(i2c, (Address << 1), 3, 2);

	if(ina219_isReady == HAL_OK)
	{
		INA219_Reset(ina219);
		// if (INA219_setCalibration_16V_400mA(ina219) == HAL_OK)
		if (INA219_setCalibration_32V_1A(ina219) == HAL_OK)
		{
			ina219->states.is_on_bus = 1;
			ina219->states.old_stat = 1;
            return 1;
        }
		return 0;
	}

	else
	{
		ina219->states.is_on_bus = 0;
		ina219->states.old_stat = 0;
		return 0;
	}
}

void INA219_isOnBus(INA219_t *ina219)
{

	uint8_t ina219_isReady = HAL_I2C_IsDeviceReady(ina219->ina219_i2c, (ina219->Address << 1), 3, 2);

	if (ina219_isReady == HAL_OK)
	{
		ina219->states.is_on_bus = 1;
	}

	else
	{
		ina219->states.is_on_bus = 0;
	}
}

