set(COMPONENT_ADD_INCLUDEDIRS ./include
                            ./driver/es8388_adf
                            ./driver/es8374
                            ./driver/es8311_adf
                            ./driver/es8156_adf
                            ./driver/es7243
                            ./driver/es7148
                            ./driver/es7210_adf
                            ./driver/es7243e_adf
                            ./driver/tas5805m
                            # ./driver/zl38063
                            # ./driver/zl38063/api_lib
                            # ./driver/zl38063/example_apps
                            # ./driver/zl38063/firmware
                            ./driver/include)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES audio_sal audio_board mbedtls esp_peripherals esp_dispatcher)

set(COMPONENT_SRCS ./audio_hal_adf.c
                    ./audio_volume.c
                    ./driver/es8388_adf/es8388_adf.c
                    ./driver/es8388_adf/headphone_detect.c
                    ./driver/es8374/es8374.c
                    ./driver/es8311_adf/es8311_adf.c
                    ./driver/es8156_adf/es8156_adf.c
                    ./driver/es7243/es7243.c
                    ./driver/es7148/es7148.c
                    ./driver/es7210_adf/es7210_adf.c
                    ./driver/es7243e_adf/es7243e_adf.c
                    ./driver/tas5805m/tas5805m.c
                    # ./driver/zl38063/zl38063.c
                    # ./driver/zl38063/api_lib/vprocTwolf_access.c
                    # ./driver/zl38063/api_lib/vproc_common.c
                    # ./driver/zl38063/example_apps/tw_hal_verify.c
                    # ./driver/zl38063/example_apps/tw_ldcfg.c
                    # ./driver/zl38063/example_apps/tw_ldfw.c
                    # ./driver/zl38063/example_apps/tw_ldfwcfg.c
                    # ./driver/zl38063/example_apps/tw_spi_access.c
                    )

register_component()
# IF (NOT (CONFIG_IDF_TARGET STREQUAL "esp32c3"))
# target_link_libraries(${COMPONENT_TARGET} INTERFACE "-L${CMAKE_CURRENT_LIST_DIR}/driver/zl38063/firmware")
# target_link_libraries(${COMPONENT_TARGET} INTERFACE firmware)
# ENDIF()
