#pragma once

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <set>
#include <algorithm>
#include <memory>

class BaseMetric
{
protected:
    std::string _name;
    std::chrono::system_clock::time_point _time;
public:
    BaseMetric(const std::chrono::system_clock::time_point& time,
        const std::string& name) : _name(name), _time(time) { }

    const std::string& GetName() const { return _name; }
    const std::chrono::system_clock::time_point& GetTime() const { return _time; }

    virtual bool operator<(const BaseMetric& other) const = 0;
    virtual void WriteToStream(std::ostream& os) const = 0;   
    virtual ~BaseMetric() = default;
};

template <typename T>
class Metric : public BaseMetric
{
private:    
    T _value;    
public:
    const T& GetValue() const { return _value; }

    Metric(const std::string& name, const T& value) :
        BaseMetric(std::chrono::system_clock::now(), name),        
        _value(value) { }
    
    Metric(const std::chrono::system_clock::time_point& time,
        const std::string& name,
        const T& value) :
        BaseMetric(time, name),
        _value(value) { }     

    bool operator<(const BaseMetric& other) const override
    {
        const auto* derived = dynamic_cast<const Metric<T>*>(&other);
        if (!derived) return false;

        if (_time != derived->_time)
            return _time < derived->_time;
        if (_value != derived->_value)
            return _value < derived->_value;
        return _name < derived->_name;
    }
    void WriteToStream(std::ostream& os) const override 
    {
        os << _name << " " << _value;
    }
};

struct MetricComparator {
    bool operator()(const std::unique_ptr<BaseMetric>& a,
        const std::unique_ptr<BaseMetric>& b) const {
        return *a < *b;
    }
};

class MetricsCollector
{
private:
    std::mutex _mutex;    
    std::set<std::unique_ptr<BaseMetric>, MetricComparator> _metrics;  
   
public:
    MetricsCollector() = default;

    MetricsCollector(const MetricsCollector&) = delete;

    template <typename T>
    bool AddMetric(const std::string& name, const T& value) {
        return AddMetric(Metric<T>(name, value));
    }
    
    bool AddMetric(const char* name, const char* value) {
        return AddMetric(std::string(name), std::string(value));
    }

    bool AddMetric(const std::string& name, const char* value) {
        return AddMetric(name, std::string(value));
    }

    bool AddMetric(const char* name, const std::string& value) {
        return AddMetric(std::string(name), value);
    }
    
    template <typename T>
    bool AddMetric(const Metric<T>& metric) {
        std::lock_guard<std::mutex> lock(_mutex);
        _metrics.insert(std::make_unique<Metric<T>>(metric));
        return true;
    }

    template <typename T>
    bool AddMetrics(const std::vector<Metric<T>>& metrics)
    {
        if (metrics.empty()) return false;

        std::lock_guard<std::mutex> lock(_mutex);
        for (const auto& metric : metrics) {
            _metrics.insert(std::make_unique<Metric<T>>(metric));
        }
        return true;
    }

    bool SaveToFile(const std::string& filename)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_metrics.empty()) 
            return false;

        std::ofstream file(filename, std::ios::app);
        if (!file.is_open())
            return false;       

        for (auto it = _metrics.begin(); it != _metrics.end(); )
        {
            auto current_time = (*it)->GetTime();
            auto time_t = std::chrono::system_clock::to_time_t(current_time);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                current_time.time_since_epoch()) % 1000;

            std::tm tm;
            localtime_s(&tm, &time_t);

            file << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S.")
                << std::setfill('0') << std::setw(3) << ms.count() << ": ";

            auto group_start_time = (*it)->GetTime();
            bool first_in_group = true;

            for (; it != _metrics.end() &&
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    (*it)->GetTime().time_since_epoch()) ==
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    group_start_time.time_since_epoch()); ++it) 
            {
                if (!first_in_group) file << ", ";
                (*it)->WriteToStream(file);
                first_in_group = false;
            }
            file << "]\n";
        }        
        file.close();
        _metrics.clear();
        return true;
    }

    bool Close()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _metrics.clear();
        return true;
    }
};