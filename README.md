# Библиотека для сбора метрик на C++

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/17)

Легковесная потокобезопасная библиотека для сбора и записи метрик в текстовый файл. Библиотека позволяет 
создавать и записывать в файл как метрики стандартных типов значений (int, double, string, char* и т.д.), так 
и специальные метрики, наследуемые от абстрактного класса BaseMetric. Метрики можно записывать как по одной, так 
и несколько сразу с помощью любых STL-контейнеров (кроме std::map, std::unordered_map, std::pair). При создании 
метрики при отсутствии передаваемого параметра времени будет создан свой параметр времени. Метрики с одинаковым 
временем (или примерно одинаковым) будут записаны в 1 строчку. В репозитории присутвтвует пример работы библиотеки.

## Диаграмма классов библиотеки:
![ДИАГРАММА КЛАССОВ](https://github.com/user-attachments/assets/f5c8dfb4-7f06-45a0-b4a5-1d2fd33a6ee8)

## Работа с библиотекой

1. Подключите заголовочный файл:
```cpp
#include "MetricsCollector.h"
```
2. Создайте экземпляр коллектора:
```cpp
MetricsCollector collector;
```
3. Добавляйте метрики:
```cpp
// Одиночная метрика
collector.AddMetric("CPU", 0.97);

// Несколько метрик сразу
collector.AddMetrics({
    Metric<double>("CPU", 1.12),
    Metric<double>("Memory Usage", 30.34)
});
```
4. Сохраните в файл:
```cpp
if (collector.SaveToFile("metrics.log")) {
    std::cout << "Метрики успешно сохранены" << std::endl;
}
```
