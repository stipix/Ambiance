################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Projects/Common/BLE/Modules/RADIO_utils/Src/RADIO_utils.c 

OBJS += \
./Projects/Common/BLE/Modules/RADIO_utils/Src/RADIO_utils.o 

C_DEPS += \
./Projects/Common/BLE/Modules/RADIO_utils/Src/RADIO_utils.d 


# Each subdirectory must supply rules for building sources it contributes
Projects/Common/BLE/Modules/RADIO_utils/Src/%.o Projects/Common/BLE/Modules/RADIO_utils/Src/%.su Projects/Common/BLE/Modules/RADIO_utils/Src/%.cyclo: ../Projects/Common/BLE/Modules/RADIO_utils/Src/%.c Projects/Common/BLE/Modules/RADIO_utils/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32WB05 -DUSE_NUCLEO_64 -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I../Drivers/STM32WB0x_HAL_Driver/Inc -I../Drivers/STM32WB0x_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32WB0X/Include -I../Drivers/CMSIS/Include -I"C:/Users/stipi/Desktop/HWstuff/ECE129/WildlifeSource/WPSSource/WildlifeSource/Drivers/BSP/STM32WB0x-nucleo" -I../Drivers/BSP/STM32WB0x-nucleo -I../STM32_BLE/App -I../STM32_BLE/Target -I../System/Config/Debug_GPIO -I../System/Interfaces -I../Utilities/trace/adv_trace -I../Projects/Common/BLE/Interfaces -I../Projects/Common/BLE/Modules -I../Projects/Common/BLE/Modules/RTDebug -I../Projects/Common/BLE/Modules/RADIO_utils/Inc -I../Projects/Common/BLE/Modules/Profiles/Inc -I../Projects/Common/BLE/Modules/PKAMGR/Inc -I../Projects/Common/BLE/Modules/NVMDB/Inc -I../Projects/Common/BLE/Modules/Flash -I../Projects/Common/BLE/Startup -I../Utilities/misc -I../Utilities/sequencer -I../Utilities/lpm/tiny_lpm -I../Middlewares/ST/STM32_BLE -I../Middlewares/ST/STM32_BLE/cryptolib/Inc -I../Middlewares/ST/STM32_BLE/cryptolib/Inc/Common -I../Middlewares/ST/STM32_BLE/cryptolib/Inc/AES -I../Middlewares/ST/STM32_BLE/cryptolib/Inc/AES/CBC -I../Middlewares/ST/STM32_BLE/cryptolib/Inc/AES/CMAC -I../Middlewares/ST/STM32_BLE/cryptolib/Inc/AES/Common -I../Middlewares/ST/STM32_BLE/cryptolib/Inc/AES/ECB -I../Middlewares/ST/STM32_BLE/evt_handler/inc -I../Middlewares/ST/STM32_BLE/queued_writes/inc -I../Middlewares/ST/STM32_BLE/stack/include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Projects-2f-Common-2f-BLE-2f-Modules-2f-RADIO_utils-2f-Src

clean-Projects-2f-Common-2f-BLE-2f-Modules-2f-RADIO_utils-2f-Src:
	-$(RM) ./Projects/Common/BLE/Modules/RADIO_utils/Src/RADIO_utils.cyclo ./Projects/Common/BLE/Modules/RADIO_utils/Src/RADIO_utils.d ./Projects/Common/BLE/Modules/RADIO_utils/Src/RADIO_utils.o ./Projects/Common/BLE/Modules/RADIO_utils/Src/RADIO_utils.su

.PHONY: clean-Projects-2f-Common-2f-BLE-2f-Modules-2f-RADIO_utils-2f-Src

