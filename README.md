# i2c_test
Проект создан для тестирования драйверов I2C на различных платформах. Работу I2C можно проверить в отладчике, сравнив прочитанные и ожидаемый данные (см. Application.hpp)

## STM32F0
В качестве тестовой платы используется АКБ для G501M, вычитываются данные из микросхемы BQ. Сборка проекта:
mkdir build
cd buld
cmake .. -DCMAKE_TOOLCHAIN_FILE=DroneDevicePlatform/cmake/cortex-m0.cmake -DCMAKE_BUILD_TYPE=DEBUG -DBOARD=STM32F0
make

## STM32F1
В качестве тестовой платы используется АКБ для G425, вычитываются данные из микросхемы BQ. Сборка проекта:
mkdir build
cd buld
cmake .. -DCMAKE_TOOLCHAIN_FILE=DroneDevicePlatform/cmake/cortex-m3.cmake -DCMAKE_BUILD_TYPE=DEBUG -DBOARD=STM32F1
make

