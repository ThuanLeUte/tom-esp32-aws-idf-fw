set(COMPONENT_SRCS "sys.c" 
                   "sys_nvs.c"
                   "sys_wifi.c"
                   "sys_aws_provision.c"
                   "sys_aws_provision.c"
                   "sys_aws_shadow.c"
                   "sys_aws_mqtt.c"
                   "sys_aws.c"
                   "sys_devcfg.c"
                   "sys_ota.c"
                   "sys_time.c"
                   "../sys/lox/lox-job.cpp"
                   "sys_http_server.c"
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
                              "../components/bsp"
                              "../build"
                              )

set(COMPONENT_EMBED_TXTFILES certs/aws-root-ca.pem 
                             certs/certificate.pem.crt
                             certs/private.pem.key
                             web/login.html
                             web/status.html
                             web/service.html
                             web/other.html
                             web/network.html
                             web/css/bootstrap.min.css
                             web/css/pixie-main.css
                             web/js/bootstrap.min.js
                             web/js/pixie-custom.js
                             web/js/jquery.min.js
                             web/images/ap.png
                             web/images/eye-close.png
                             web/images/light.png
                             web/images/network.png
                             web/images/other.png
                             web/images/periperal.png
                             web/images/reboot.png
                             web/images/service.png
                             web/images/status.png
                             web/images/timezone.png
                             web/images/upgrade.png
                             )
 

set(COMPONENT_REQUIRES nvs_flash 
                       jsmn spiffs
                       bt esp_http_client
                       esp_https_ota
                       esp-tls
                       app_update
                       esp_http_server
                       )

register_component()


