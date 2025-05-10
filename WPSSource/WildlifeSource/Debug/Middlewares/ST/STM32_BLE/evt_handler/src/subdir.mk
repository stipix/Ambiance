################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/ST/STM32_BLE/evt_handler/src/ble_evt.c 

OBJS += \
./Middlewares/ST/STM32_BLE/evt_handler/src/ble_evt.o 

C_DEPS += \
./Middlewares/ST/STM32_BLE/evt_handler/src/ble_evt.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/ST/STM32_BLE/evt_handler/src/%.o Middlewares/ST/STM32_BLE/evt_handler/src/%.su Middlewares/ST/STM32_BLE/evt_handler/src/%.cyclo: ../Middlewares/ST/STM32_BLE/evt_handler/src/%.c Middlewares/ST/STM32_BLE/evt_handler/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32WB05 -DUSE_NUCLEO_64 -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I../Drivers/STM32WB0x_HAL_Driver/Inc -I../Drivers/STM32WB0x_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32WB0X/Include -I../Drivers/CMSIS/Include -I"C:/Users/stipi/Desktop/HWstuff/ECE129/WildlifeSource/WPSSource/WildlifeSource/Drivers/BSP/STM32WB0x-nucleo" -I../Drivers/BSP/STM32WB0x-nucleo -I../STM32_BLE/App -I../STM32_BLE/Target -I../System/Config/Debug_GPIO -I../System/Interfaces -I../Utilities/trace/adv_trace -I../Projects/Common/BLE/Interfaces -I../Projects/Common/BLE/Modules -I../Projects/Common/BLE/Modules/RTDebug -I../Projects/Common/BLE/Modules/RADIO_utils/Inc -I../Projects/Common/BLE/Modules/Profiles/Inc -I../Projects/Common/BLE/Modules/PKAMGR/Inc -I../Projects/Common/BLE/Modules/NVMDB/Inc -I../Projects/Common/BLE/Modules/Flash -I../Projects/Common/BLE/Startup -I../Utilities/misc -I../Utilities/sequencer -I../Utilities/lpm/tiny_lpm -I../Middlewares/ST/STM32_BLE -I../Middlewares/ST/STM32_BLE/cryptolib/Inc -I../Middlewares/ST/STM32_BLE/cryptolib/Inc/Common -I../Middlewares/ST/STM32_BLE/cryptolib/Inc/AES -I../Middlewares/ST/STM32_BLE/cryptolib/Inc/AES/CBC -I../Middlewares/ST/STM32_BLE/cryptolib/Inc/AES/CMAC -I../Middlewares/ST/STM32_BLE/cryptolib/Inc/AES/Common -I../Middlewares/ST/STM32_BLE/cryptolib/Inc/AES/ECB -I../Middlewares/ST/STM32_BLE/evt_handler/inc -I../Middlewares/ST/STM32_BLE/queued_writes/inc -I../Middlewares/ST/STM32_BLE/stack/include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Middlewares-2f-ST-2f-STM32_BLE-2f-evt_handler-2f-src

clean-Middlewares-2f-ST-2f-STM32_BLE-2f-evt_handler-2f-src:
	-$(RM) ./Middlewares/ST/STM32_BLE/evt_handler/src/ble_evt.cyclo ./Middlewares/ST/STM32_BLE/evt_handler/src/ble_evt.d ./Middlewares/ST/STM32_BLE/evt_handler/src/ble_evt.o ./Middlewares/ST/STM32_BLE/evt_handler/src/ble_evt.su

.PHONY: clean-Middlewares-2f-ST-2f-STM32_BLE-2f-evt_handler-2f-src

