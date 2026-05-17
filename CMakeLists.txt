cmake_minimum_required(VERSION 3.22)

project(SceneLooper VERSION 0.9.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.12
)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(AtmocycleFigmaAssets
    SOURCES
        Source/Assets/FigmaUI/atmocycle_figma_ui.png
        Source/Assets/FigmaUI/control_states_reference.png
        Source/Assets/FigmaUI/master_knobs_reference.png
        Source/Assets/FigmaUI/load_scene_button.png
        Source/Assets/FigmaUI/save_scene_button.png
        Source/Assets/FigmaUI/randomization_button.png)

juce_add_plugin(SceneLooper
    COMPANY_NAME "Echosynthesis"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Sclp
    PLUGIN_CODE Slp1
    FORMATS VST3
    PRODUCT_NAME "Atmocycle")

target_sources(SceneLooper
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)

if(APPLE)
    add_custom_command(TARGET SceneLooper_VST3 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<TARGET_BUNDLE_DIR:SceneLooper_VST3>"
            "$<TARGET_BUNDLE_DIR:SceneLooper_VST3>/../SceneLooper.vst3")
endif()

target_compile_definitions(SceneLooper
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(SceneLooper
    PRIVATE
        AtmocycleFigmaAssets
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
