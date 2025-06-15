#pragma once

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <unordered_set>

template <typename T>
class Metric
{
private:
    std::string _name;
    T _value;
    std::chrono::system_clock::time_point _time;
public:
    Metric() {}
    Metric(const std::string& name, 
        const T& value)
    {
        _name = name;
        _value = value;
        _time = std::chrono::system_clock::now();
    }
    Metric(const std::chrono::system_clock::time_point& time,
        const std::string& name,
        const T& value)
    {
        _name = name;
        _value = value;
        _time = time;
    }
};

template <typename T>
class MetricsCollector
{
private:
    std::mutex _mutex;    
    std::map<std::chrono::system_clock::time_point, std::vector<std::pair<std::string, T>>> _metrics;
public:
    bool AddMetrics(const std::vector<std::pair<std::string, T>>& metrics)
    {
        if (metrics.empty())
            return false;

        auto timestamp = std::chrono::system_clock::now();
        std::lock_guard<std::mutex> lock(_mutex);
        _metrics[timestamp] = metrics;
        return true;
    }

    bool AddMetric(std::pair<std::string, T> metric)
    {
        std::vector<std::pair<std::string, T>> vectorWIthOneMetric;
        vectorWIthOneMetric.push_back(metric);
        AddMetrics(vectorWIthOneMetric);
    }

    bool SaveToFile(const std::string& filename)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        std::ofstream file(filename, std::ios::app);
        if (!file.is_open())
            return false;

        for (const auto& entry : _metrics)
        {
            const auto& timestamp = entry.first;
            const auto& metricsVector = entry.second;

            auto time_t = std::chrono::system_clock::to_time_t(timestamp);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                timestamp.time_since_epoch()) % 1000;

            std::tm tm;
            localtime_s(&tm, &time_t);

            file << std::put_time(&tm, "%Y-%m-%d %H:%M:%S.")
                << std::setfill('0') << std::setw(3) << ms.count() << " ";

            for (const auto& metric : metricsVector)
            {
                file << metric.first << " " << metric.second << " ";
            }
            file << '\n';
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