cmake_minimum_required(VERSION 3.22)

project(SceneLooper VERSION 1.0.0)

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
        Source/Assets/FigmaUI/small_knobs_reference.png
        Source/Assets/FigmaUI/small_knob_frames.png
        Source/Assets/FigmaUI/master_knob_frames.png
        Source/Assets/FigmaUI/asset_power_off.png
        Source/Assets/FigmaUI/asset_power_on.png
        Source/Assets/FigmaUI/asset_solo_off.png
        Source/Assets/FigmaUI/asset_solo_on.png
        Source/Assets/FigmaUI/asset_layer_load_normal.png
        Source/Assets/FigmaUI/asset_layer_load_hover.png
        Source/Assets/FigmaUI/asset_layer_load_down.png
        Source/Assets/FigmaUI/asset_ap_off.png
        Source/Assets/FigmaUI/asset_ap_on.png
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
    FORMATS VST3 AU AAX
    PRODUCT_NAME "Atmocycle")

target_sources(SceneLooper
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)

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
