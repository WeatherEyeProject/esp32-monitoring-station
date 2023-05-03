#include "sensor_api_glue.h"

#include <bme68x.h>
#include <i2c/sensors_i2c_bus.hpp>
#include <cstring>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

BME68X_INTF_RET_TYPE bme68x_i2c_read(uint8_t reg_addr, uint8_t* reg_data, uint32_t len, void* intf_ptr)
{
	const uint8_t dev_addr = (uint32_t)intf_ptr;
	auto i2c_bus = sensors_i2c_bus::get_instance();

	if (len == 1) {
		auto ret = i2c_bus->i2c_read(dev_addr, reg_addr);
		if (!ret) {
			return -1;
		}

		*reg_data = *ret;
	}
	else {
		auto ret = i2c_bus->i2c_read_bytes(dev_addr, reg_addr, len);
		if (!ret) {
			return -1;
		}

		memcpy(reg_data, ret->data(), len);
	}

	return BME68X_INTF_RET_SUCCESS;
}

BME68X_INTF_RET_TYPE bme68x_i2c_write(uint8_t reg_addr, const uint8_t* reg_data, uint32_t len, void* intf_ptr)
{
	const uint8_t dev_addr = (uint32_t)intf_ptr;
	auto i2c_bus = sensors_i2c_bus::get_instance();

	bool ret;
	if (len == 1) {
		ret = i2c_bus->i2c_write(dev_addr, reg_addr, *reg_data);
	}
	else {
		ret = i2c_bus->i2c_write_bytes(dev_addr, reg_addr, reg_data, len);
	}

	return ret ? BME68X_INTF_RET_SUCCESS : -1;
}

// All waits used in driver are at least 1ms so use FreeRTOS method
// To prevent busy waits usage
void bme68x_delay_us(uint32_t period, void* intf_ptr)
{
	vTaskDelay(pdMS_TO_TICKS(period / 1000));
}