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

template <typename T>
class Metric
{
private:
    std::string _name;
    T _value;
    std::chrono::system_clock::time_point _time;
public:
    Metric() = defuult;

    Metric(const std::string& name, const T& value) :
        _name(name),
        _value(value),
        _time(std::chrono::system_clock::now()) { }
    
    Metric(const std::chrono::system_clock::time_point& time,
        const std::string& name,
        const T& value) :
        _name(name),
        _value(value),
        _time(time) { }

    const std::string& GetName() const { return _name; }
    const T& GetValue() const { return _value; }
    const std::chrono::system_clock::time_point& GetTime() const { return _time; }

    bool operator<(const Metric& other) const
    {
        if (_time != other._time)
            return _time < other._time;
        if (_value != other._value)
            return _value < other._value;
        if (_name != other._name)
            return _name < other._name;        
        return _time < other._time;
    }
};

template <typename T>
class MetricsCollector
{
private:
    std::mutex _mutex;        
    std::set<Metric> _metrics;
public:
    bool AddMetrics(const std::vector<Metric>& metrics)
    {
        if (metrics.empty())
            return false;

        std::lock_guard<std::mutex> lock(_mutex);
        _metrics.insert(metrics.begin(), metrics.end());
        return true;
    }

    bool AddMetric(const std::string& name, const T& metric)
    {        
        Metric metric(name, metric);  
        std::vector <Metric> metrics;
        metric.push_back(metric);
        AddMetrics(metrics);
    }

    bool SaveToFile(const std::string& filename)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_metrics.empty()) 
            return false;

        std::ofstream file(filename, std::ios::app);
        if (!file.is_open())
            return false;

        std::sort(_metrics.begin(), _metrics.end(),
            [](const Metric<T>& a, const Metric<T>& b) 
            {
                return a.GetTime() < b.GetTime();
            });

        auto it = _metrics.begin();
        while (it != _metrics.end())
        {
            auto current_time = it->GetTime();
            auto time_t = std::chrono::system_clock::to_time_t(current_time);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                current_time.time_since_epoch()) % 1000;

            std::tm tm;
            localtime_s(&tm, &time_t);

            file << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S.")
                << std::setfill('0') << std::setw(3) << ms.count() << ": ";

            if (it == _metrics.end()) break;  

            auto group_start_time = it->GetTime();  
            bool first_in_group = true;

            while (it != _metrics.end() && it->SameTime(group_start_time)) {
                if (!first_in_group) file << ", ";
                file << it->GetName() << ": " << it->GetValue();
                first_in_group = false;
                ++it;
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