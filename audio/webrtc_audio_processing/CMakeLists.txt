project(main)
cmake_minimum_required(VERSION 3.0)

# bin names
set(LIBNAME_WEBTRC_AUDIO webrtc_audio)
set(TEST_LOUDNESS_HISTOGRAM test_loudness_histogram)

# paths
set(MODULES_PATH ${CMAKE_SOURCE_DIR}/modules)
set(MODULE_AUDIO_PROCESSING_PATH ${MODULES_PATH}/audio_processing)
set(MODULE_AUDIO_PROCESSING_AGC_PATH ${MODULE_AUDIO_PROCESSING_PATH}/agc)
set(MODULE_AUDIO_PROCESSING_AGC2_PATH ${MODULE_AUDIO_PROCESSING_PATH}/agc2)
set(MODULE_AUDIO_PROCESSING_VAD_PATH ${MODULE_AUDIO_PROCESSING_PATH}/vad)
set(MODULE_AUDIO_PROCESSING_LOGGING_PATH ${MODULE_AUDIO_PROCESSING_PATH}/logging)
set(COMMON_AUDIO_PATH ${CMAKE_SOURCE_DIR}/common_audio)
set(COMMON_AUDIO_VAD_PATH ${COMMON_AUDIO_PATH}/vad)
set(COMMON_AUDIO_SIGNAL_PROCESSING ${COMMON_AUDIO_PATH}/signal_processing)
set(COMMON_AUDIO_THIRD_PARTY_OOURA_FFT_SIZE_256_PATH ${COMMON_AUDIO_PATH}/third_party/ooura/fft_size_256)
set(COMMON_AUDIO_RESAMPLER_PATH ${COMMON_AUDIO_PATH}/resampler)
set(MODULE_AUDIO_CODING_CODECS_ISAC_PATH ${MODULES_PATH}/audio_coding/codecs/isac/main/source)
set(RTC_BASE_PATH ${CMAKE_SOURCE_DIR}/rtc_base)
set(SYSTEM_WRAPPERS_SOURCE ${CMAKE_SOURCE_DIR}/system_wrappers/source)

# deps/abseil
set(ABSEIL_INCLUDE_PATH /home/test/opensrc/abseil/build/output/include)
set(ABSEIL_LIB_PATH /home/test/opensrc/abseil/build/output/lib)

# deps/gtest
set(GTEST_INCLUDE_PATH /home/test/opensrc/googletest/build/output/include)
set(GTEST_LIB_PATH /home/test/opensrc/googletest/build/output/lib)
set(GTEST_TESTING_INCLUDE_PATH /home/test/opensrc/googletest/build/output)

# source
set(SRC_AGC
    ${MODULE_AUDIO_PROCESSING_AGC_PATH}/agc_manager_direct.cc
    ${MODULE_AUDIO_PROCESSING_AGC_PATH}/agc.cc
    ${MODULE_AUDIO_PROCESSING_AGC_PATH}/loudness_histogram.cc
    ${MODULE_AUDIO_PROCESSING_AGC_PATH}/utility.cc
)

set(SRC_AGC2
    ${MODULE_AUDIO_PROCESSING_AGC2_PATH}/input_volume_stats_reporter.cc)

set(SRC_VAD
    ${MODULE_AUDIO_PROCESSING_VAD_PATH}/voice_activity_detector.cc
    ${MODULE_AUDIO_PROCESSING_VAD_PATH}/pitch_based_vad.cc
    ${MODULE_AUDIO_PROCESSING_VAD_PATH}/vad_audio_proc.cc
    ${MODULE_AUDIO_PROCESSING_VAD_PATH}/standalone_vad.cc
    ${MODULE_AUDIO_PROCESSING_VAD_PATH}/vad_circular_buffer.cc
    ${MODULE_AUDIO_PROCESSING_VAD_PATH}/gmm.cc
    ${MODULE_AUDIO_PROCESSING_VAD_PATH}/pole_zero_filter.cc
    ${MODULE_AUDIO_PROCESSING_VAD_PATH}/pitch_internal.cc
)

