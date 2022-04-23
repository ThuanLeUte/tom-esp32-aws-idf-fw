set(COMPONENT_SRCS "sys.c" 
                   "sys_nvs.c"
                   "sys_wifi.c"
                   "sys_aws_provision.c"
                   "sys_aws_provision.c"
                   "sys_aws_shadow.c"
                   "sys_aws_mqtt.c"
                   "sys_aws.c"
                   "sys_devcfg.c"
                   )

set(COMPONENT_ADD_INCLUDEDIRS .
                              "../platform"
                              "../components/aws_iot/aws-iot-device-sdk-embedded-C/include"
                              "../components/aws_iot/include"
                              "../components/frozen-1.6"
                              "../components/blufi_security"
                              "../components/one_button"
                              "../components/protocol"
                              "../components/lib_adf"
                              "../build"
                              )

set(COMPONENT_EMBED_TXTFILES certs/aws-root-ca.pem 
                             certs/certificate.pem.crt
                             certs/private.pem.key
                             )
 

set(COMPONENT_REQUIRES nvs_flash jsmn spiffs bt)

register_component()


