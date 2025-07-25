#pragma once

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string_view>

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

    void WriteToStream(std::ostream& os) const override
    {
        os << std::quoted(_name) << " " << _value;
    }
};

class MetricsCollector
{
private:
    std::mutex _mutex;
    std::map<std::chrono::system_clock::time_point,
        std::vector<std::unique_ptr<BaseMetric>>> _metrics;

    template <typename T>
    void AddMetricImpl(const Metric<T>& metric)
    {
        _metrics[metric.GetTime()].push_back(std::make_unique<Metric<T>>(metric));
    }

    std::string FormatTime(const std::chrono::system_clock::time_point& time) const
    {
        auto t = std::chrono::system_clock::to_time_t(time);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            time.time_since_epoch()) % 1000;

        std::tm tm;
        localtime_s(&tm, &t);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S.")
            << std::setfill('0') << std::setw(3) << ms.count();
        return oss.str();
    }

public:
    MetricsCollector() = default;
    MetricsCollector(const MetricsCollector&) = delete;

    template <typename T>
    void AddMetric(std::string_view name, const T& value)\
    {
        AddMetric(Metric<T>(std::string(name), value));
    }

    template <typename T>
    void AddMetric(const Metric<T>& metric)
    {
        std::lock_guard lock(_mutex);
        AddMetricImpl(metric);        
    }

    template <typename T>
    void AddMetrics(const std::vector<Metric<T>>& metrics)  //��� Vector (����������� �������� ����������)
    {
        std::lock_guard lock(_mutex);
        for (const auto& m : metrics) {
            AddMetricImpl(m);
        }
    }    

    template <typename T>
    void AddMetrics(std::initializer_list<Metric<T>> metrics) //��� Initializer List
    {
        AddMetrics(std::vector<Metric<T>>(metrics));  
    }
    
    template <typename Iterator>
    void AddMetrics(Iterator begin, Iterator end) //������������� ����� ��� stl �����������
    {
        std::lock_guard lock(_mutex);
        for (; begin != end; ++begin) {
            AddMetricImpl(*begin);
        }
    }

    bool SaveToFile(const std::string& filename)
    {
        std::lock_guard lock(_mutex);
        if (_metrics.empty())
            return false;

        std::ofstream file(filename);
        if (!file.is_open())
            return false;

        for (const auto& [time, metrics] : _metrics) {
            file << FormatTime(time);
            for (const auto& metric_ptr : metrics) {
                file << " ";
                metric_ptr->WriteToStream(file);
            }
            file << "\n";
        }
        file.close();
        return true;
    }

    ~MetricsCollector() 
    {
        std::lock_guard lock(_mutex);
        _metrics.clear();
    }
};