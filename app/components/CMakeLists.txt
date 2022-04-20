set(COMPONENT_SRCS "./blufi_security/blufi_security.c" 
                   "./frozen-1.6/frozen.c"
                   "./one_button/one_button.c"
                   )

set(COMPONENT_ADD_INCLUDEDIRS .
                              "./blufi_security"
                              "./frozen-1.6"
                              "./one_button"
                              "./aws_iot/aws-iot-device-sdk-embedded-C/include"
                              "./aws_iot/include")

set(COMPONENT_REQUIRES)
set(COMPONENT_PRIV_REQUIRES  esp-adf-libs esp_peripherals jsmn)

register_component()