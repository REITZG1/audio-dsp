idf_component_register(
    SRCS "audio_mixer_p4.cpp" "web_server.cpp"
    INCLUDE_DIRS "."
    REQUIRES arduino yummydsp arduinojson
)
