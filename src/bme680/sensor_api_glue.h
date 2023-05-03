#include <bme68x.h>

extern "C" BME68X_INTF_RET_TYPE bme68x_i2c_read(uint8_t reg_addr, uint8_t * reg_data, uint32_t len, void* intf_ptr);
extern "C" BME68X_INTF_RET_TYPE bme68x_i2c_write(uint8_t reg_addr, const uint8_t* reg_data, uint32_t len, void* intf_ptr);
extern "C" void bme68x_delay_us(uint32_t period, void* intf_ptr);