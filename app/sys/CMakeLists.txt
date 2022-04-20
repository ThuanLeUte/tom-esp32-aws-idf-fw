set(COMPONENT_SRCS "sys.c" 
                   "sys_nvs.c"
                   "sys_aws_provision.c"
                   "sys_wifi.c"
                   )

set(COMPONENT_ADD_INCLUDEDIRS .
                              "../platform"
                              "../components/aws_iot/aws-iot-device-sdk-embedded-C/include"
                              "../components/aws_iot/include"
                              "../components/frozen-1.6"
                              "../components/blufi_security"
                              "../components/one_button"
                              "../build"
                              )

set(COMPONENT_REQUIRES)
set(COMPONENT_PRIV_REQUIRES  esp-adf-libs esp_peripherals audio_stream jsmn wifi_service)

register_component()


