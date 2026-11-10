# The two VST3 plugins the hosting checks use (scripts\m10_vst3.txt). Not part of the normal build:
# "build.bat testplugins" builds them into build\release\testplugins.
# On macOS they are built as Audio Units too, for checking the AU hosting.
if(APPLE)
  set(PULP_TEST_PLUGIN_FORMATS VST3 AU)
else()
  set(PULP_TEST_PLUGIN_FORMATS VST3)
endif()

function(pulp_test_plugin target name code synth)
  juce_add_plugin(${target}
    PRODUCT_NAME "${name}"
    COMPANY_NAME "PULP"
    PLUGIN_MANUFACTURER_CODE Pulp
    PLUGIN_CODE ${code}
    FORMATS ${PULP_TEST_PLUGIN_FORMATS}
    IS_SYNTH ${synth}
    NEEDS_MIDI_INPUT ${synth}
    NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_AUTO_MANIFEST FALSE
    NEEDS_CURL FALSE
    NEEDS_WEB_BROWSER FALSE)
  target_sources(${target} PRIVATE TestPlugin.cpp)
  if(${synth})
    target_compile_definitions(${target} PRIVATE PULP_TEST_SYNTH=1)
  else()
    target_compile_definitions(${target} PRIVATE PULP_TEST_SYNTH=0)
  endif()
  target_compile_definitions(${target} PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0)
  target_link_libraries(${target} PRIVATE juce::juce_audio_processors juce::juce_recommended_config_flags)
endfunction()

pulp_test_plugin(pulp_test_tone "PULP Test Tone" PtTn TRUE)
pulp_test_plugin(pulp_test_delay "PULP Test Delay" PtDl FALSE)
if(APPLE)
  add_custom_target(testplugins DEPENDS pulp_test_tone_VST3 pulp_test_delay_VST3 pulp_test_tone_AU pulp_test_delay_AU)
else()
  add_custom_target(testplugins DEPENDS pulp_test_tone_VST3 pulp_test_delay_VST3)
endif()