set(SRC_COMMON_AUDIO_VAD
    ${COMMON_AUDIO_VAD_PATH}/webrtc_vad.c
    ${COMMON_AUDIO_VAD_PATH}/vad_core.c
    ${COMMON_AUDIO_VAD_PATH}/vad_gmm.c
    ${COMMON_AUDIO_VAD_PATH}/vad_sp.c
    ${COMMON_AUDIO_VAD_PATH}/vad_filterbank.c
)

set(SRC_SIGNAL_PROCESSING
    ${COMMON_AUDIO_SIGNAL_PROCESSING}/resample_48khz.c
    ${COMMON_AUDIO_SIGNAL_PROCESSING}/resample.c
    ${COMMON_AUDIO_SIGNAL_PROCESSING}/resample_by_2.c
    ${COMMON_AUDIO_SIGNAL_PROCESSING}/division_operations.c
    ${COMMON_AUDIO_SIGNAL_PROCESSING}/energy.c
    ${COMMON_AUDIO_SIGNAL_PROCESSING}/resample_by_2_internal.c
    ${COMMON_AUDIO_SIGNAL_PROCESSING}/resample_fractional.c
    ${COMMON_AUDIO_SIGNAL_PROCESSING}/get_scaling_square.c)

set(SRC_ISAC
    ${MODULE_AUDIO_CODING_CODECS_ISAC_PATH}/isac_vad.c
    ${MODULE_AUDIO_CODING_CODECS_ISAC_PATH}/filter_functions.c
    ${MODULE_AUDIO_CODING_CODECS_ISAC_PATH}/pitch_estimator.c
    ${MODULE_AUDIO_CODING_CODECS_ISAC_PATH}/pitch_filter.c)

set(SRC_THIRD_OOURA
    ${COMMON_AUDIO_THIRD_PARTY_OOURA_FFT_SIZE_256_PATH}/fft4g.cc)

set(SRC_RESAMPLE
    ${COMMON_AUDIO_RESAMPLER_PATH}/resampler.cc)

set(SRC_RTC_BASE
    ${RTC_BASE_PATH}/checks.cc
#    ${RTC_BASE_PATH}/logging.cc
)

set(SRC_AUDIO_PROCESSING_LOGGING
    ${MODULE_AUDIO_PROCESSING_LOGGING_PATH}/apm_data_dumper.cc)

set(SRC_SYSTEM_WRAPPER
    ${SYSTEM_WRAPPERS_SOURCE}/metrics.cc
    ${SYSTEM_WRAPPERS_SOURCE}/field_trial.cc)

# common include directories
set(COMMON_INC_DIRECTORY 
    ${ABSEIL_INCLUDE_PATH}
    ${CMAKE_SOURCE_DIR}
    ${GTEST_TESTING_INCLUDE_PATH}
    ${GTEST_INCLUDE_PATH})

add_library(${LIBNAME_WEBTRC_AUDIO} SHARED
    ${SRC_AGC}
    ${SRC_AGC2}
    ${SRC_VAD}
    ${SRC_COMMON_AUDIO_VAD}
    ${SRC_RTC_BASE}
    ${SRC_ISAC}
    ${SRC_RESAMPLE}
    ${SRC_THIRD_OOURA}
    ${SRC_SIGNAL_PROCESSING}
    ${SRC_AUDIO_PROCESSING_LOGGING}
    ${SRC_SYSTEM_WRAPPER})

target_include_directories(
    ${LIBNAME_WEBTRC_AUDIO}
    PRIVATE ${COMMON_INC_DIRECTORY}
)

target_compile_definitions(${LIBNAME_WEBTRC_AUDIO}
    PRIVATE WEBRTC_APM_DEBUG_DUMP=0
    PRIVATE WEBRTC_POSIX
    RTC_DISABLE_LOGGING # logging relies some modules
)

# test: loudness_histogram
add_executable(${TEST_LOUDNESS_HISTOGRAM}
    ${MODULE_AUDIO_PROCESSING_AGC_PATH}/loudness_histogram_unittest.cc)
target_link_libraries(${TEST_LOUDNESS_HISTOGRAM}
    PRIVATE ${LIBNAME_WEBTRC_AUDIO} gtest)
target_link_directories(${TEST_LOUDNESS_HISTOGRAM} 
    PRIVATE ${GTEST_LIB_PATH})
target_include_directories(${TEST_LOUDNESS_HISTOGRAM}
    PRIVATE ${COMMON_INC_DIRECTORY})
