idf_component_register(
    SRCS
        "AudioDriver.cpp"
        "Interpolator.cpp"
        "WaveSynth.cpp"
        "YummyDSP.cpp"
        "Nodes/DelayNode.cpp"
        "Nodes/FilterNode.cpp"
        "Nodes/MixerNode.cpp"
        "Nodes/WaveShaperNode.cpp"
    INCLUDE_DIRS "."
    REQUIRES arduino
)
