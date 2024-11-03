/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        xxx.c
 * Description:  
 *
 * $Date:        15 February 2023
 * $Revision:    V0.0.1
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2023 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 */

#ifdef __cplusplus
 extern "C" {
#endif


#ifndef cSTREAM_TOOL_DEFINE_H
#define cSTREAM_TOOL_DEFINE_H

#define MAX_NB_NODES 100
#define MAX_NB_FORMAT 32

#define MAXINPUT 100000
#define MAXOUTPUT 100000
#define MAXBINARYGRAPHW32 10000
#define MAXDBGTRACEGRAPH 10000

#define SUBGMAXDEPTH 5          /* subgraphs calling subgraphs */
#define MAXCHAR_NAME 80
#define MAXNBARCH 8
#define MAXNBLIBRARY 16
#define NOT_YET_END_OF_FILE 1
#define FOUND_END_OF_FILE 2
#define MAX_NB_PATH 32          /* number of file paths in the graph */
#define MAX_NB_IO 128           /* number of IO streams of the graph */

#define NBCHAR_LINE 200
#define NBCHAR_NAME 120
#define NBCHAR_STREAM_NAME 44   /* name of the HW IO streams and graph interfaces */

#define MAXPARAMETERSIZE ((int64_t)1e4)  /* in word32 */

#define MAX_NBOPTIONS_SETS 8
#define MAX_NBOPTIONS 256 

#define MAX_PROC_MEMBANK 63     /* number of physical memory banks of the processor, for the graph processing */

#define MAXNB_WORD32_BOOT_KEY 32    /* 1024 bits boot key */
#define MAXNB_WORD32_USER_KEY 32    /* 1024 bits user key */

#define VID_default 0

#define MAX_POSSIBLE_NB_MEMORY_OFFSET 8 /* long_offset[] size */

#define SECTION_END "end"
/*----------------------------------------------------------------------- IO MANIFESTS ------------- */
#define io_commander0_servant1      "io_commander0_servant1"
#define io_buffer_allocation        "io_buffer_allocation"
#define io_direction_rx0tx1         "io_direction_rx0tx1"
#define io_raw_format               "io_raw_format"
#define io_nb_channels              "io_nb_channels"
#define io_interleaving             "io_interleaving"
#define io_frame_length             "io_frame_length"
#define io_frame_duration           "io_frame_duration"
#define io_units_rescale            "io_units_rescale"
#define io_units_rescale_multiple   "io_units_rescale_multiple"
#define io_power_mode               "io_power_mode"
#define io_position                 "io_position"
#define io_euler_angles             "io_euler_angles"
#define io_sampling_rate            "io_sampling_rate"
#define io_sampling_period_s        "io_sampling_period_s"
#define io_sampling_period_day      "io_sampling_period_day"
#define io_sampling_rate_accuracy   "io_sampling_rate_accuracy"
#define io_time_stamp_format        "io_time_stamp_format"

// ---- general             
// ---- audio_in            
// ---- audio_out       
#define io_subtype_units            "io_subtype_units"
#define io_analog_scale             "io_analog_scale"
#define io_rescale_factor           "io_rescale_factor"
#define io_sampling_rate            "io_sampling_rate"
#define io_sampling_period_s        "io_sampling_period_s"
#define io_sampling_period_day      "io_sampling_period_day"
#define io_nb_channels              "io_nb_channels"
#define io_channel_mapping          "io_channel_mapping"
#define io_analog_gain              "io_analog_gain"
#define io_digital_gain             "io_digital_gain"
#define io_hp_filter                "io_hp_filter"
#define io_agc                      "io_agc"
#define io_router                   "io_router"
#define io_gbass_filter             "io_gbass_filter"
#define io_fbass_filter             "io_fbass_filter"
#define io_gmid_filter              "io_gmid_filter"
#define io_fmid_filter              "io_fmid_filter"
#define io_ghigh_filter             "io_ghigh_filter"
#define io_fhigh_filter             "io_fhigh_filter"

// ---- gpio_in             
// ---- gpio_out            
// ---- motion              
#define io_sampling_rate            "io_sampling_rate"
#define io_sampling_period_s        "io_sampling_period_s"
#define io_motion_format            "io_motion_format"
#define io_motion_sensitivity       "io_motion_sensitivity"
#define io_motion_averaging         "io_motion_averaging"
#define io_high_pass                "io_high_pass" 
#define io_DC_canceller             "io_DC_canceller"

// ---- 2d_in               
// ---- 2d_out         
#define io_raw_format_2d               "io_raw_format_2d"
#define io_trigger_flash               "io_trigger_flash"
#define io_synchronize_with            "io_synchronize_with"
#define io_frame_rate_per_s            "io_frame_rate_per_s"
#define io_exposure_time               "io_exposure_time"
#define io_image_size                  "io_image_size"
#define io_modes                       "io_modes"
#define io_gain	                       "io_gain"
#define io_whitebalancecolortemp	   "io_whitebalancecolortemp"
#define io_whitebalancecolortint	   "io_whitebalancecolortint"
#define io_mosaicpattern	           "io_mosaicpattern"
#define io_whitebalancergbcoefficients "io_whitebalancergbcoefficients"  
#define io_enablewhitebalancergbcoef   "io_enablewhitebalancergbcoef"
#define io_autowhite_balance	       "io_autowhite_balance"
#define io_time_stamp                  "io_time_stamp"
#define io_wdr                         "io_wdr"
#define io_watermark                   "io_watermark"
#define io_flip                        "io_flip"
#define io_night_mode                  "io_night_mode"
#define io_detection_zones             "io_detection_zones"
#define io_focus_area                  "io_focus_area"
#define io_auto_exposure_on_focus      "io_auto_exposure_on_focus"
#define io_focus_distance              "io_focus_distance"
#define io_get_distance                "io_get_distance"
#define io_zoom_area                   "io_zoom_area"
#define io_time_stamp                  "io_time_stamp"
#define io_light_detection             "io_light_detection"
#define io_jpeg_quality                "io_jpeg_quality"
#define io_sound_detection             "io_sound_detection"
#define io_backlight_brightness        "io_backlight_brightness"

/*----------------------------------------------------------------------- NODE MANIFESTS ------------- */
#define node_developer_name             "node_developer_name"
#define node_name                       "node_name"
#define node_logo                       "node_logo"
#define node_nb_arcs                    "node_nb_arcs"
#define node_arc_parameter              "node_arc_parameter"
#define node_variable_rxtx_data_rate    "node_variable_rxtx_data_rate"
#define node_use_mpdtcm                 "node_use_mpdtcm"
#define node_using_arc_format           "node_using_arc_format"
#define node_mask_library               "node_mask_library"
#define node_subtype_units              "node_subtype_units"
#define node_architecture               "node_architecture"
#define node_fpu_used                   "node_fpu_used"
#define node_use_boot_key               "node_use_boot_key"
#define node_node_version               "node_node_version"
#define node_stream_version             "node_stream_version"
#define node_mem                        "node_mem "
#define node_mem_alloc                  "node_mem_alloc"
#define node_mem_nbchan                 "node_mem_nbchan"
#define node_mem_sampling_rate          "node_mem_sampling_rate"
#define node_mem_frame_size             "node_mem_frame_size"
#define node_mem_alignement             "node_mem_alignement"
#define node_mem_type                   "node_mem_type"
#define node_mem_speed                  "node_mem_speed"
#define node_mem_relocatable            "node_mem_relocatable"
#define node_mem_data0prog1             "node_mem_data0prog1"
#define node_not_reentrant              "node_not_reentrant"
#define node_new_arc                    "node_arc "
#define node_arc_rx0tx1                 "node_arc_rx0tx1"
#define node_arc_sampling_rate          "node_arc_sampling_rate"
#define node_arc_nb_channels            "node_arc_nb_channels"
#define node_arc_interleaving           "node_arc_interleaving"
#define node_arc_raw_format             "node_arc_raw_format"
#define node_arc_frame_length           "node_arc_frame_length"
#define node_arc_frame_duration         "node_arc_frame_duration"
#define node_arc_sampling_rate          "node_arc_sampling_rate"
#define node_arc_sampling_period_s      "node_arc_sampling_period_s"
#define node_arc_sampling_period_day    "node_arc_sampling_period_day"
#define node_arc_sampling_accuracy      "node_arc_sampling_accuracy"
#define node_arc_inPlaceProcessing      "node_arc_inPlaceProcessing"

/*----------------------------------------------------------------------- GRAPH HEADER ------------- */
#define subgraph                        "subgraph"
#define set_file_path                   "set_file_path"
#define graph_location                  "graph_location"
#define graph_location_offset           "graph_location_offset"
#define graph_memory_bank               "graph_memory_bank"
#define debug_script_fields             "debug_script_fields"
#define scheduler_return                "scheduler_return"
#define allowed_processors              "allowed_processors"
#define include                         "include"
#define memory_mapping                  "memory_mapping"
#define top_trace_node                  "top_trace_node"
#define stream_io_new                   "stream_io"
#define stream_io_format                "stream_io_format"
#define stream_io_hwid                  "stream_io_hwid"
#define stream_io_setting               "stream_io_setting"
#define stream_io_setting_callback      "stream_io_setting_callback"

#define mem_fill_pattern                "mem_fill_pattern"

/*----------------------------------------------------------------------- GRAPH FORMAT ------------- */
#define format_index                    "format_index"
#define format_raw_data                 "format_raw_data"
#define format_frame_length             "format_frame_length"
#define format_interleaving             "format_interleaving"
#define format_nbchan                   "format_nbchan"
#define format_time_stamp               "format_time_stamp"
#define format_sdomain                  "format_sdomain"
#define format_domain                   "format_domain"
#define format_sampling_rate            "format_sampling_rate"
#define format_audio_mapping            "format_audio_mapping"
#define format_motion_mapping           "format_motion_mapping"
#define format_2d_height                "format_2d_height"
#define format_2d_width                 "format_2d_width"
#define format_2d_border                "format_2d_border"

/*----------------------------------------------------------------------- GRAPH NODES ------------- */
#define node_new                        "node "
#define node_preset                     "node_preset"
#define node_map_hwblock                "node_map_hwblock"
#define node_malloc_add                 "node_malloc_add"
#define node_map_copy                   "node_map_copy"
#define node_map_swap                   "node_map_swap"
#define node_trace_id                   "node_trace_id"
#define node_map_proc                   "node_map_proc"
#define node_map_arch                   "node_map_arch"
#define node_map_rtos                   "node_map_rtos"
#define node_map_verbose                "node_map_verbose"
#define node_memory_isolation           "node_memory_isolation"
#define node_memory_clear               "node_memory_clear"
#define node_parameters                 "node_parameters"
#define node_script_code                "node_script_code"

/*----------------------------------------------------------------------- GRAPH SCRIPTS ------------- */
#define common_script                   "script "
#define script_register                 "script_register"
#define script_stack                    "script_stack"
#define script_parameter                "script_parameter"
#define script_mem_shared               "script_mem_shared"
#define script_mem_map                  "script_mem_map"
#define script_code                     "script_code"
#define script_assembler                "script_assembler"
#define script_parameters               "script_parameters"
#define script_heap                     "script_heap"

/*----------------------------------------------------------------------- GRAPH ARCS ------------- */
#define arc_input                       "arc_input"
#define arc_output                      "arc_output"
#define arc_new                         "arc"
#define arc_flow_error                  "arc_flow_error"
#define arc_debug_cmd                   "arc_debug_cmd"
#define arc_debug_reg                   "arc_debug_reg"
#define arc_debug_page                  "arc_debug_page"
#define arc_flush                       "arc_flush"
#define arc_extend_address              "arc_extend_address"
#define arc_map_hwblock                 "arc_map_hwblock"
#define arc_jitter_ctrl                 "arc_jitter_ctrl"

#endif /* #ifndef cSTREAM_TOOL_DEFINE_H */
#ifdef __cplusplus
}
#endif
