cmake_minimum_required(VERSION 3.22)
project(Bailarina VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "")

# JUCE: usa -DJUCE_DIR=/ruta/a/JUCE si ya lo tienes; si no, se descarga.
set(JUCE_DIR "" CACHE PATH "Ruta a una copia local de JUCE")
if(JUCE_DIR AND EXISTS "${JUCE_DIR}/CMakeLists.txt")
    add_subdirectory("${JUCE_DIR}" juce EXCLUDE_FROM_ALL)
else()
    include(FetchContent)
    FetchContent_Declare(juce GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(juce)
endif()

set(BAILARINA_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND BAILARINA_FORMATS AU)
endif()

juce_add_plugin(Bailarina
    COMPANY_NAME "Eloy Platas"
    PLUGIN_MANUFACTURER_CODE Elpl
    PLUGIN_CODE Blrn
    FORMATS ${BAILARINA_FORMATS}
    PRODUCT_NAME "Bailarina"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Instrument Synth)

juce_add_binary_data(BailarinaAssets SOURCES assets/SpecialElite.ttf)

target_sources(Bailarina PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/LumePanel.cpp)

target_compile_definitions(Bailarina PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0)

target_link_libraries(Bailarina
    PRIVATE juce::juce_audio_utils BailarinaAssets
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags)

# Render offline del motor, sin JUCE (para probar el sonido)
add_executable(render_demo tools/render_demo.cpp)

if(MSVC)
    target_compile_options(Bailarina PRIVATE /utf-8)
endif()
