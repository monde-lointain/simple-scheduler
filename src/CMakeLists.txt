add_executable(simple-scheduler main.c led.c)

target_include_directories(simple-scheduler PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(simple-scheduler PRIVATE
    CMSIS::STM32::F407VG
    STM32::Nano
)
stm32_generate_extra_files(simple-scheduler)
