#pragma once

#include <list>
#include <memory>

class sensor_data {
    float data;

public:
    sensor_data(float data = 0.0) :
        data(data)
    {
    }

    virtual ~sensor_data()
    {
    }

    virtual void set_value(float value)
    {
        data = value;
    }

    virtual float get_value() const
    {
        return data;
    }

    virtual std::string get_name() const = 0;
    virtual std::string get_unit() const = 0;
    virtual uint8_t get_id() const = 0;
};

using sensor_data_ptr = std::shared_ptr<sensor_data>;
using sensor_data_list = std::list<sensor_data_ptr>;