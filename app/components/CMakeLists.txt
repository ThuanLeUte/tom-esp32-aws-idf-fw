set(COMPONENT_SRCS "./blufi_security/blufi_security.c" 
                   "./frozen-1.6/frozen.c"
                   "./one_button/one_button.c"
                   "./protocol/aws_builder.c"
                   "./protocol/aws_parser.c"
                   )

set(COMPONENT_ADD_INCLUDEDIRS .
                              "./blufi_security"
                              "./frozen-1.6"
                              "./one_button"
                              "./aws_iot/aws-iot-device-sdk-embedded-C/include"
                              "./aws_iot/include"
                              "./protocol"
                              "../platform"
                              "../sys"
                              )

set(COMPONENT_REQUIRES mbedtls)
set(COMPONENT_PRIV_REQUIRES  jsmn )

register_component()