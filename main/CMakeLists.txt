set(COMPONENT_SRCS app_main.c
    app_nvs.c
    app_wifi.c
    app_ota.c
    app_sensor.c
    )

set(COMPONENT_ADD_INCLUDEDIRS include)

set(COMPONENT_REQUIRES
    nvs_flash
    app_update
    )

register_component()
