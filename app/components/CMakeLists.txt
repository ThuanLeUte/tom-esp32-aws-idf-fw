set(COMPONENT_SRCS "./blufi_security/blufi_security.c" 
                   "./frozen-1.6/frozen.c"
                   "./one_button/one_button.c"
                   "./protocol/aws_builder.c"
                   "./protocol/aws_parser.c"
                   "./lib_adf/audio_mem.c"
                   "./lib_adf/audio_thread.c"
                   "./lib_adf/esp_delegate.c"
                   "./lib_adf/esp_dispatcher.c"
                   "./lib_adf/nvs_action.c"
                   "./lib_adf/wifi_ssid_manager.c"
                   "./bsp/bsp_timer.c"
                   )

set(COMPONENT_ADD_INCLUDEDIRS .
                              "./blufi_security"
                              "./frozen-1.6"
                              "./one_button"
                              "./aws_iot/aws-iot-device-sdk-embedded-C/include"
                              "./aws_iot/include"
                              "./protocol"
                              "./lib_adf"
                              "./bsp"
                              "../platform"
                              "../sys"
                              )

set(COMPONENT_REQUIRES mbedtls nvs_flash)
set(COMPONENT_PRIV_REQUIRES  jsmn )

register_component()