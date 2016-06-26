/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLGRthread.c
 *	This file handles a separate thread for drawing graphics.
**/
#include "JLGRinternal.h"

static void jlgr_thread_programsresize(jlgr_t* jlgr) {
	jl_fnct resize_ = jlgr->draw.redraw.resize;
	resize_(jlgr->jl);
}

static void jlgr_thread_resize(jlgr_t* jlgr, uint16_t w, uint16_t h) {
	JL_PRINT_DEBUG(jlgr->jl, "Resizing to %dx%d....", w, h);
	// Set window size & aspect ratio stuff.
	jl_wm_resz__(jlgr, w, h);
	// Update the size of the background.
	jl_sg_resz__(jlgr->jl);
	// Taskbar resize.
	jlgr_menu_resize_(jlgr);
	// Mouse resize
	if(jlgr->mouse.mutex) jlgr_sprite_resize(jlgr, &jlgr->mouse, NULL);
}

static void jlgr_thread_event(jl_t* jl, void* data) {
	jlgr_t* jlgr = jl->jlgr;
	jlgr_thread_packet_t* packet = data;

	switch(packet->id) {
		case JLGR_COMM_RESIZE: {
			jl_wm_updatewh_(jlgr);
			if(packet->x == 0) packet->x = jlgr_wm_getw(jlgr);
			if(packet->y == 0) packet->y = jlgr_wm_geth(jlgr);
			jlgr_thread_resize(jlgr, packet->x, packet->y);
			jlgr_thread_programsresize(jlgr);
			break;
		} case JLGR_COMM_KILL: {
			JL_PRINT_DEBUG(jl, "Thread exiting....");
			jlgr->draw.rtn = 1;
			break;
		} case JLGR_COMM_SEND: {
			if(packet->x==0) jlgr->draw.redraw.single = packet->fn;
			if(packet->x==1) jlgr->draw.redraw.upper = packet->fn;
			if(packet->x==2) jlgr->draw.redraw.lower = packet->fn;
			if(packet->x==3) {
				jlgr->draw.fn = packet->fn;
				packet->fn(jl);
			}
			break;
		} case JLGR_COMM_NOTIFY: {
			jlgr_comm_notify_t* packeta = data;
			jl_mem_copyto(packeta->string,
				jlgr->gr.notification.message, 255);
			jlgr->gr.notification.timeTilVanish = 3.5;
			break;
		} default: {
			break;
		}
	}
}

static void jlgr_thread_resize_event(jl_t* jl, void* data) {
	jlgr_t* jlgr = jl->jlgr;
	jlgr_thread_packet_t* packet = data;

	switch(packet->id) {
		case JLGR_COMM_RESIZE: {
			uint16_t w = packet->x;
			uint16_t h = packet->y;
			JL_PRINT_DEBUG(jlgr->jl, "Resizing to %dx%d....", w, h);
			// Set window size & aspect ratio stuff.
			jl_wm_resz__(jlgr, w, h);
			break;
		} case JLGR_COMM_INIT: {
			jlgr->draw.fn = packet->fn;
			jlgr->draw.rtn = 2;
			break;
		} default: {
			break;
		}
	}
}

static uint8_t jlgr_thread_draw_event__(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;
	jlgr->draw.rtn = 0;

	jl_thread_comm_recv(jl, jlgr->comm2draw, jlgr_thread_event);
	return jlgr->draw.rtn;
}

static void jlgr_thread_draw_init__(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;

	// Initialize subsystems
	JL_PRINT_DEBUG(jl, "Creating the window....");
	jl_wm_init__(jlgr);
	
	jlgr_text_init__(jlgr);
	JL_PRINT_DEBUG(jl, "Loading default graphics from package....");
	jl_sg_init__(jlgr);
	JL_PRINT_DEBUG(jl, "Setting up OpenGL....");
	jl_gl_init__(jlgr);
	JL_PRINT_DEBUG(jl, "Setting up effects....");
	jlgr_effects_init__(jlgr);
	JL_PRINT_DEBUG(jl, "Load graphics....");
	jlgr_init__(jlgr);
	JL_PRINT_DEBUG(jl, "Creating Taskbar sprite....");
	jlgr_menubar_init__(jlgr);
	JL_PRINT_DEBUG(jl, "Creating Mouse sprite....");
	jlgr_mouse_init__(jlgr);
	JL_PRINT_DEBUG(jl, "User's Init....");
	jlgr->draw.rtn = 0;
	while(jlgr->draw.rtn != 2) {
		jl_thread_comm_recv(jl, jlgr->comm2draw,
			jlgr_thread_resize_event);
	}
	jlgr->draw.fn(jl);
	jlgr_thread_resize(jlgr, jlgr_wm_getw(jlgr), jlgr_wm_geth(jlgr));
	jlgr_wm_setwindowname(jlgr, jl->name);
	JL_PRINT_DEBUG(jl, "Sending finish packet....");
	// Tell main thread to stop waiting.
	jl_thread_wait_stop(jl, &jlgr->wait);
}

void jlgr_thread_send(jlgr_t* jlgr,uint8_t id,uint16_t x,uint16_t y,jl_fnct fn){
	jlgr_thread_packet_t packet = { id, x, y, fn };

	// Send resize packet.
	jl_thread_comm_send(jlgr->jl, jlgr->comm2draw, &packet);
}

int jlgr_thread_draw(void* data) {
	jl_t* jl = data;
	jlgr_t* jlgr = jl->jlgr;

	// Initialize subsystems
	jl_thread_mutex_use(jl, jlgr->mutex, jlgr_thread_draw_init__);
	// Redraw loop
	while(1) {
		// Check for events.
		if(jlgr_thread_draw_event__(jl)) break;
		// Deselect any pre-renderer.
		jlgr->gl.cp = NULL;
		//Redraw screen.
		_jl_sg_loop(jlgr);
		//Update Screen.
		jl_wm_loop__(jlgr);
	}
	jl_wm_kill__(jlgr); // Kill window
	jlgr_sprite_free(jlgr, &jlgr->sg.bg.up);
	jlgr_sprite_free(jlgr, &jlgr->sg.bg.dn);
	return 0;
}

void jlgr_thread_init(jlgr_t* jlgr) {
	jlgr->thread = jl_thread_new(jlgr->jl, "JL_Lib/Graphics",
		jlgr_thread_draw);
}

void jlgr_thread_kill(jlgr_t* jlgr) {
	jl_thread_old(jlgr->jl, jlgr->thread);
}
