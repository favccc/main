# variable

set(WAV_DEMUXER_TEST test_wav_demuxer)

# targets

aux_source_directory(. src_wav_demuxer)
aux_source_directory(test src_wav_demuxer_test)

include_directories(
    ${G_XTOOLS_XLOG_HEADER_PATH}
    ${G_XWAV_DEMUXER_HEADER_PATH}
)

add_library(${G_XWAV_DEMUXER}
    SHARED
    ${src_wav_demuxer})

# tests

if(${gnu})
set(COMMON_LIB gtest ${G_XWAV_DEMUXER} ${G_XTOOLS} pthread)
elseif(${msvc})
set(COMMON_LIB gtest ${G_XWAV_DEMUXER} ${G_XTOOLS})
endif()

add_definitions(-DRES_AUDIO_PATH="${G_RESOURCE_PATH}/audio")

add_executable(${WAV_DEMUXER_TEST}
    ${src_wav_demuxer_test})
target_link_libraries(${WAV_DEMUXER_TEST}
    ${COMMON_LIB})
add_test(NAME ${WAV_DEMUXER_TEST}
    COMMAND ${WAV_DEMUXER_TEST})