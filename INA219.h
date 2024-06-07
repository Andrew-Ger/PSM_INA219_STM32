#ifndef INC_INA219_H_
#define INC_INA219_H_

 //    I2C ADDRESS TABLE
 // | A1   | A0   | Address |
 // | GND  | GND  | 0x40    |
 // | GND  | V_S+ | 0x41    |
 // | GND  | SDA  | 0x42    |
 // | GND  | SCL  | 0x43    |
 // | V_S+ | GND  | 0x44    |
 // | V_S+ | V_S+ | 0x45    |
 // | V_S+ | SDA  | 0x46    |
 // | V_S+ | SCL  | 0x47    |
 // | SDA  | GND  | 0x48    |
 // | SDA  | V_S+ | 0x49    |
 // | SDA  | SDA  | 0x4A    |
 // | SDA  | SCL  | 0x4B    |
 // | SCL  | GND  | 0x4C    |
 // | SCL  | V_S+ | 0x4D    |
 // | SCL  | SDA  | 0x4E    |
 // | SCL  | SCL  | 0x4F    |


#define INA219_REG_CONFIG       (0x00)
#define INA219_REG_SHUNTVOLTAGE (0x01)
#define INA219_REG_BUSVOLTAGE   (0x02)
#define INA219_REG_POWER        (0x03)
#define INA219_REG_CURRENT      (0x04)
#define INA219_REG_CALIBRATION  (0x05)

#define INA219_CONFIG_RESET (0x8000)

#define INA219_CONFIG_BVOLTAGERANGE_16V (0x0000) // 0-16V Range
#define INA219_CONFIG_BVOLTAGERANGE_32V (0x2000) // 0-32V Range

#define INA219_CONFIG_GAIN_1_40MV              (0x0000) // Gain 1, 40mV Range
#define INA219_CONFIG_GAIN_2_80MV              (0x0800) // Gain 2, 80mV Range
#define INA219_CONFIG_GAIN_4_160MV             (0x1000) // Gain 4, 160mV Range
#define INA219_CONFIG_GAIN_8_320MV             (0x1800) // Gain 8, 320mV Range

#define INA219_CONFIG_SADCRES_9BIT_1S_84US     (0x0000) // 1 x 9-bit shunt sample
#define INA219_CONFIG_SADCRES_10BIT_1S_148US   (0x0008) // 1 x 10-bit shunt sample
#define INA219_CONFIG_SADCRES_11BIT_1S_276US   (0x0010) // 1 x 11-bit shunt sample
#define INA219_CONFIG_SADCRES_12BIT_1S_532US   (0x0018) // 1 x 12-bit shunt sample
#define INA219_CONFIG_SADCRES_12BIT_2S_1060US  (0x0048) // 2 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_4S_2130US  (0x0050) // 4 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_8S_4260US  (0x0058) // 8 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_16S_8510US (0x0060) // 16 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_32S_17MS   (0x0068) // 32 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_64S_34MS   (0x0070) // 64 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_128S_69MS  (0x0078) // 128 x 12-bit shunt samples averaged together

#define INA219_CONFIG_MODE_MASK                (0x07)
#define INA219_CONFIG_MODE_POWERDOWN           (0x00) /**< power down */
#define INA219_CONFIG_MODE_SVOLT_TRIGGERED     (0x01) /**< shunt voltage triggered */
#define INA219_CONFIG_MODE_ADCOFF              (0x04) /**< ADC off */
#define INA219_CONFIG_MODE_SVOLT_CONTINUOUS    (0x05) /**< shunt voltage continuous */


// calibrate and check koef correctly
#define INA219_HIGH_CALIBREG                   (10240)
#define INA219_LOW_CALIBREG                    (27768)
#define INA219_HIGH_CURR_DIV                   (25)
#define INA219_LOW_CURR_DIV                    (61)

#define INA219_HIGH_CONFIG                                                                                  \
     INA219_CONFIG_BVOLTAGERANGE_32V | INA219_CONFIG_GAIN_8_320MV | INA219_CONFIG_SADCRES_12BIT_1S_532US |   \
         INA219_CONFIG_MODE_SVOLT_CONTINUOUS

#define INA219_LOW_CONFIG                                                                                   \
     INA219_CONFIG_BVOLTAGERANGE_16V | INA219_CONFIG_GAIN_1_40MV | INA219_CONFIG_SADCRES_12BIT_128S_69MS |   \
         INA219_CONFIG_MODE_SVOLT_CONTINUOUS

 struct calibCoef {
     uint16_t ina219_calibrationValue;
     int16_t  ina219_currentDivider_mA;
     int16_t  ina219_config;
 };

 extern struct calibCoef high_calib; // 32v 1A
 extern struct calibCoef low_calib;  // 16v 400ma
 typedef struct {
     I2C_HandleTypeDef *ina219_i2c;
     uint8_t            Address;
     struct calibCoef  *calib;
     struct {
         uint8_t is_on_bus : 1;
         uint8_t is_inited : 1;
         uint8_t target    : 4;
         uint8_t reserved  : 2;

     } states;

 } INA219_t;

 void initI2cMutex();
 void INA219_Reset(INA219_t *ina219);
 void INA219_isOnBus(INA219_t *ina219);
 float INA219_ReadCurrent(INA219_t *ina219);
 uint8_t INA219_Init(INA219_t *ina219, I2C_HandleTypeDef *i2c, uint8_t Address);
 uint16_t Read16(INA219_t *ina219, uint8_t Register);
 HAL_StatusTypeDef Write16(INA219_t *ina219, uint8_t Register, uint16_t Value);
 HAL_StatusTypeDef INA219_setCalibration(INA219_t *ina219, uint16_t CalibrationData);
 HAL_StatusTypeDef INA219_setConfig(INA219_t *ina219, uint16_t Config);
#endif /* INC_INA219_H_ */
