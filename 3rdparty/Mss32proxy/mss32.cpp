#include <Windows.h>

extern "C"
{
__declspec(dllexport, naked) void __stdcall AIL_debug_printf			 () { }
__declspec(dllexport, naked) void __stdcall AIL_sprintf					 () { }
__declspec(dllexport, naked) void __stdcall DLSClose					 () { }
__declspec(dllexport, naked) void __stdcall DLSCompactMemory			 () { }
__declspec(dllexport, naked) void __stdcall DLSGetInfo					 () { }
__declspec(dllexport, naked) void __stdcall DLSLoadFile					 () { }
__declspec(dllexport, naked) void __stdcall DLSLoadMemFile				 () { }
__declspec(dllexport, naked) void __stdcall DLSMSSOpen					 () { }
__declspec(dllexport, naked) void __stdcall DLSSetAttribute				 () { }
__declspec(dllexport, naked) void __stdcall DLSUnloadAll				 () { }
__declspec(dllexport, naked) void __stdcall DLSUnloadFile				 () { }
__declspec(dllexport, naked) void __stdcall RIB_alloc_provider_handle	 () { }
__declspec(dllexport, naked) void __stdcall RIB_enumerate_interface		 () { }
__declspec(dllexport, naked) void __stdcall RIB_error					 () { }
__declspec(dllexport, naked) void __stdcall RIB_find_file_provider		 () { }
__declspec(dllexport, naked) void __stdcall RIB_free_provider_handle	 () { }
__declspec(dllexport, naked) void __stdcall RIB_free_provider_library	 () { }
__declspec(dllexport, naked) void __stdcall RIB_load_provider_library	 () { }
__declspec(dllexport, naked) void __stdcall RIB_register_interface		 () { }
__declspec(dllexport, naked) void __stdcall RIB_request_interface		 () { }
__declspec(dllexport, naked) void __stdcall RIB_request_interface_entry	 () { }
__declspec(dllexport, naked) void __stdcall RIB_type_string				 () { }
__declspec(dllexport, naked) void __stdcall RIB_unregister_interface     () { }
__declspec(dllexport, naked) void __stdcall _AIL_3D_auto_update_position(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_3D_orientation(long arg1, long arg2, long arg3, long arg4, long arg5, long arg6, long arg7) { }
__declspec(dllexport, naked) void __stdcall _AIL_3D_position(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall _AIL_3D_provider_attribute(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_3D_room_type(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_3D_sample_attribute(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_3D_sample_cone(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall _AIL_3D_sample_distances(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_3D_sample_effects_level(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_3D_sample_length(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_3D_sample_loop_count(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_3D_sample_obstruction(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_3D_sample_occlusion(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_3D_sample_offset(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_3D_sample_playback_rate(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_3D_sample_status(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_3D_sample_volume(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_3D_speaker_type(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_3D_update_position(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_3D_user_data(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_3D_velocity(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall _AIL_DLS_close(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_DLS_compact(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_DLS_get_info(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_DLS_get_reverb(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall _AIL_DLS_load_file(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_DLS_load_memory(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_DLS_open(long arg1, long arg2, long arg3, long arg4, long arg5, long arg6, long arg7) { }
__declspec(dllexport, naked) void __stdcall _AIL_DLS_set_reverb(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall _AIL_DLS_unload(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_HWND() { }
__declspec(dllexport, naked) void __stdcall _AIL_MIDI_handle_reacquire(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_MIDI_handle_release(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_MIDI_to_XMI(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall _AIL_MMX_available() { }
__declspec(dllexport, naked) void __stdcall _AIL_WAV_file_write(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall _AIL_WAV_info(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_XMIDI_master_volume(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_active_3D_sample_count(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_active_sample_count(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_active_sequence_count(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_allocate_3D_sample_handle(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_allocate_file_sample(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_allocate_sample_handle(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_allocate_sequence_handle(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_auto_service_stream(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_background() { }
__declspec(dllexport, naked) void __stdcall _AIL_branch_index(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_channel_notes(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_close_3D_listener(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_close_3D_object(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_close_3D_provider(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_close_filter(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_close_input(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_close_stream(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_compress_ADPCM(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_compress_ASI(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall _AIL_compress_DLS(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall _AIL_controller_value(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_create_wave_synthesizer(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall _AIL_decompress_ADPCM(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_decompress_ASI(long arg1, long arg2, long arg3, long arg4, long arg5, long arg6) { }
__declspec(dllexport, naked) void __stdcall _AIL_delay(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_destroy_wave_synthesizer(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_digital_CPU_percent(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_digital_configuration(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall _AIL_digital_handle_reacquire(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_digital_handle_release(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_digital_latency(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_digital_master_volume(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_end_3D_sample(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_end_sample(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_end_sequence(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_enumerate_3D_provider_attributes(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_enumerate_3D_providers(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_enumerate_3D_sample_attributes(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_enumerate_filter_attributes(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_enumerate_filter_sample_attributes(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_enumerate_filters(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_extract_DLS(long arg1, long arg2, long arg3, long arg4, long arg5, long arg6, long arg7) { }
__declspec(dllexport, naked) void __stdcall _AIL_file_error() { }
__declspec(dllexport, naked) void __stdcall _AIL_file_read(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_file_size(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_file_type(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_file_write(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_filter_DLS_attribute(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_filter_DLS_with_XMI(long arg1, long arg2, long arg3, long arg4, long arg5, long arg6) { }
__declspec(dllexport, naked) void __stdcall _AIL_filter_attribute(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_filter_sample_attribute(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_filter_stream_attribute(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_find_DLS(long arg1, long arg2, long arg3, long arg4, long arg5, long arg6) { }
__declspec(dllexport, naked) void __stdcall _AIL_get_DirectSound_info(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_get_preference(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_get_timer_highest_delay() { }
__declspec(dllexport, naked) void __stdcall _AIL_init_sample(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_init_sequence(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_input_info(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_last_error() { }
__declspec(dllexport, naked) void __stdcall _AIL_list_DLS(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall _AIL_list_MIDI(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall _AIL_load_sample_buffer(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall _AIL_lock() { }
__declspec(dllexport, naked) void __stdcall _AIL_lock_channel(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_lock_mutex() { }
__declspec(dllexport, naked) void __stdcall _AIL_map_sequence_channel(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_mem_alloc_lock(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_mem_free_lock(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_merge_DLS_with_XMI(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall _AIL_midiOutClose(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_midiOutOpen(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_minimum_sample_buffer_size(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_ms_count() { }
__declspec(dllexport, naked) void __stdcall _AIL_open_3D_listener(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_open_3D_object(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_open_3D_provider(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_open_filter(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_open_input(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_open_stream(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_pause_stream(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_primary_digital_driver(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_process_digital_audio(long arg1, long arg2, long arg3, long arg4, long arg5, long arg6) { }
__declspec(dllexport, naked) void __stdcall _AIL_quick_copy(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_quick_halt(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_quick_handles(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_quick_load(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_quick_load_and_play(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_quick_load_mem(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_quick_ms_length(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_quick_ms_position(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_quick_play(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_quick_set_ms_position(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_quick_set_reverb(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall _AIL_quick_set_speed(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_quick_set_volume(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_quick_shutdown() { }
__declspec(dllexport, naked) void __stdcall _AIL_quick_startup(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall _AIL_quick_status(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_quick_type(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_quick_unload(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_redbook_close(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_redbook_eject(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_redbook_id(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_redbook_open(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_redbook_open_drive(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_redbook_pause(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_redbook_play(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_redbook_position(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_redbook_resume(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_redbook_retract(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_redbook_set_volume(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_redbook_status(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_redbook_stop(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_redbook_track(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_redbook_track_info(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall _AIL_redbook_tracks(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_redbook_volume(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_register_3D_EOS_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_register_EOB_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_register_EOF_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_register_EOS_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_register_ICA_array(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_register_SOB_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_register_beat_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_register_event_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_register_prefix_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_register_sequence_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_register_stream_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_register_timbre_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_register_timer(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_register_trigger_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_release_3D_sample_handle(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_release_all_timers() { }
__declspec(dllexport, naked) void __stdcall _AIL_release_channel(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_release_sample_handle(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_release_sequence_handle(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_release_timer_handle(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_resume_3D_sample(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_resume_sample(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_resume_sequence(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_sample_buffer_info(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall _AIL_sample_buffer_ready(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_sample_granularity(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_sample_loop_count(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_sample_ms_position(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_sample_pan(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_sample_playback_rate(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_sample_position(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_sample_reverb(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall _AIL_sample_status(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_sample_user_data(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_sample_volume(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_send_channel_voice_message(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall _AIL_send_sysex_message(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_sequence_loop_count(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_sequence_ms_position(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_sequence_position(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_sequence_status(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_sequence_tempo(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_sequence_user_data(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_sequence_volume(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_serve() { }
__declspec(dllexport, naked) void __stdcall _AIL_service_stream(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_3D_orientation(long arg1, long arg2, long arg3, long arg4, long arg5, long arg6, long arg7) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_3D_position(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_3D_provider_preference(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_3D_room_type(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_3D_sample_cone(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_3D_sample_distances(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_3D_sample_effects_level(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_3D_sample_file(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_3D_sample_info(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_3D_sample_loop_block(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_3D_sample_loop_count(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_3D_sample_obstruction(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_3D_sample_occlusion(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_3D_sample_offset(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_3D_sample_playback_rate(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_3D_sample_preference(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_3D_sample_volume(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_3D_speaker_type(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_3D_user_data(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_3D_velocity(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_3D_velocity_vector(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_DirectSound_HWND(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_XMIDI_master_volume(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_digital_driver_processor(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_digital_master_volume(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_error(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_filter_DLS_preference(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_filter_preference(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_filter_sample_preference(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_filter_stream_preference(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_input_state(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_named_sample_file(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_preference(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_redist_directory(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_sample_address(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_sample_adpcm_block_size(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_sample_file(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_sample_loop_block(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_sample_loop_count(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_sample_ms_position(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_sample_pan(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_sample_playback_rate(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_sample_position(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_sample_processor(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_sample_reverb(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_sample_type(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_sample_user_data(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_sample_volume(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_sequence_loop_count(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_sequence_ms_position(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_sequence_tempo(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_sequence_user_data(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_sequence_volume(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_stream_loop_block(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_stream_loop_count(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_stream_ms_position(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_stream_pan(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_stream_playback_rate(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_stream_position(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_stream_reverb(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_stream_user_data(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_stream_volume(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_timer_divisor(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_timer_frequency(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_timer_period(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_set_timer_user(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_shutdown() { }
__declspec(dllexport, naked) void __stdcall _AIL_size_processed_digital_audio(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall _AIL_start_3D_sample(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_start_all_timers() { }
__declspec(dllexport, naked) void __stdcall _AIL_start_sample(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_start_sequence(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_start_stream(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_start_timer(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_startup() { }
__declspec(dllexport, naked) void __stdcall _AIL_stop_3D_sample(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_stop_all_timers() { }
__declspec(dllexport, naked) void __stdcall _AIL_stop_sample(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_stop_sequence(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_stop_timer(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_stream_info(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall _AIL_stream_loop_count(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_stream_ms_position(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _AIL_stream_pan(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_stream_playback_rate(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_stream_position(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_stream_reverb(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall _AIL_stream_status(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_stream_user_data(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_stream_volume(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_true_sequence_channel(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _AIL_unlock() { }
__declspec(dllexport, naked) void __stdcall _AIL_unlock_mutex() { }
__declspec(dllexport, naked) void __stdcall _AIL_us_count() { }
__declspec(dllexport, naked) void __stdcall _AIL_waveOutClose(long arg1) { }
__declspec(dllexport, naked) void __stdcall _AIL_waveOutOpen(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall _DLSMSSGetCPU(long arg1) { }
__declspec(dllexport, naked) void __stdcall _DllMain(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _RIB_enumerate_providers(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _RIB_find_provider(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _RIB_load_application_providers(long arg1) { }
__declspec(dllexport, naked) void __stdcall _RIB_provider_library_handle() { }
__declspec(dllexport, naked) void __stdcall _RIB_provider_system_data(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _RIB_provider_user_data(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall _RIB_set_provider_system_data(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall _RIB_set_provider_user_data(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_3D_auto_update_position(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_3D_orientation(long arg1, long arg2, long arg3, long arg4, long arg5, long arg6, long arg7) { }
__declspec(dllexport, naked) void __stdcall AIL_3D_position(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall AIL_3D_provider_attribute(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_3D_room_type(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_3D_sample_attribute(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_3D_sample_cone(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall AIL_3D_sample_distances(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_3D_sample_effects_level(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_3D_sample_length(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_3D_sample_loop_count(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_3D_sample_obstruction(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_3D_sample_occlusion(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_3D_sample_offset(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_3D_sample_playback_rate(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_3D_sample_status(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_3D_sample_volume(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_3D_speaker_type(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_3D_update_position(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_3D_user_data(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_3D_velocity(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall AIL_DLS_close(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_DLS_compact(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_DLS_get_info(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_DLS_get_reverb(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall AIL_DLS_load_file(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_DLS_load_memory(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_DLS_open(long arg1, long arg2, long arg3, long arg4, long arg5, long arg6, long arg7) { }
__declspec(dllexport, naked) void __stdcall AIL_DLS_set_reverb(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall AIL_DLS_unload(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_HWND() { }
__declspec(dllexport, naked) void __stdcall AIL_MIDI_handle_reacquire(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_MIDI_handle_release(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_MIDI_to_XMI(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall AIL_MMX_available() { }
__declspec(dllexport, naked) void __stdcall AIL_WAV_file_write(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall AIL_WAV_info(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_XMIDI_master_volume(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_active_3D_sample_count(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_active_sample_count(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_active_sequence_count(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_allocate_3D_sample_handle(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_allocate_file_sample(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_allocate_sample_handle(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_allocate_sequence_handle(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_auto_service_stream(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_background() { }
__declspec(dllexport, naked) void __stdcall AIL_branch_index(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_channel_notes(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_close_3D_listener(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_close_3D_object(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_close_3D_provider(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_close_filter(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_close_input(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_close_stream(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_compress_ADPCM(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_compress_ASI(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall AIL_compress_DLS(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall AIL_controller_value(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_create_wave_synthesizer(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall AIL_decompress_ADPCM(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_decompress_ASI(long arg1, long arg2, long arg3, long arg4, long arg5, long arg6) { }
__declspec(dllexport, naked) void __stdcall AIL_delay(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_destroy_wave_synthesizer(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_digital_CPU_percent(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_digital_configuration(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall AIL_digital_handle_reacquire(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_digital_handle_release(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_digital_latency(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_digital_master_volume(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_end_3D_sample(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_end_sample(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_end_sequence(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_enumerate_3D_provider_attributes(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_enumerate_3D_providers(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_enumerate_3D_sample_attributes(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_enumerate_filter_attributes(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_enumerate_filter_sample_attributes(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_enumerate_filters(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_extract_DLS(long arg1, long arg2, long arg3, long arg4, long arg5, long arg6, long arg7) { }
__declspec(dllexport, naked) void __stdcall AIL_file_error() { }
__declspec(dllexport, naked) void __stdcall AIL_file_read(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_file_size(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_file_type(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_file_write(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_filter_DLS_attribute(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_filter_DLS_with_XMI(long arg1, long arg2, long arg3, long arg4, long arg5, long arg6) { }
__declspec(dllexport, naked) void __stdcall AIL_filter_attribute(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_filter_sample_attribute(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_filter_stream_attribute(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_find_DLS(long arg1, long arg2, long arg3, long arg4, long arg5, long arg6) { }
__declspec(dllexport, naked) void __stdcall AIL_get_DirectSound_info(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_get_preference(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_get_timer_highest_delay() { }
__declspec(dllexport, naked) void __stdcall AIL_init_sample(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_init_sequence(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_input_info(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_last_error() { }
__declspec(dllexport, naked) void __stdcall AIL_list_DLS(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall AIL_list_MIDI(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall AIL_load_sample_buffer(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall AIL_lock() { }
__declspec(dllexport, naked) void __stdcall AIL_lock_channel(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_lock_mutex() { }
__declspec(dllexport, naked) void __stdcall AIL_map_sequence_channel(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_mem_alloc_lock(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_mem_free_lock(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_merge_DLS_with_XMI(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall AIL_midiOutClose(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_midiOutOpen(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_minimum_sample_buffer_size(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_ms_count() { }
__declspec(dllexport, naked) void __stdcall AIL_open_3D_listener(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_open_3D_object(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_open_3D_provider(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_open_filter(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_open_input(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_open_stream(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_pause_stream(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_primary_digital_driver(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_process_digital_audio(long arg1, long arg2, long arg3, long arg4, long arg5, long arg6) { }
__declspec(dllexport, naked) void __stdcall AIL_quick_copy(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_quick_halt(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_quick_handles(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_quick_load(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_quick_load_and_play(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_quick_load_mem(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_quick_ms_length(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_quick_ms_position(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_quick_play(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_quick_set_ms_position(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_quick_set_reverb(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall AIL_quick_set_speed(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_quick_set_volume(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_quick_shutdown() { }
__declspec(dllexport, naked) void __stdcall AIL_quick_startup(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall AIL_quick_status(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_quick_type(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_quick_unload(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_redbook_close(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_redbook_eject(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_redbook_id(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_redbook_open(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_redbook_open_drive(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_redbook_pause(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_redbook_play(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_redbook_position(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_redbook_resume(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_redbook_retract(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_redbook_set_volume(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_redbook_status(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_redbook_stop(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_redbook_track(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_redbook_track_info(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall AIL_redbook_tracks(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_redbook_volume(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_register_3D_EOS_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_register_EOB_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_register_EOF_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_register_EOS_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_register_ICA_array(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_register_SOB_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_register_beat_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_register_event_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_register_prefix_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_register_sequence_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_register_stream_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_register_timbre_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_register_timer(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_register_trigger_callback(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_release_3D_sample_handle(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_release_all_timers() { }
__declspec(dllexport, naked) void __stdcall AIL_release_channel(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_release_sample_handle(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_release_sequence_handle(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_release_timer_handle(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_resume_3D_sample(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_resume_sample(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_resume_sequence(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_sample_buffer_info(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall AIL_sample_buffer_ready(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_sample_granularity(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_sample_loop_count(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_sample_ms_position(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_sample_pan(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_sample_playback_rate(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_sample_position(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_sample_reverb(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall AIL_sample_status(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_sample_user_data(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_sample_volume(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_send_channel_voice_message(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall AIL_send_sysex_message(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_sequence_loop_count(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_sequence_ms_position(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_sequence_position(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_sequence_status(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_sequence_tempo(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_sequence_user_data(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_sequence_volume(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_serve() { }
__declspec(dllexport, naked) void __stdcall AIL_service_stream(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_3D_orientation(long arg1, long arg2, long arg3, long arg4, long arg5, long arg6, long arg7) { }
__declspec(dllexport, naked) void __stdcall AIL_set_3D_position(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall AIL_set_3D_provider_preference(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_set_3D_room_type(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_3D_sample_cone(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall AIL_set_3D_sample_distances(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_set_3D_sample_effects_level(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_3D_sample_file(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_3D_sample_info(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_3D_sample_loop_block(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_set_3D_sample_loop_count(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_3D_sample_obstruction(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_3D_sample_occlusion(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_3D_sample_offset(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_3D_sample_playback_rate(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_3D_sample_preference(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_set_3D_sample_volume(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_3D_speaker_type(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_3D_user_data(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_set_3D_velocity(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall AIL_set_3D_velocity_vector(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall AIL_set_DirectSound_HWND(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_XMIDI_master_volume(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_digital_driver_processor(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_set_digital_master_volume(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_error(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_set_filter_DLS_preference(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_set_filter_preference(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_set_filter_sample_preference(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_set_filter_stream_preference(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_set_input_state(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_named_sample_file(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall AIL_set_preference(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_redist_directory(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_set_sample_address(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_set_sample_adpcm_block_size(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_sample_file(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_set_sample_loop_block(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_set_sample_loop_count(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_sample_ms_position(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_sample_pan(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_sample_playback_rate(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_sample_position(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_sample_processor(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_set_sample_reverb(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall AIL_set_sample_type(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_set_sample_user_data(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_set_sample_volume(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_sequence_loop_count(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_sequence_ms_position(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_sequence_tempo(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_set_sequence_user_data(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_set_sequence_volume(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_set_stream_loop_block(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_set_stream_loop_count(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_stream_ms_position(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_stream_pan(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_stream_playback_rate(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_stream_position(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_stream_reverb(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall AIL_set_stream_user_data(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_set_stream_volume(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_timer_divisor(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_timer_frequency(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_timer_period(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_set_timer_user(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_shutdown() { }
__declspec(dllexport, naked) void __stdcall AIL_size_processed_digital_audio(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall AIL_start_3D_sample(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_start_all_timers() { }
__declspec(dllexport, naked) void __stdcall AIL_start_sample(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_start_sequence(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_start_stream(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_start_timer(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_startup() { }
__declspec(dllexport, naked) void __stdcall AIL_stop_3D_sample(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_stop_all_timers() { }
__declspec(dllexport, naked) void __stdcall AIL_stop_sample(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_stop_sequence(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_stop_timer(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_stream_info(long arg1, long arg2, long arg3, long arg4, long arg5) { }
__declspec(dllexport, naked) void __stdcall AIL_stream_loop_count(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_stream_ms_position(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall AIL_stream_pan(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_stream_playback_rate(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_stream_position(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_stream_reverb(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall AIL_stream_status(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_stream_user_data(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_stream_volume(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_true_sequence_channel(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall AIL_unlock() { }
__declspec(dllexport, naked) void __stdcall AIL_unlock_mutex() { }
__declspec(dllexport, naked) void __stdcall AIL_us_count() { }
__declspec(dllexport, naked) void __stdcall AIL_waveOutClose(long arg1) { }
__declspec(dllexport, naked) void __stdcall AIL_waveOutOpen(long arg1, long arg2, long arg3, long arg4) { }
__declspec(dllexport, naked) void __stdcall DLSMSSGetCPU(long arg1) { }
__declspec(dllexport, naked) void __stdcall DllMain(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall RIB_enumerate_providers(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall RIB_find_provider(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall RIB_load_application_providers(long arg1) { }
__declspec(dllexport, naked) void __stdcall RIB_provider_library_handle() { }
__declspec(dllexport, naked) void __stdcall RIB_provider_system_data(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall RIB_provider_user_data(long arg1, long arg2) { }
__declspec(dllexport, naked) void __stdcall RIB_set_provider_system_data(long arg1, long arg2, long arg3) { }
__declspec(dllexport, naked) void __stdcall RIB_set_provider_user_data(long arg1, long arg2, long arg3) { }

}

/*BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDLL,
	_In_ DWORD     fdwReason,
	_In_ LPVOID    lpvReserved
)
{
	return TRUE;
}
*/