cmake_minimum_required(VERSION 3.9.2)

###############################################################################
# project name
###############################################################################
project(coreCoder)

if (MSVC)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:16777216")
endif()

###############################################################################
# target sources (libraries)
###############################################################################

if(UNIX AND NOT APPLE)
  set(ADD_MATH_LIB m)
endif()

# epToolLib
add_library(epToolLib
  mpegD_usac/epTool/src/bch.c
  mpegD_usac/epTool/src/bch.h
  mpegD_usac/epTool/src/bintoint.c
  mpegD_usac/epTool/src/calcoverhead.c
  mpegD_usac/epTool/src/calcoverhead.h
  mpegD_usac/epTool/src/concat.c
  mpegD_usac/epTool/src/concat.h
  mpegD_usac/epTool/src/crc.c
  mpegD_usac/epTool/src/dcm_io.c
  mpegD_usac/epTool/src/dcm_io.h
  mpegD_usac/epTool/src/eptool.c
  mpegD_usac/epTool/src/eptool.h
  mpegD_usac/epTool/src/feedframeclass.h
  mpegD_usac/epTool/src/interleave.c
  mpegD_usac/epTool/src/libeptool.c
  mpegD_usac/epTool/src/libeptool.h
  mpegD_usac/epTool/src/outinfo.c
  mpegD_usac/epTool/src/Rconcatenation.c
  mpegD_usac/epTool/src/reedsolomon.c
  mpegD_usac/epTool/src/rs.h
  mpegD_usac/epTool/src/sideinfoparse.h
  mpegD_usac/epTool/src/srcpc2.c
)
target_compile_definitions(epToolLib PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(epToolLib PUBLIC "${PROJECT_SOURCE_DIR}/mpegD_usac/epTool/src")
set_target_properties(epToolLib PROPERTIES FOLDER "Libraries/coreCoder")

# spatialDecLib
add_library(spatialDecLib
  mpegD_usac/mp4spatialdec/include/sac_dec_interface.h
  mpegD_usac/mp4spatialdec/src/sac_arbitraryTree.c
  mpegD_usac/mp4spatialdec/src/sac_bitdec.c
  mpegD_usac/mp4spatialdec/src/sac_bitdec.h
  mpegD_usac/mp4spatialdec/src/sac_bitinput.c
  mpegD_usac/mp4spatialdec/src/sac_bitinput.h
  mpegD_usac/mp4spatialdec/src/sac_blind.c
  mpegD_usac/mp4spatialdec/src/sac_blind.h
  mpegD_usac/mp4spatialdec/src/sac_calcM1andM2.c
  mpegD_usac/mp4spatialdec/src/sac_calcM1andM2.h
  mpegD_usac/mp4spatialdec/src/sac_chandefs.h
  mpegD_usac/mp4spatialdec/src/sac_config.c
  mpegD_usac/mp4spatialdec/src/sac_config.h
  mpegD_usac/mp4spatialdec/src/sac_dec.c
  mpegD_usac/mp4spatialdec/src/sac_dec.h
  mpegD_usac/mp4spatialdec/src/sac_decor.c
  mpegD_usac/mp4spatialdec/src/sac_decor.h
  mpegD_usac/mp4spatialdec/src/sac_hrtf.c
  mpegD_usac/mp4spatialdec/src/sac_hrtf.h
  mpegD_usac/mp4spatialdec/src/sac_hrtffilter.c
  mpegD_usac/mp4spatialdec/src/sac_huff_nodes.h
  mpegD_usac/mp4spatialdec/src/sac_hybfilter.c
  mpegD_usac/mp4spatialdec/src/sac_hybfilter.h
  mpegD_usac/mp4spatialdec/src/sac_interface.h
  mpegD_usac/mp4spatialdec/src/sac_mdct2qmf.c
  mpegD_usac/mp4spatialdec/src/sac_mdct2qmf.h
  mpegD_usac/mp4spatialdec/src/sac_mdct2qmf_wf_tables.c
  mpegD_usac/mp4spatialdec/src/sac_nlc_dec.c
  mpegD_usac/mp4spatialdec/src/sac_nlc_dec.h
  mpegD_usac/mp4spatialdec/src/sac_nodes_cld.c
  mpegD_usac/mp4spatialdec/src/sac_nodes_cpc.c
  mpegD_usac/mp4spatialdec/src/sac_nodes_icc.c
  mpegD_usac/mp4spatialdec/src/sac_nodes_ipd.c
  mpegD_usac/mp4spatialdec/src/sac_nodes_pt0.c
  mpegD_usac/mp4spatialdec/src/sac_nodes_reshape.c
  mpegD_usac/mp4spatialdec/src/sac_parallelReverb.c
  mpegD_usac/mp4spatialdec/src/sac_parallelReverb.h
  mpegD_usac/mp4spatialdec/src/sac_partiallyComplex.c
  mpegD_usac/mp4spatialdec/src/sac_polyphase.c
  mpegD_usac/mp4spatialdec/src/sac_polyphase.h
  mpegD_usac/mp4spatialdec/src/sac_process.c
  mpegD_usac/mp4spatialdec/src/sac_process.h
  mpegD_usac/mp4spatialdec/src/sac_resdecode.c
  mpegD_usac/mp4spatialdec/src/sac_resdecode.h
  mpegD_usac/mp4spatialdec/src/sac_resdefs.h
  mpegD_usac/mp4spatialdec/src/sac_reshapeBBEnv.c
  mpegD_usac/mp4spatialdec/src/sac_reshapeBBEnv.h
  mpegD_usac/mp4spatialdec/src/sac_reshuffdec.h
  mpegD_usac/mp4spatialdec/src/sac_reshuffdec1.c
  mpegD_usac/mp4spatialdec/src/sac_reshuffdec2.c
  mpegD_usac/mp4spatialdec/src/sac_reshuffdec3.c
  mpegD_usac/mp4spatialdec/src/sac_reshuffdec_intern.h
  mpegD_usac/mp4spatialdec/src/sac_reshuffinit.c
  mpegD_usac/mp4spatialdec/src/sac_reshuffinit.h
  mpegD_usac/mp4spatialdec/src/sac_reshufftables.c
  mpegD_usac/mp4spatialdec/src/sac_reshufftables.h
  mpegD_usac/mp4spatialdec/src/sac_resintrins.c
  mpegD_usac/mp4spatialdec/src/sac_resintrins.h
  mpegD_usac/mp4spatialdec/src/sac_restns.c
  mpegD_usac/mp4spatialdec/src/sac_restns.h
  mpegD_usac/mp4spatialdec/src/sac_sbrconst.h
  mpegD_usac/mp4spatialdec/src/sac_smoothing.c
  mpegD_usac/mp4spatialdec/src/sac_smoothing.h
  mpegD_usac/mp4spatialdec/src/sac_stream.h
  mpegD_usac/mp4spatialdec/src/sac_tonality.c
  mpegD_usac/mp4spatialdec/src/sac_tonality.h
  mpegD_usac/mp4spatialdec/src/sac_TPprocess.c
  mpegD_usac/mp4spatialdec/src/sac_TPprocess.h
  mpegD_usac/mp4spatialdec/src/sac_types.h
  mpegD_usac/mp4spatialdec/src/spatial_bitstreamreader.h
)
target_compile_definitions(spatialDecLib PRIVATE HRTF_DYNAMIC_UPDATE)
target_include_directories(spatialDecLib PUBLIC "${PROJECT_SOURCE_DIR}/mpegD_usac/mp4spatialdec/include"
                                         PUBLIC "${PROJECT_SOURCE_DIR}/mpegD_usac/mp4spatialdec/src")
target_link_libraries(spatialDecLib icCodecLib frameLib)
set_target_properties(spatialDecLib PROPERTIES FOLDER "Libraries/coreCoder")

# spatialEncLib
add_library(spatialEncLib
  mpegD_usac/mp4spatialenc/include/sac_enc.h
  mpegD_usac/mp4spatialenc/src/bitstream.c
  mpegD_usac/mp4spatialenc/src/defines.h
  mpegD_usac/mp4spatialenc/src/sac_bd_embedder.c
  mpegD_usac/mp4spatialenc/src/sac_bd_embedder.h
  mpegD_usac/mp4spatialenc/src/sac_bitstream_enc.h
  mpegD_usac/mp4spatialenc/src/sac_enc.c
  mpegD_usac/mp4spatialenc/src/sac_huff_cld.c
  mpegD_usac/mp4spatialenc/src/sac_huff_cpc.c
  mpegD_usac/mp4spatialenc/src/sac_huff_icc.c
  mpegD_usac/mp4spatialenc/src/sac_huff_ipd.c
  mpegD_usac/mp4spatialenc/src/sac_huff_tab.h
  mpegD_usac/mp4spatialenc/src/sac_hybfilter.c
  mpegD_usac/mp4spatialenc/src/sac_hybfilter_enc.h
  mpegD_usac/mp4spatialenc/src/sac_ipd.c
  mpegD_usac/mp4spatialenc/src/sac_ipd.h
  mpegD_usac/mp4spatialenc/src/sac_nlc_enc.c
  mpegD_usac/mp4spatialenc/src/sac_nlc_enc.h
  mpegD_usac/mp4spatialenc/src/sac_polyphase.c
  mpegD_usac/mp4spatialenc/src/sac_polyphase_enc.h
  mpegD_usac/mp4spatialenc/src/sac_sbrconst_enc.h
  mpegD_usac/mp4spatialenc/src/sac_stream.c
  mpegD_usac/mp4spatialenc/src/sac_stream_enc.h
  mpegD_usac/mp4spatialenc/src/sac_types_enc.h
)
target_compile_definitions(spatialEncLib PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(spatialEncLib PUBLIC "${PROJECT_SOURCE_DIR}/mpegD_usac/mp4spatialenc/include"
                                         PUBLIC "${PROJECT_SOURCE_DIR}/mpegD_usac/mp4spatialenc/src")
set_target_properties(spatialEncLib PROPERTIES FOLDER "Libraries/coreCoder")

# 3DAudioCoreDeclib
add_library(3DAudioCoreDeclib
  mpegD_usac/usacEncDec/3DAudioCoreDeclib/include/3DAudioCoreDeclib.h
  mpegD_usac/usacEncDec/3DAudioCoreDeclib/src/3DAudioCoreDeclib.c
)
target_compile_definitions(3DAudioCoreDeclib PRIVATE CT_SBR ISOMP4 MP4FF PNS SSC_MP4REF_BUILD SONY_PVC _CRT_SECURE_NO_WARNINGS)
target_include_directories(3DAudioCoreDeclib PUBLIC "${PROJECT_SOURCE_DIR}/mpegD_usac/usacEncDec/3DAudioCoreDeclib/include")
target_link_libraries(3DAudioCoreDeclib aacDecLib usacDecLib streamfileLib qmflib_3da cicp2geometryLib libisomediafile)
set_target_properties(3DAudioCoreDeclib PROPERTIES FOLDER "Libraries/coreCoder")

# frameLib
add_library(frameLib
  mpegD_usac/usacEncDec/src_frame/allHandles.h
  mpegD_usac/usacEncDec/src_frame/audio.c
  mpegD_usac/usacEncDec/src_frame/audio.h
  mpegD_usac/usacEncDec/src_frame/austream.c
  mpegD_usac/usacEncDec/src_frame/austream.h
  mpegD_usac/usacEncDec/src_frame/bitstream.c
  mpegD_usac/usacEncDec/src_frame/bitstream.h
  mpegD_usac/usacEncDec/src_frame/cmdline.c
  mpegD_usac/usacEncDec/src_frame/cmdline.h
  mpegD_usac/usacEncDec/src_frame/common_m4a.c
  mpegD_usac/usacEncDec/src_frame/common_m4a.h
  # mpegD_usac/usacEncDec/src_frame/decifc.c
  # mpegD_usac/usacEncDec/src_frame/decifc.h
  mpegD_usac/usacEncDec/src_frame/fir_filt.c
  mpegD_usac/usacEncDec/src_frame/fir_filt.h
  mpegD_usac/usacEncDec/src_frame/flex_mux.c
  mpegD_usac/usacEncDec/src_frame/flex_mux.h
  mpegD_usac/usacEncDec/src_frame/mp4au.h
  # mpegD_usac/usacEncDec/src_frame/mp4ifc.c
  # mpegD_usac/usacEncDec/src_frame/mp4ifc.h
  mpegD_usac/usacEncDec/src_frame/obj_descr.h
  mpegD_usac/usacEncDec/src_frame/plotmtv.h
  mpegD_usac/usacEncDec/src_frame/spatial_bitstreamreader.c
  mpegD_usac/usacEncDec/src_frame/spatial_bitstreamreader.h
  # mpegD_usac/usacEncDec/src_frame/usacDec.c
  mpegD_usac/usacEncDec/src_frame/usac_channelconf.h
  mpegD_usac/usacEncDec/src_frame/usac_config.c
  mpegD_usac/usacEncDec/src_frame/usac_config.h
  mpegD_usac/usacEncDec/src_frame/vector_ops.c
  mpegD_usac/usacEncDec/src_frame/vector_ops.h
)
target_compile_definitions(frameLib PUBLIC CT_SBR USE_AFSP PNS CORRIGENDUM1 EXT2PAR I2R_LOSSLESS _CRT_SECURE_NO_WARNINGS)
target_include_directories(frameLib PUBLIC "${PROJECT_SOURCE_DIR}/mpegD_usac/usacEncDec/src_frame")
target_link_libraries(frameLib aacDecLib streamfileLib spatialDecLib wavIO_3da cicp2geometryLib libtsplite)
set_target_properties(frameLib PROPERTIES FOLDER "Libraries/coreCoder")

# icCodecLib
add_library(icCodecLib
  mpegD_usac/usacEncDec/src_ic/ic_defs.h
  mpegD_usac/usacEncDec/src_ic/ic_lib.c
  mpegD_usac/usacEncDec/src_ic/ic_rom.c
  mpegD_usac/usacEncDec/src_ic/ic_rom.h
  mpegD_usac/usacEncDec/src_ic/ic_spkr.c
)
target_compile_definitions(icCodecLib PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(icCodecLib PUBLIC "${PROJECT_SOURCE_DIR}/mpegD_usac/usacEncDec/src_ic")
set_target_properties(icCodecLib PROPERTIES FOLDER "Libraries/coreCoder")

# lpcDecLib
add_library(lpcDecLib
  mpegD_usac/usacEncDec/src_lpc/att_abs_postp.c
  mpegD_usac/usacEncDec/src_lpc/att_bwx.c
  mpegD_usac/usacEncDec/src_lpc/att_firfilt.c
  mpegD_usac/usacEncDec/src_lpc/att_header.h
  mpegD_usac/usacEncDec/src_lpc/att_iirfilt.c
  mpegD_usac/usacEncDec/src_lpc/att_lsf2pc.c
  mpegD_usac/usacEncDec/src_lpc/att_pc2lsf.c
  mpegD_usac/usacEncDec/src_lpc/att_proto.h
  mpegD_usac/usacEncDec/src_lpc/att_testbound.c
  mpegD_usac/usacEncDec/src_lpc/celp_bitstream_demux.c
  mpegD_usac/usacEncDec/src_lpc/celp_bitstream_demux.h
  mpegD_usac/usacEncDec/src_lpc/celp_decoder.c
  mpegD_usac/usacEncDec/src_lpc/celp_decoder.h
  mpegD_usac/usacEncDec/src_lpc/celp_proto_dec.h
  mpegD_usac/usacEncDec/src_lpc/dec_lpc.c
  mpegD_usac/usacEncDec/src_lpc/dec_lpc.h
  mpegD_usac/usacEncDec/src_lpc/err_concealment.h
  mpegD_usac/usacEncDec/src_lpc/err_state.h
  mpegD_usac/usacEncDec/src_lpc/fe_sub.h
  mpegD_usac/usacEncDec/src_lpc/lpc_common.h
  mpegD_usac/usacEncDec/src_lpc/nb_celp_dec.c
  mpegD_usac/usacEncDec/src_lpc/nec_abs_const.h
  mpegD_usac/usacEncDec/src_lpc/nec_abs_exc_generation.c
  mpegD_usac/usacEncDec/src_lpc/nec_abs_proto.h
  mpegD_usac/usacEncDec/src_lpc/nec_bws_acb_dec.c
  mpegD_usac/usacEncDec/src_lpc/nec_bws_exc_generation.c
  mpegD_usac/usacEncDec/src_lpc/nec_bws_gain_dec.c
  mpegD_usac/usacEncDec/src_lpc/nec_bws_module.c
  mpegD_usac/usacEncDec/src_lpc/nec_bws_mp_dec.c
  mpegD_usac/usacEncDec/src_lpc/nec_bws_qlsp_dec.c
  mpegD_usac/usacEncDec/src_lpc/nec_bws_qrms_dec.c
  mpegD_usac/usacEncDec/src_lpc/nec_cng_dec.c
  mpegD_usac/usacEncDec/src_lpc/nec_cng_lspvq.c
  mpegD_usac/usacEncDec/src_lpc/nec_cng_module.c
  mpegD_usac/usacEncDec/src_lpc/nec_enh_gain_dec.c
  mpegD_usac/usacEncDec/src_lpc/nec_enh_mp_dec.c
  mpegD_usac/usacEncDec/src_lpc/nec_exc_acb_dec.c
  mpegD_usac/usacEncDec/src_lpc/nec_exc_gain_dec.c
  mpegD_usac/usacEncDec/src_lpc/nec_exc_module.c
  mpegD_usac/usacEncDec/src_lpc/nec_exc_mp_config.c
  mpegD_usac/usacEncDec/src_lpc/nec_exc_mp_dec.c
  mpegD_usac/usacEncDec/src_lpc/nec_exc_proto.h
  mpegD_usac/usacEncDec/src_lpc/nec_qrms_dec.c
  mpegD_usac/usacEncDec/src_lpc/nec_vad.c
  mpegD_usac/usacEncDec/src_lpc/nec_vad.h
  mpegD_usac/usacEncDec/src_lpc/pan_celp_const.h
  mpegD_usac/usacEncDec/src_lpc/pan_celp_proto.h
  mpegD_usac/usacEncDec/src_lpc/pan_lspdec.c
  mpegD_usac/usacEncDec/src_lpc/pan_lspqtz2_dd.c
  mpegD_usac/usacEncDec/src_lpc/pan_lsp_intp.c
  mpegD_usac/usacEncDec/src_lpc/pan_mv_data.c
  mpegD_usac/usacEncDec/src_lpc/pan_sort.c
  mpegD_usac/usacEncDec/src_lpc/pan_stab.c
  mpegD_usac/usacEncDec/src_lpc/phi_cons.h
  mpegD_usac/usacEncDec/src_lpc/phi_excq.h
  mpegD_usac/usacEncDec/src_lpc/phi_fbit.c
  mpegD_usac/usacEncDec/src_lpc/phi_fbit.h
  mpegD_usac/usacEncDec/src_lpc/phi_gxit.c
  mpegD_usac/usacEncDec/src_lpc/phi_gxit.h
  mpegD_usac/usacEncDec/src_lpc/phi_lpc.c
  mpegD_usac/usacEncDec/src_lpc/phi_lpc.h
  mpegD_usac/usacEncDec/src_lpc/phi_lpcq.h
  mpegD_usac/usacEncDec/src_lpc/phi_lsfr.c
  mpegD_usac/usacEncDec/src_lpc/phi_lsfr.h
  mpegD_usac/usacEncDec/src_lpc/phi_post.c
  mpegD_usac/usacEncDec/src_lpc/phi_post.h
  mpegD_usac/usacEncDec/src_lpc/phi_priv.c
  mpegD_usac/usacEncDec/src_lpc/phi_priv.h
  mpegD_usac/usacEncDec/src_lpc/phi_xits.c
  mpegD_usac/usacEncDec/src_lpc/phi_xits.h
)
target_include_directories(lpcDecLib PUBLIC "${PROJECT_SOURCE_DIR}/mpegD_usac/usacEncDec/src_lpc")
target_link_libraries(lpcDecLib frameLib)
set_target_properties(lpcDecLib PROPERTIES FOLDER "Libraries/coreCoder")

# # lpcEncLib
# add_library(lpcEncLib
#   mpegD_usac/usacEncDec/src_lpc_enc/celp_bitstream_mux.h
#   mpegD_usac/usacEncDec/src_lpc_enc/celp_proto_enc.h
#   mpegD_usac/usacEncDec/src_lpc_enc/enc_lpc.h
#   mpegD_usac/usacEncDec/src_lpc_enc/phi_apre.h
#   mpegD_usac/usacEncDec/src_lpc_enc/phi_axit.h
#   mpegD_usac/usacEncDec/src_lpc_enc/phi_nec_lpc.h
#   mpegD_usac/usacEncDec/src_lpc_enc/phi_prep.h
# )
# target_include_directories(lpcEncLib PUBLIC "${PROJECT_SOURCE_DIR}/mpegD_usac/usacEncDec/src_lpc_enc")
# set_target_properties(lpcEncLib PROPERTIES FOLDER "Libraries/coreCoder")

# streamfileLib
add_library(streamfileLib
  mpegD_usac/usacEncDec/src_mp4/ep_convert.c
  mpegD_usac/usacEncDec/src_mp4/ep_convert.h
  mpegD_usac/usacEncDec/src_mp4/streamfile.c
  mpegD_usac/usacEncDec/src_mp4/streamfile.h
  mpegD_usac/usacEncDec/src_mp4/streamfile_diagnose.c
  mpegD_usac/usacEncDec/src_mp4/streamfile_diagnose.h
  mpegD_usac/usacEncDec/src_mp4/streamfile_fl4.c
  mpegD_usac/usacEncDec/src_mp4/streamfile_fl4.h
  mpegD_usac/usacEncDec/src_mp4/streamfile_helper.c
  mpegD_usac/usacEncDec/src_mp4/streamfile_helper.h
  mpegD_usac/usacEncDec/src_mp4/streamfile_latm.c
  mpegD_usac/usacEncDec/src_mp4/streamfile_latm.h
  mpegD_usac/usacEncDec/src_mp4/streamfile_mhas.c
  mpegD_usac/usacEncDec/src_mp4/streamfile_mhas.h
  mpegD_usac/usacEncDec/src_mp4/streamfile_mp4.c
  mpegD_usac/usacEncDec/src_mp4/streamfile_mp4.h
  mpegD_usac/usacEncDec/src_mp4/streamfile_raw.c
  mpegD_usac/usacEncDec/src_mp4/streamfile_raw.h
)
target_include_directories(streamfileLib PUBLIC "${PROJECT_SOURCE_DIR}/mpegD_usac/usacEncDec/src_mp4")
target_link_libraries(streamfileLib epToolLib frameLib libisomediafile mpegh_in_isom libtsplite)
set_target_properties(streamfileLib PROPERTIES FOLDER "Libraries/coreCoder")

# # mpeg12DecLib
# add_library(mpeg12DecLib
#   mpegD_usac/usacEncDec/src_mpeg12/dec_mpeg12.h
# )
# target_include_directories(mpeg12DecLib PUBLIC "${PROJECT_SOURCE_DIR}/mpegD_usac/usacEncDec/src_mpeg12")
# set_target_properties(mpeg12DecLib PROPERTIES FOLDER "Libraries/coreCoder")

# parDecLib
add_library(parDecLib
  mpegD_usac/usacEncDec/src_par/dec_hvxc.h
  mpegD_usac/usacEncDec/src_par/dec_par.c
  mpegD_usac/usacEncDec/src_par/dec_par.h
  mpegD_usac/usacEncDec/src_par/hvxc.h
  mpegD_usac/usacEncDec/src_par/hvxcCbAm.h
  mpegD_usac/usacEncDec/src_par/hvxcCbAm4k.h
  mpegD_usac/usacEncDec/src_par/hvxcCbCelp.h
  mpegD_usac/usacEncDec/src_par/hvxcCbCelp4k.h
  mpegD_usac/usacEncDec/src_par/hvxcCbLsp.h
  mpegD_usac/usacEncDec/src_par/hvxcCommon.c
  mpegD_usac/usacEncDec/src_par/hvxcCommon.h
  mpegD_usac/usacEncDec/src_par/hvxcDec.c
  mpegD_usac/usacEncDec/src_par/hvxcDec.h
  mpegD_usac/usacEncDec/src_par/hvxcFltDec.c
  mpegD_usac/usacEncDec/src_par/hvxcItpDec.c
  mpegD_usac/usacEncDec/src_par/hvxcProtect.c
  mpegD_usac/usacEncDec/src_par/hvxcProtect.h
  mpegD_usac/usacEncDec/src_par/hvxcQAmDec.c
  mpegD_usac/usacEncDec/src_par/hvxcQAmDec.h
  mpegD_usac/usacEncDec/src_par/hvxcQLspDec.c
  mpegD_usac/usacEncDec/src_par/hvxcSynUVDec.c
  mpegD_usac/usacEncDec/src_par/hvxcSynVDec.c
  mpegD_usac/usacEncDec/src_par/hvxcUVExtGenDec.c
  mpegD_usac/usacEncDec/src_par/hvxcVExtGenDec.c
  mpegD_usac/usacEncDec/src_par/hvxcVxcDec.c
  mpegD_usac/usacEncDec/src_par/hvxcVxcDec.h
  mpegD_usac/usacEncDec/src_par/hvxc_lpc_dmy.c
  mpegD_usac/usacEncDec/src_par/hvxc_struct.h
  mpegD_usac/usacEncDec/src_par/indilinecfg.h
  mpegD_usac/usacEncDec/src_par/indilinecom.c
  mpegD_usac/usacEncDec/src_par/indilinecom.h
  mpegD_usac/usacEncDec/src_par/indilinedec.c
  mpegD_usac/usacEncDec/src_par/indilinedec.h
  mpegD_usac/usacEncDec/src_par/indilineqnt.c
  mpegD_usac/usacEncDec/src_par/indilineqnt.h
  mpegD_usac/usacEncDec/src_par/indilinesyn.c
  mpegD_usac/usacEncDec/src_par/indilinesyn.h
  mpegD_usac/usacEncDec/src_par/mp4_par.h
  mpegD_usac/usacEncDec/src_par/pan_par_const.h
  mpegD_usac/usacEncDec/src_par/uhd_fft.c
  mpegD_usac/usacEncDec/src_par/uhd_fft.h
)
target_compile_definitions(parDecLib PRIVATE ISOMP4 MP4FF SSC_MP4REF_BUILD SONY_PVC)
target_include_directories(parDecLib PUBLIC "${PROJECT_SOURCE_DIR}/mpegD_usac/usacEncDec/src_par")
target_link_libraries(parDecLib aacDecLib frameLib)
set_target_properties(parDecLib PROPERTIES FOLDER "Libraries/coreCoder")

# # parEncLib
# add_library(parEncLib
  # mpegD_usac/usacEncDec/src_par_enc/enc_par.h
  # mpegD_usac/usacEncDec/src_par_enc/hvxcEnc.h
  # mpegD_usac/usacEncDec/src_par_enc/hvxcQAmEnc.h
  # mpegD_usac/usacEncDec/src_par_enc/indilineenc.h
  # mpegD_usac/usacEncDec/src_par_enc/indilinextr.h
# )
# target_include_directories(parEncLib PUBLIC "${PROJECT_SOURCE_DIR}/mpegD_usac/usacEncDec/src_par_enc")
# set_target_properties(parEncLib PROPERTIES FOLDER "Libraries/coreCoder")

# slsCodecLib
add_library(slsCodecLib
  mpegD_usac/usacEncDec/src_sls/aaz_enc_dec.h
  mpegD_usac/usacEncDec/src_sls/acod.c
  mpegD_usac/usacEncDec/src_sls/acod.h
  mpegD_usac/usacEncDec/src_sls/cbac.c
  mpegD_usac/usacEncDec/src_sls/cbac.h
  mpegD_usac/usacEncDec/src_sls/chandefs.h
  mpegD_usac/usacEncDec/src_sls/common.h
  mpegD_usac/usacEncDec/src_sls/decoder_sls.c
  mpegD_usac/usacEncDec/src_sls/int_compact_tables.c
  mpegD_usac/usacEncDec/src_sls/int_compact_tables.h
  mpegD_usac/usacEncDec/src_sls/int_kbd_win.h
  mpegD_usac/usacEncDec/src_sls/int_mdct.c
  mpegD_usac/usacEncDec/src_sls/int_mdct.h
  mpegD_usac/usacEncDec/src_sls/int_mdct_defs.h
  mpegD_usac/usacEncDec/src_sls/int_tns.c
  mpegD_usac/usacEncDec/src_sls/int_tns.h
  mpegD_usac/usacEncDec/src_sls/int_win_dctiv.c
  mpegD_usac/usacEncDec/src_sls/int_win_dctiv.h
  mpegD_usac/usacEncDec/src_sls/inv_int_mdct.c
  mpegD_usac/usacEncDec/src_sls/inv_int_mdct.h
  mpegD_usac/usacEncDec/src_sls/lit_ll_common.c
  mpegD_usac/usacEncDec/src_sls/lit_ll_common.h
  mpegD_usac/usacEncDec/src_sls/lit_ll_dec.c
  mpegD_usac/usacEncDec/src_sls/lit_ll_dec.h
  mpegD_usac/usacEncDec/src_sls/lit_ll_en.h
  mpegD_usac/usacEncDec/src_sls/lit_ms.c
  mpegD_usac/usacEncDec/src_sls/lit_ms.h
  mpegD_usac/usacEncDec/src_sls/ll_bitmux.h
  mpegD_usac/usacEncDec/src_sls/mc_enc.h
  mpegD_usac/usacEncDec/src_sls/sls.h
  mpegD_usac/usacEncDec/src_sls/sls_int.h
)
target_include_directories(slsCodecLib PUBLIC "${PROJECT_SOURCE_DIR}/mpegD_usac/usacEncDec/src_sls")
target_link_libraries(slsCodecLib aacDecLib aacEncLib frameLib)
set_target_properties(slsCodecLib PROPERTIES FOLDER "Libraries/coreCoder")

# tccDecLib
add_library(tccDecLib
  mpegD_usac/usacEncDec/src_tcc/tcc.c
  mpegD_usac/usacEncDec/src_tcc/tcc.h
  mpegD_usac/usacEncDec/src_tcc/tcc_decode_huff.c
  mpegD_usac/usacEncDec/src_tcc/tcc_decode_huff.h
  mpegD_usac/usacEncDec/src_tcc/tcc_decode_huff_tables.c
  mpegD_usac/usacEncDec/src_tcc/tcc_decode_huff_tables.h
  mpegD_usac/usacEncDec/src_tcc/tcc_mathutils.c
  mpegD_usac/usacEncDec/src_tcc/tcc_mathutils.h
  mpegD_usac/usacEncDec/src_tcc/tcc_struct.h
)
target_include_directories(tccDecLib PUBLIC "${PROJECT_SOURCE_DIR}/mpegD_usac/usacEncDec/src_tcc")
target_link_libraries(tccDecLib aacDecLib qmflib_3da)
set_target_properties(tccDecLib PROPERTIES FOLDER "Libraries/coreCoder")

# aacDecLib
add_library(aacDecLib
  mpegD_usac/usacEncDec/src_tf/aac.h
  mpegD_usac/usacEncDec/src_tf/adif.c
  mpegD_usac/usacEncDec/src_tf/adif.h
  mpegD_usac/usacEncDec/src_tf/all.h
  mpegD_usac/usacEncDec/src_tf/allVariables.h
  mpegD_usac/usacEncDec/src_tf/bitfct.c
  mpegD_usac/usacEncDec/src_tf/bitfct.h
  mpegD_usac/usacEncDec/src_tf/block.h
  mpegD_usac/usacEncDec/src_tf/buffer.h
  mpegD_usac/usacEncDec/src_tf/buffers.c
  mpegD_usac/usacEncDec/src_tf/buffers.h
  mpegD_usac/usacEncDec/src_tf/concealment.h
  mpegD_usac/usacEncDec/src_tf/concealment_dummy.c
  mpegD_usac/usacEncDec/src_tf/config.c
  mpegD_usac/usacEncDec/src_tf/coupling.c
  mpegD_usac/usacEncDec/src_tf/coupling.h
  mpegD_usac/usacEncDec/src_tf/ct_envcalc.c
  mpegD_usac/usacEncDec/src_tf/ct_envcalc.h
  mpegD_usac/usacEncDec/src_tf/ct_envdec.c
  mpegD_usac/usacEncDec/src_tf/ct_envdec.h
  mpegD_usac/usacEncDec/src_tf/ct_envextr.c
  mpegD_usac/usacEncDec/src_tf/ct_envextr.h
  mpegD_usac/usacEncDec/src_tf/ct_freqsca.c
  mpegD_usac/usacEncDec/src_tf/ct_freqsca.h
  mpegD_usac/usacEncDec/src_tf/ct_hfgen.c
  mpegD_usac/usacEncDec/src_tf/ct_hfgen.h
  mpegD_usac/usacEncDec/src_tf/ct_polyphase.c
  mpegD_usac/usacEncDec/src_tf/ct_polyphase.h
  mpegD_usac/usacEncDec/src_tf/ct_psdec.h
  mpegD_usac/usacEncDec/src_tf/ct_sbrbitb.c
  mpegD_usac/usacEncDec/src_tf/ct_sbrbitb.h
  mpegD_usac/usacEncDec/src_tf/ct_sbrconst.h
  mpegD_usac/usacEncDec/src_tf/ct_sbrcrc.c
  mpegD_usac/usacEncDec/src_tf/ct_sbrcrc.h
  mpegD_usac/usacEncDec/src_tf/ct_sbrdec.c
  mpegD_usac/usacEncDec/src_tf/ct_sbrdec.h
  mpegD_usac/usacEncDec/src_tf/ct_sbrdecoder.c
  mpegD_usac/usacEncDec/src_tf/ct_sbrdecoder.h
  mpegD_usac/usacEncDec/src_tf/decdata.c
  mpegD_usac/usacEncDec/src_tf/decoder_tf.c
  mpegD_usac/usacEncDec/src_tf/dec_tf.c
  mpegD_usac/usacEncDec/src_tf/dec_tf.h
  mpegD_usac/usacEncDec/src_tf/dolby_win.h
  mpegD_usac/usacEncDec/src_tf/dolby_win_ssr.h
  mpegD_usac/usacEncDec/src_tf/drc.h
  mpegD_usac/usacEncDec/src_tf/elements.h
  mpegD_usac/usacEncDec/src_tf/er_main_elements.h
  mpegD_usac/usacEncDec/src_tf/escapeScfResil.c
  mpegD_usac/usacEncDec/src_tf/escapeScfResil.h
  mpegD_usac/usacEncDec/src_tf/extension_type.h
  mpegD_usac/usacEncDec/src_tf/HFgen_preFlat.c
  mpegD_usac/usacEncDec/src_tf/HFgen_preFlat.h
  mpegD_usac/usacEncDec/src_tf/huffdec1.c
  mpegD_usac/usacEncDec/src_tf/huffdec2.c
  mpegD_usac/usacEncDec/src_tf/huffdec2.h
  mpegD_usac/usacEncDec/src_tf/huffdec3.c
  mpegD_usac/usacEncDec/src_tf/huffdec3.h
  mpegD_usac/usacEncDec/src_tf/huffinit.c
  mpegD_usac/usacEncDec/src_tf/huffScfResil.c
  mpegD_usac/usacEncDec/src_tf/huffScfResil.h
  mpegD_usac/usacEncDec/src_tf/hufftables.c
  mpegD_usac/usacEncDec/src_tf/imdct.c
  mpegD_usac/usacEncDec/src_tf/intensity.c
  mpegD_usac/usacEncDec/src_tf/interface.h
  mpegD_usac/usacEncDec/src_tf/intrins.c
  mpegD_usac/usacEncDec/src_tf/mat_scale_set_shift_para.c
  mpegD_usac/usacEncDec/src_tf/mat_scale_tf_requantize_spectrum.c
  mpegD_usac/usacEncDec/src_tf/mod_buf.c
  mpegD_usac/usacEncDec/src_tf/mod_buf.h
  mpegD_usac/usacEncDec/src_tf/monopred.c
  mpegD_usac/usacEncDec/src_tf/monopred.h
  mpegD_usac/usacEncDec/src_tf/monopredStruct.h
  mpegD_usac/usacEncDec/src_tf/mp4_tf.h
  mpegD_usac/usacEncDec/src_tf/nok_ltp_common.h
  mpegD_usac/usacEncDec/src_tf/nok_ltp_common_internal.h
  mpegD_usac/usacEncDec/src_tf/nok_lt_prediction.c
  mpegD_usac/usacEncDec/src_tf/nok_lt_prediction.h
  mpegD_usac/usacEncDec/src_tf/nok_prediction.h
  mpegD_usac/usacEncDec/src_tf/ntt_BitUnPack.c
  mpegD_usac/usacEncDec/src_tf/ntt_check.c
  mpegD_usac/usacEncDec/src_tf/ntt_cnst_chk.c
  mpegD_usac/usacEncDec/src_tf/ntt_conf.h
  mpegD_usac/usacEncDec/src_tf/ntt_dec_bark_env.c
  mpegD_usac/usacEncDec/src_tf/ntt_dec_gain.c
  mpegD_usac/usacEncDec/src_tf/ntt_dec_lpc_spectrum_inv.c
  mpegD_usac/usacEncDec/src_tf/ntt_dec_pgain.c
  mpegD_usac/usacEncDec/src_tf/ntt_dec_pitch.c
  mpegD_usac/usacEncDec/src_tf/ntt_dec_pit_seq.c
  mpegD_usac/usacEncDec/src_tf/ntt_dec_sq_gain.c
  mpegD_usac/usacEncDec/src_tf/ntt_denormalizer_spectrum.c
  mpegD_usac/usacEncDec/src_tf/ntt_extend_pitch.c
  mpegD_usac/usacEncDec/src_tf/ntt_fwex.c
  mpegD_usac/usacEncDec/src_tf/ntt_get_cdbk.c
  mpegD_usac/usacEncDec/src_tf/ntt_get_code.c
  mpegD_usac/usacEncDec/src_tf/ntt_headerdec.c
  mpegD_usac/usacEncDec/src_tf/ntt_lsptowt_int.c
  mpegD_usac/usacEncDec/src_tf/ntt_lsp_decw.c
  mpegD_usac/usacEncDec/src_tf/ntt_movdd.c
  mpegD_usac/usacEncDec/src_tf/ntt_mulawinv.c
  mpegD_usac/usacEncDec/src_tf/ntt_post_process.c
  mpegD_usac/usacEncDec/src_tf/ntt_redec.c
  mpegD_usac/usacEncDec/src_tf/ntt_relsp.h
  mpegD_usac/usacEncDec/src_tf/ntt_scale_conf.h
  mpegD_usac/usacEncDec/src_tf/ntt_scale_dec_bark_env.c
  mpegD_usac/usacEncDec/src_tf/ntt_scale_init.c
  mpegD_usac/usacEncDec/src_tf/ntt_scale_message.c
  mpegD_usac/usacEncDec/src_tf/ntt_scale_tf_proc_spectrum_d.c
  mpegD_usac/usacEncDec/src_tf/ntt_scale_vq_decoder.c
  mpegD_usac/usacEncDec/src_tf/ntt_SclBitUnPack.c
  mpegD_usac/usacEncDec/src_tf/ntt_set_isp.c
  mpegD_usac/usacEncDec/src_tf/ntt_TfInit.c
  mpegD_usac/usacEncDec/src_tf/ntt_tf_proc_spectrum_d.c
  mpegD_usac/usacEncDec/src_tf/ntt_tf_requantize_spectrum.c
  mpegD_usac/usacEncDec/src_tf/ntt_tvq.h
  mpegD_usac/usacEncDec/src_tf/ntt_vec_lenp.c
  mpegD_usac/usacEncDec/src_tf/ntt_vex_pn.c
  mpegD_usac/usacEncDec/src_tf/ntt_vq_decoder.c
  mpegD_usac/usacEncDec/src_tf/ntt_zerod.c
  mpegD_usac/usacEncDec/src_tf/pns.c
  mpegD_usac/usacEncDec/src_tf/port.h
  mpegD_usac/usacEncDec/src_tf/reorderspec.c
  mpegD_usac/usacEncDec/src_tf/reorderspec.h
  mpegD_usac/usacEncDec/src_tf/resilience.c
  mpegD_usac/usacEncDec/src_tf/resilience.h
  mpegD_usac/usacEncDec/src_tf/rvlcScfConceal.h
  mpegD_usac/usacEncDec/src_tf/rvlcScfConceal_dummy.c
  mpegD_usac/usacEncDec/src_tf/rvlcScfResil.c
  mpegD_usac/usacEncDec/src_tf/rvlcScfResil.h
  mpegD_usac/usacEncDec/src_tf/sam_bitbuffer.c
  mpegD_usac/usacEncDec/src_tf/sam_dec.h
  mpegD_usac/usacEncDec/src_tf/sam_decoder.c
  mpegD_usac/usacEncDec/src_tf/sam_decode_bsac.c
  mpegD_usac/usacEncDec/src_tf/sam_decode_frame.c
  mpegD_usac/usacEncDec/src_tf/sam_facoding.h
  mpegD_usac/usacEncDec/src_tf/sam_fadecode.c
  mpegD_usac/usacEncDec/src_tf/sam_model.h
  mpegD_usac/usacEncDec/src_tf/sam_tns.c
  mpegD_usac/usacEncDec/src_tf/sam_tns.h
  mpegD_usac/usacEncDec/src_tf/scal_dec.h
  mpegD_usac/usacEncDec/src_tf/scal_dec_frame.c
  mpegD_usac/usacEncDec/src_tf/sony_local.h
  mpegD_usac/usacEncDec/src_tf/sony_pvcdec.c
  mpegD_usac/usacEncDec/src_tf/sony_pvcdec.h
  mpegD_usac/usacEncDec/src_tf/sony_pvclib.c
  mpegD_usac/usacEncDec/src_tf/sony_pvclib.h
  mpegD_usac/usacEncDec/src_tf/sony_pvcparams.h
  mpegD_usac/usacEncDec/src_tf/sony_pvcprepro.h
  mpegD_usac/usacEncDec/src_tf/son_gc_common.c
  mpegD_usac/usacEncDec/src_tf/son_gc_compensate.c
  mpegD_usac/usacEncDec/src_tf/son_gc_etc.c
  mpegD_usac/usacEncDec/src_tf/son_gc_unpack.c
  mpegD_usac/usacEncDec/src_tf/son_ipqf.c
  mpegD_usac/usacEncDec/src_tf/son_pqf_common.c
  mpegD_usac/usacEncDec/src_tf/statistics.h
  mpegD_usac/usacEncDec/src_tf/statistics_aac.h
  mpegD_usac/usacEncDec/src_tf/statistics_aac_dummy.c
  mpegD_usac/usacEncDec/src_tf/statistics_dummy.c
  mpegD_usac/usacEncDec/src_tf/stdinc.h
  mpegD_usac/usacEncDec/src_tf/stereo.c
  mpegD_usac/usacEncDec/src_tf/tf_main.h
  mpegD_usac/usacEncDec/src_tf/tf_mainHandle.h
  mpegD_usac/usacEncDec/src_tf/tf_mainStruct.h
  mpegD_usac/usacEncDec/src_tf/tns.c
  mpegD_usac/usacEncDec/src_tf/tns.h
  mpegD_usac/usacEncDec/src_tf/tns2.c
  mpegD_usac/usacEncDec/src_tf/transfo.c
  mpegD_usac/usacEncDec/src_tf/transfo.h
  mpegD_usac/usacEncDec/src_tf/tvqAUDecode.c
  mpegD_usac/usacEncDec/src_tf/util.c
  mpegD_usac/usacEncDec/src_tf/util.h
  mpegD_usac/usacEncDec/src_tf/vmtypes.h
  mpegD_usac/usacEncDec/src_tf/win480LD.h
  mpegD_usac/usacEncDec/src_tf/win512LD.h
  mpegD_usac/usacEncDec/src_tf/win512LD2.h
)
target_compile_definitions(aacDecLib PRIVATE SONY_PVC)
target_include_directories(aacDecLib PUBLIC "${PROJECT_SOURCE_DIR}/mpegD_usac/usacEncDec/src_tf")
target_link_libraries(aacDecLib lpcDecLib usacDecLib frameLib slsCodecLib icCodecLib parDecLib ${ADD_MATH_LIB})
set_target_properties(aacDecLib PROPERTIES FOLDER "Libraries/coreCoder")

# aacEncLib
add_library(aacEncLib
  mpegD_usac/usacEncDec/src_frame_enc/encoder.h
  mpegD_usac/usacEncDec/src_tf_enc/aac_qc.c
  mpegD_usac/usacEncDec/src_tf_enc/aac_qc.h
  mpegD_usac/usacEncDec/src_tf_enc/aac_tools.c
  mpegD_usac/usacEncDec/src_tf_enc/aac_tools.h
  mpegD_usac/usacEncDec/src_tf_enc/bitmux.c
  mpegD_usac/usacEncDec/src_tf_enc/bitmux.h
  mpegD_usac/usacEncDec/src_tf_enc/ct_polyphase_enc.c
  mpegD_usac/usacEncDec/src_tf_enc/ct_polyphase_enc.h
  mpegD_usac/usacEncDec/src_tf_enc/ct_sbr.c
  mpegD_usac/usacEncDec/src_tf_enc/ct_sbr.h
  mpegD_usac/usacEncDec/src_tf_enc/enc_tf.c
  mpegD_usac/usacEncDec/src_tf_enc/enc_tf.h
  mpegD_usac/usacEncDec/src_tf_enc/ms.c
  mpegD_usac/usacEncDec/src_tf_enc/ms.h
  mpegD_usac/usacEncDec/src_tf_enc/nok_bwp_enc.c
  mpegD_usac/usacEncDec/src_tf_enc/nok_bwp_enc.h
  mpegD_usac/usacEncDec/src_tf_enc/nok_ltp_enc.c
  mpegD_usac/usacEncDec/src_tf_enc/nok_ltp_enc.h
  mpegD_usac/usacEncDec/src_tf_enc/nok_pitch.c
  mpegD_usac/usacEncDec/src_tf_enc/nok_pitch.h
  mpegD_usac/usacEncDec/src_tf_enc/ntt_BitPack.c
  mpegD_usac/usacEncDec/src_tf_enc/ntt_encode.h
  mpegD_usac/usacEncDec/src_tf_enc/ntt_enc_para.h
  mpegD_usac/usacEncDec/src_tf_enc/ntt_scale_coder.c
  mpegD_usac/usacEncDec/src_tf_enc/ntt_scale_encode.h
  mpegD_usac/usacEncDec/src_tf_enc/ntt_scale_enc_para.h
  mpegD_usac/usacEncDec/src_tf_enc/ntt_scale_vq_coder.c
  mpegD_usac/usacEncDec/src_tf_enc/ntt_tf_perceptual_model.c
  mpegD_usac/usacEncDec/src_tf_enc/ntt_tf_proc_spectrum.c
  mpegD_usac/usacEncDec/src_tf_enc/ntt_tf_quantize_spectrum.c
  mpegD_usac/usacEncDec/src_tf_enc/ntt_tools.c
  mpegD_usac/usacEncDec/src_tf_enc/ntt_tools.h
  mpegD_usac/usacEncDec/src_tf_enc/ntt_vq_coder.c
  mpegD_usac/usacEncDec/src_tf_enc/ntt_win_sw.c
  mpegD_usac/usacEncDec/src_tf_enc/ntt_win_sw.h
  mpegD_usac/usacEncDec/src_tf_enc/psych.c
  mpegD_usac/usacEncDec/src_tf_enc/psych.h
  mpegD_usac/usacEncDec/src_tf_enc/rm5hufbook.h
  mpegD_usac/usacEncDec/src_tf_enc/sam_bit_buffer.c
  mpegD_usac/usacEncDec/src_tf_enc/sam_encode.h
  mpegD_usac/usacEncDec/src_tf_enc/sam_encode_bsac.c
  mpegD_usac/usacEncDec/src_tf_enc/sam_encode_frame.c
  mpegD_usac/usacEncDec/src_tf_enc/sam_faencode.c
  mpegD_usac/usacEncDec/src_tf_enc/scal_enc_frame.c
  mpegD_usac/usacEncDec/src_tf_enc/scal_enc_frame.h
  mpegD_usac/usacEncDec/src_tf_enc/sony_pvcenc.c
  mpegD_usac/usacEncDec/src_tf_enc/sony_pvcenc.h
  mpegD_usac/usacEncDec/src_tf_enc/sony_pvcenc_prepro.h
  mpegD_usac/usacEncDec/src_tf_enc/son_gc_detecter.c
  mpegD_usac/usacEncDec/src_tf_enc/son_gc_detectreset.c
  mpegD_usac/usacEncDec/src_tf_enc/son_gc_etc_enc.c
  mpegD_usac/usacEncDec/src_tf_enc/son_gc_modifier.c
  mpegD_usac/usacEncDec/src_tf_enc/son_gc_pack.c
  mpegD_usac/usacEncDec/src_tf_enc/son_pqf.c
  mpegD_usac/usacEncDec/src_tf_enc/tns3.c
  mpegD_usac/usacEncDec/src_tf_enc/tns3.h
)
target_compile_definitions(aacEncLib PRIVATE SONY_PVC)
target_include_directories(aacEncLib PUBLIC "${PROJECT_SOURCE_DIR}/mpegD_usac/usacEncDec/src_tf_enc"
                                     PUBLIC "${PROJECT_SOURCE_DIR}/mpegD_usac/usacEncDec/src_frame_enc")
target_link_libraries(aacEncLib aacDecLib frameLib)
set_target_properties(aacEncLib PROPERTIES FOLDER "Libraries/coreCoder")

# usacDecLib
add_library(usacDecLib
  mpegD_usac/usacEncDec/src_usac/acelp_plus.h
  mpegD_usac/usacEncDec/src_usac/cnst.h
  mpegD_usac/usacEncDec/src_usac/decoder_usac.c
  mpegD_usac/usacEncDec/src_usac/decode_chan_ele.c
  mpegD_usac/usacEncDec/src_usac/dec_acelp.c
  mpegD_usac/usacEncDec/src_usac/dec_ace_fac.c
  mpegD_usac/usacEncDec/src_usac/dec_fd_fac.c
  mpegD_usac/usacEncDec/src_usac/dec_HREP.c
  mpegD_usac/usacEncDec/src_usac/dec_HREP.h
  mpegD_usac/usacEncDec/src_usac/dec_IGF.c
  mpegD_usac/usacEncDec/src_usac/dec_IGF.h
  mpegD_usac/usacEncDec/src_usac/dec_LPD.c
  mpegD_usac/usacEncDec/src_usac/dec_lpd_stereo.c
  mpegD_usac/usacEncDec/src_usac/dec_lpd_stereo.h
  mpegD_usac/usacEncDec/src_usac/dec_TBE.c
  mpegD_usac/usacEncDec/src_usac/dec_TBE.h
  mpegD_usac/usacEncDec/src_usac/dec_tcx_fac.c
  mpegD_usac/usacEncDec/src_usac/dec_usac.c
  mpegD_usac/usacEncDec/src_usac/dec_usac.h
  mpegD_usac/usacEncDec/src_usac/dlpc_avq.c
  mpegD_usac/usacEncDec/src_usac/d_gain2p.c
  mpegD_usac/usacEncDec/src_usac/enc_gain.c
  mpegD_usac/usacEncDec/src_usac/enc_rom.c
  mpegD_usac/usacEncDec/src_usac/enc_util.c
  mpegD_usac/usacEncDec/src_usac/get_gain.c
  mpegD_usac/usacEncDec/src_usac/harmsbr.h
  mpegD_usac/usacEncDec/src_usac/harmsbrMathlib.c
  mpegD_usac/usacEncDec/src_usac/harmsbrMathlib.h
  mpegD_usac/usacEncDec/src_usac/harmsbrPhasevoc.c
  mpegD_usac/usacEncDec/src_usac/harmsbrPhasevoc.h
  mpegD_usac/usacEncDec/src_usac/harmsbrQmftrans.c
  mpegD_usac/usacEncDec/src_usac/harmsbrQmftrans.h
  mpegD_usac/usacEncDec/src_usac/harmsbrWingen.c
  mpegD_usac/usacEncDec/src_usac/harmsbrWingen.h
  mpegD_usac/usacEncDec/src_usac/hp50.c
  mpegD_usac/usacEncDec/src_usac/int3gpp.h
  mpegD_usac/usacEncDec/src_usac/int_lpc.c
  mpegD_usac/usacEncDec/src_usac/LPD_fac.c
  mpegD_usac/usacEncDec/src_usac/lsf_tab.c
  mpegD_usac/usacEncDec/src_usac/mp4_usac.h
  mpegD_usac/usacEncDec/src_usac/options.h
  mpegD_usac/usacEncDec/src_usac/pit_fr4.c
  mpegD_usac/usacEncDec/src_usac/proto_func.h
  mpegD_usac/usacEncDec/src_usac/re8.h
  mpegD_usac/usacEncDec/src_usac/re8_cod.c
  mpegD_usac/usacEncDec/src_usac/re8_dec.c
  mpegD_usac/usacEncDec/src_usac/re8_dic.c
  mpegD_usac/usacEncDec/src_usac/re8_ppv.c
  mpegD_usac/usacEncDec/src_usac/re8_vor.c
  mpegD_usac/usacEncDec/src_usac/savq_dec.c
  mpegD_usac/usacEncDec/src_usac/spatial_bitstreamreader_buffer.c
  mpegD_usac/usacEncDec/src_usac/spatial_bitstreamreader_buffer.h
  mpegD_usac/usacEncDec/src_usac/tables_plus.c
  mpegD_usac/usacEncDec/src_usac/table_decl.h
  mpegD_usac/usacEncDec/src_usac/td_frame.h
  mpegD_usac/usacEncDec/src_usac/typedef.h
  mpegD_usac/usacEncDec/src_usac/usac.h
  mpegD_usac/usacEncDec/src_usac/usac_all.h
  mpegD_usac/usacEncDec/src_usac/usac_allVariables.h
  mpegD_usac/usacEncDec/src_usac/usac_arith_dec.c
  mpegD_usac/usacEncDec/src_usac/usac_arith_dec.h
  mpegD_usac/usacEncDec/src_usac/usac_arith_tables.h
  mpegD_usac/usacEncDec/src_usac_enc/usac_bits.c
  mpegD_usac/usacEncDec/src_usac/usac_calcWindow.c
  mpegD_usac/usacEncDec/src_usac/usac_calcWindow.h
  mpegD_usac/usacEncDec/src_usac/usac_cfftn.c
  mpegD_usac/usacEncDec/src_usac/usac_cplx_pred.c
  mpegD_usac/usacEncDec/src_usac/usac_cplx_pred.h
  mpegD_usac/usacEncDec/src_usac/usac_decdata.c
  mpegD_usac/usacEncDec/src_usac/usac_elements.h
  mpegD_usac/usacEncDec/src_usac/usac_enc_lpc.c
  mpegD_usac/usacEncDec/src_usac/usac_fd_dec.c
  mpegD_usac/usacEncDec/src_usac/usac_imdct.c
  mpegD_usac/usacEncDec/src_usac/usac_interface.h
  mpegD_usac/usacEncDec/src_usac/usac_ltpf.c
  mpegD_usac/usacEncDec/src_usac/usac_ltpf.h
  mpegD_usac/usacEncDec/src_usac/usac_main.h
  mpegD_usac/usacEncDec/src_usac/usac_mainStruct.h
  mpegD_usac/usacEncDec/src_usac/usac_multichannel.c
  mpegD_usac/usacEncDec/src_usac/usac_multichannel.h
  mpegD_usac/usacEncDec/src_usac/usac_port.h
  mpegD_usac/usacEncDec/src_usac/usac_tcx_mdct.c
  mpegD_usac/usacEncDec/src_usac/usac_tcx_mdct.h
  mpegD_usac/usacEncDec/src_usac/usac_td_dec.c
  mpegD_usac/usacEncDec/src_usac/usac_td_resampler.c
  mpegD_usac/usacEncDec/src_usac/usac_td_resampler.h
  mpegD_usac/usacEncDec/src_usac/usac_tns.c
  mpegD_usac/usacEncDec/src_usac/usac_toolsdec3.c
  mpegD_usac/usacEncDec/src_usac/usac_toolsinit.c
  mpegD_usac/usacEncDec/src_usac/usac_transfo.c
  mpegD_usac/usacEncDec/src_usac/usac_transfo.h
  mpegD_usac/usacEncDec/src_usac/usac_tw_defines.h
  mpegD_usac/usacEncDec/src_usac/usac_tw_tools.c
  mpegD_usac/usacEncDec/src_usac/usac_tw_tools.h
  mpegD_usac/usacEncDec/src_usac/usac_windows.c
  mpegD_usac/usacEncDec/src_usac/util_td.c
  mpegD_usac/usacEncDec/src_usac/vlpc_1st_dec.c
  mpegD_usac/usacEncDec/src_usac/vlpc_1st_dic.c
  mpegD_usac/usacEncDec/src_usac/vlpc_2st_dec.c
)
target_compile_definitions(usacDecLib PRIVATE SONY_PVC)
target_include_directories(usacDecLib PUBLIC "${PROJECT_SOURCE_DIR}/mpegD_usac/usacEncDec/src_usac")
target_link_libraries(usacDecLib aacDecLib spatialDecLib tccDecLib frameLib)
set_target_properties(usacDecLib PROPERTIES FOLDER "Libraries/coreCoder")

# usacEncLib
add_library(usacEncLib
  mpegD_usac/usacEncDec/src_usac_enc/cod_ace_fac.c
  mpegD_usac/usacEncDec/src_usac_enc/cod_lf_fac.c
  mpegD_usac/usacEncDec/src_usac_enc/cod_main.c
  mpegD_usac/usacEncDec/src_usac_enc/cod_tcx_fac.c
  mpegD_usac/usacEncDec/src_usac_enc/cplx_pred.c
  mpegD_usac/usacEncDec/src_usac_enc/cplx_pred.h
  mpegD_usac/usacEncDec/src_usac_enc/enc_acelp.c
  mpegD_usac/usacEncDec/src_usac_enc/enc_IGF.c
  mpegD_usac/usacEncDec/src_usac_enc/enc_IGF.h
  mpegD_usac/usacEncDec/src_usac_enc/enc_prm.c
  mpegD_usac/usacEncDec/src_usac_enc/enc_usac.c
  mpegD_usac/usacEncDec/src_usac_enc/enc_usac.h
  mpegD_usac/usacEncDec/src_usac_enc/fd_fac.c
  mpegD_usac/usacEncDec/src_usac_enc/int_dec.c
  mpegD_usac/usacEncDec/src_usac_enc/int_dec.h
  mpegD_usac/usacEncDec/src_usac_enc/lag_wind.c
  mpegD_usac/usacEncDec/src_usac_enc/lsf_wt.c
  mpegD_usac/usacEncDec/src_usac_enc/mct_enc.c
  mpegD_usac/usacEncDec/src_usac_enc/mct_enc.h
  mpegD_usac/usacEncDec/src_usac_enc/qlpc_avq.c
  mpegD_usac/usacEncDec/src_usac_enc/q_gain2p.c
  mpegD_usac/usacEncDec/src_usac_enc/savq_cod.c
  mpegD_usac/usacEncDec/src_usac_enc/segsnr.c
  mpegD_usac/usacEncDec/src_usac_enc/signal_classifier.c
  mpegD_usac/usacEncDec/src_usac_enc/signal_classifier.h
  mpegD_usac/usacEncDec/src_usac_enc/usac_bitmux.c
  mpegD_usac/usacEncDec/src_usac_enc/usac_bitmux.h
  mpegD_usac/usacEncDec/src_usac_enc/usac_fd_enc.c
  mpegD_usac/usacEncDec/src_usac_enc/usac_fd_enc.h
  mpegD_usac/usacEncDec/src_usac_enc/usac_fd_qc.c
  mpegD_usac/usacEncDec/src_usac_enc/usac_fd_qc.h
  mpegD_usac/usacEncDec/src_usac_enc/usac_mdct.c
  mpegD_usac/usacEncDec/src_usac_enc/usac_mdct.h
  mpegD_usac/usacEncDec/src_usac_enc/usac_nf.c
  mpegD_usac/usacEncDec/src_usac_enc/usac_nf.h
  mpegD_usac/usacEncDec/src_usac_enc/usac_tw_enc.c
  mpegD_usac/usacEncDec/src_usac_enc/usac_tw_enc.h
  mpegD_usac/usacEncDec/src_usac_enc/vlpc_1st_cod.c
  mpegD_usac/usacEncDec/src_usac_enc/vlpc_2st_cod.c
  mpegD_usac/usacEncDec/src_usac_enc/window.c
)
target_compile_definitions(usacEncLib PRIVATE SONY_PVC)
target_include_directories(usacEncLib PUBLIC "${PROJECT_SOURCE_DIR}/mpegD_usac/usacEncDec/src_usac_enc")
target_link_libraries(usacEncLib usacDecLib aacDecLib spatialEncLib frameLib)
set_target_properties(usacEncLib PROPERTIES FOLDER "Libraries/coreCoder")

###############################################################################
# target sources (executables)
###############################################################################
# 3DAudioCoreDecoder
add_executable(3DAudioCoreDecoder
  mpegD_usac/usacEncDec/3DAudioCoreDecoder/src/3DAudioCoreDecoder.c
)
target_compile_definitions(3DAudioCoreDecoder PRIVATE _CRT_SECURE_NO_WARNINGS)
set_target_properties(3DAudioCoreDecoder PROPERTIES FOLDER "Executables/3DAudioCoreCoder")
target_link_libraries(3DAudioCoreDecoder 3DAudioCoreDeclib wavIO_3da)
install(TARGETS 3DAudioCoreDecoder DESTINATION bin)

# 3DAudioCoreEncoder
add_executable(usacEnc
  mpegD_usac/usacEncDec/src_frame_enc/usacEnc.c
)
target_link_libraries(usacEnc frameLib usacEncLib usacDecLib aacEncLib streamfileLib spatialEncLib spatialDecLib libisomediafile mpegh_in_isom cicp2geometryLib libtsplite)
set_target_properties(usacEnc PROPERTIES FOLDER "Executables/3DAudioCoreCoder")
install(TARGETS usacEnc DESTINATION bin)
