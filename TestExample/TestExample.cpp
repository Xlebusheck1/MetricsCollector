#include "MetricsCollector.h"
#include <iostream>
#include <thread>
#include <random>
#include <chrono>
#include <vector>
#include <iomanip>
using namespace std;

template <typename T>
T random_value(const string& metric_name, T min, T max) 
{
    static random_device rd;
    static mt19937 gen(rd());

    T value;
    if constexpr (is_integral_v<T>) {
        uniform_int_distribution<T> dis(min, max);
        value = dis(gen);
    }
    else {
        uniform_real_distribution<T> dis(min, max);
        value = dis(gen);
    }

    cout << "[GEN] " << metric_name << ": " << value << endl;
    return value;
}

void print_current_time() 
{
    auto now = chrono::system_clock::now();
    auto now_time = chrono::system_clock::to_time_t(now);

    tm time_struct;
    localtime_s(&time_struct, &now_time);

    cout << put_time(&time_struct, "[%H:%M:%S] ");
}


void test_basic_functionality() //Тест базовой функциональности с разными типами данных
{
    print_current_time();
    cout << "=== Starting Basic Functionality Test ===\n";

    MetricsCollector collector;
    
    collector.AddMetric("Temperature (C)", 23.5);
    collector.AddMetric("Pressure (hPa)", 1012);
    collector.AddMetric("Humidity (%)", 45.0f);
    collector.AddMetric("Is_Raining", true);
    collector.AddMetric("Status", string("Normal"));
    
    vector<Metric<int>> metrics = {
        {"Error_Count", 0},
        {"Warning_Count", 2},
        {"Users_Online", 154}
    };
    collector.AddMetrics(metrics.begin(), metrics.end());
    
    if (collector.SaveToFile("basic_metrics.txt")) {
        print_current_time();
        cout << "Metrics saved to basic_metrics.txt\n";
    }
    else {
        print_current_time();
        cerr << "Failed to save metrics!\n";
    }

    print_current_time();
    cout << "=== Basic Test Completed ===\n\n";
}

void test_multithreading() 
{
    print_current_time();
    cout << "=== Starting Multithreading Test ===\n";

    MetricsCollector collector;
    atomic<bool> running{ true };

    // Коды цветов
    const string red = "\033[31m";
    const string green = "\033[32m";
    const string blue = "\033[34m";
    const string reset = "\033[0m";
    
    auto cpu_thread = thread([&]() 
        {
            while (running)
            {
                double load = random_value("CPU_Load", 0.0, 4.0);
                collector.AddMetric("CPU_Load", load);

                print_current_time();
                cout << blue << "[CPU] Added: " << load << reset << endl;

                this_thread::sleep_for(chrono::milliseconds(200));
            }
        });
    
    auto mem_thread = thread([&]() 
        {
            while (running)
            {
                double usage = random_value("Memory_Usage", 0.0, 100.0);
                collector.AddMetric("Memory_Usage", usage);

                print_current_time();
                cout << green << "[MEM] Added: " << usage << "%" << reset << endl;

                this_thread::sleep_for(chrono::milliseconds(300));
            }
        });

    auto net_thread = thread([&]() 
        {
            while (running)
            {
                int requests = random_value("Network_Requests", 0, 500);
                collector.AddMetric("Network_Requests", requests);

                print_current_time();
                cout << red << "[NET] Added: " << requests << " req/s" << reset << endl;

                this_thread::sleep_for(chrono::milliseconds(150));
            }
        });
   
    this_thread::sleep_for(chrono::seconds(5));
    running = false;

    cpu_thread.join();
    mem_thread.join();
    net_thread.join();

    if (collector.SaveToFile("thread_metrics.txt")) 
    {
        print_current_time();
        cout << "Thread metrics saved to thread_metrics.txt\n";
    }

    print_current_time();
    cout << "=== Multithreading Test Completed ===\n\n";
}

// Тест с реальными сценариями
void test_real_world_scenario() 
{
    print_current_time();
    cout << "=== Starting Real-World Scenario Test ===\n";

    MetricsCollector collector;
    
    for (int i = 0; i < 10; ++i) 
    {
        // Системные метрики
        collector.AddMetric("CPU_Utilization", random_value("CPU", 0.5, 3.8));
        collector.AddMetric("Memory_Usage_GB", random_value("Memory", 2.1, 3.9));
        collector.AddMetric("Disk_IO_MBps", random_value("Disk", 5.0, 120.0));

        // Метрики приложения
        collector.AddMetric("Active_Users", random_value("Users", 100, 500));
        collector.AddMetric("API_Requests", random_value("API", 50, 300));
        collector.AddMetric("Cache_Hit_Rate", random_value("Cache", 0.7, 0.99));

        // Состояние сервисов
        collector.AddMetric("DB_Response_Time_ms", random_value("DB", 2.0, 50.0));
        collector.AddMetric("Service_Availability", random_value("Availability", 0.99, 1.0));

        print_current_time();
        cout << "Added server metrics batch #" << i + 1 << endl;

        this_thread::sleep_for(chrono::milliseconds(1000));
    }

    if (collector.SaveToFile("server_metrics.txt")) 
    {
        print_current_time();
        cout << "Server metrics saved to server_metrics.txt\n";
    }

    print_current_time();
    cout << "=== Real-World Test Completed ===\n\n";
}

int main() 
{
    try 
    {
        cout << "\n=== Starting Metrics Collector Tests ===\n\n";

        test_basic_functionality();       
        test_multithreading();
        test_real_world_scenario();

        cout << "\n=== All Tests Completed Successfully ===\n";
        return 0;
    }
    catch (const exception& e)
    {
        print_current_time();
        cerr << "ERROR: " << e.what() << endl;
        return 1;
    }
}