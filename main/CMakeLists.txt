idf_component_register(
    SRCS "audio_mixer_p4.cpp" "web_server.cpp"
    INCLUDE_DIRS "."
    REQUIRES arduino yummydsp arduinojson nvs_flash esp_wifi esp_event esp_netif esp_http_server
)
