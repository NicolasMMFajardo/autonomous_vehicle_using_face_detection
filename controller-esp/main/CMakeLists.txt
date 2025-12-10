idf_component_register(
    SRCS 
        "hello_world_main.c"
        "wifi.c"
        "pid.c"
        "motors.c"
        "servo.c"
    INCLUDE_DIRS "."
    REQUIRES esp_wifi esp_netif nvs_flash lwip driver
)