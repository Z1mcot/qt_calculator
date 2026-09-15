# Qt/QML калькулятор — тестовое задание

## Структура

```
CMakeLists.txt        — сборка основного приложения
src/                   — C++ (backend, движки, поток вычислений, очереди)
qml/Main.qml           — интерфейс (QML)
external_lib/           — отдельный проект: пример внешней библиотеки с DoIt()
```

## Сборка

Нужны Qt6 (Quick, Qml, Core) и CMake ≥ 3.16.

```bash
# 1. Внешняя библиотека (аналог DLL/SO с DoIt)
cd external_lib && mkdir build && cd build
cmake .. && cmake --build .
cd ../..

# 2. Основное приложение
mkdir build && cd build
cmake .. && cmake --build .

# 3. Положить внешнюю библиотеку рядом с исполняемым файлом,
#    чтобы приложение нашло её при старте
cp ../external_lib/build/libdoit.so .

./QtCalculator
```

На Windows `libdoit.so` → `doit.dll` (сборка внешней библиотеки под Windows
даст `.dll`; `ExternalEngine` сам выбирает нужное расширение).
