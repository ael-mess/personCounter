set(COMPONENT_SRCS app_main.c
    app_nvs.c
    app_wifi.c
    app_mqtt.c
    app_ota.c
    app_sensor.c
    app_main.c
    )

set(COMPONENT_ADD_INCLUDEDIRS include)

set(COMPONENT_REQUIRES
    nvs_flash
    mqtt
    app_update
    )

register_component()
