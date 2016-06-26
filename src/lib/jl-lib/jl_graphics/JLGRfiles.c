#include "JLGRinternal.h"

static char* jlgr_file_fullname__(jlgr_t* jlgr, char* selecteddir,
	char* selecteditem)
{
	char *newdir = malloc(
		strlen(jlgr->fl.dirname) +
		strlen(selecteditem) + 2);
	memcpy(newdir, jlgr->fl.dirname,
		strlen(jlgr->fl.dirname));
	memcpy(newdir + strlen(jlgr->fl.dirname),
		selecteditem,
		strlen(selecteditem));
	newdir[strlen(jlgr->fl.dirname) +
		strlen(selecteditem)] = '/';
	newdir[strlen(jlgr->fl.dirname) +
		strlen(selecteditem) + 1] = '\0';
	return newdir;
}

static void jl_fl_user_select_check_extradir__(char *dirname) {
	if(dirname[strlen(dirname)-1] == '/' && strlen(dirname) > 1) {
		if(dirname[strlen(dirname)-2] == '/')
			dirname[strlen(dirname)-1] = '\0';
	}
}

// Return 1 on success
// Return 0 if directory not available
static uint8_t jl_fl_user_select_open_dir__(jlgr_t* jlgr, char *dirname) {
	DIR *dir;
	struct dirent *ent;
	str_t converted_filename;

	jl_fl_user_select_check_extradir__(dirname);
	if(dirname[1] == '\0') {
		jl_mem(jlgr->jl, dirname, 0);
		dirname = SDL_GetPrefPath("JL_Lib", "\0");
		jl_fl_user_select_check_extradir__(dirname);
	}
	jlgr->fl.dirname = dirname;
	jlgr->fl.cursor = 0;
	jlgr->fl.cpage = 0;
	converted_filename = jl_file_convert__(jlgr->jl, jlgr->fl.dirname);
	cl_list_clear(jlgr->fl.filelist);
//UnComment to test file system conversion code.
	JL_PRINT_DEBUG(jlgr->jl, "dirname=%s:%s\n", jlgr->fl.dirname,
		converted_filename);
	if ((dir = opendir (converted_filename)) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL) {
			char *element = malloc(strlen(ent->d_name) + 1);
			memcpy(element, ent->d_name, strlen(ent->d_name));
			element[strlen(ent->d_name)] = '\0';
			cl_list_add(jlgr->fl.filelist, element);
		}
		closedir(dir);
		jl_cl_list_alphabetize(jlgr->fl.filelist);
	} else {
		//Couldn't open Directory
		int errsv = errno;
		if(errsv == ENOTDIR) { //Not a directory - is a file
			jlgr->fl.returnit = 1;
			jlgr->fl.dirname[strlen(dirname)-1] = '\0';
			jlgr->fl.inloop = 0;
			// Exit this mode.
			jl_mode_switch(jlgr->jl, jlgr->jl->mode.which);
		}else if(errsv == ENOENT) { // Directory Doesn't Exist
			return 0;
		}else if(errsv == EACCES) { // Doesn't have permission
			return 0;
		}else if((errsv == EMFILE) || (errsv == ENFILE) ||
			(errsv == ENOMEM)) //Not enough memory!
		{
			jl_print(jlgr->jl, "FileViewer Can't Open Directory:"
				"Not Enough Memory!");
			exit(-1);
		}else{ //Unknown error
			jl_print(jlgr->jl, "FileViewer Can't Open Directory:"
				"Unknown Error!");
			exit(-1);
		}
	}
	return 1;
}

/**
 * Open directory for file viewer.
 * If '!' is put at the beginning of "program_name", then it's treated as a
 *	relative path instead of a program name.
 * @param jl: The library context
 * @param program_name: program name or '!'+relative path
 * @param newfiledata: any new files created with the fileviewer will
 *	automatically be saved with this data.
 * @param newfilesize: size of "newfiledata"
 * @returns 0: if can't open the directory. ( Doesn't exist, Bad permissions )
 * @returns 1: on success.
**/
uint8_t jlgr_openfile_init(jlgr_t* jlgr, str_t program_name, void *newfiledata,
	uint64_t newfilesize)
{
	jlgr->fl.returnit = 0;
	jlgr->fl.inloop = 1;
	jlgr->fl.newfiledata = newfiledata;
	jlgr->fl.newfilesize = newfilesize;
	jlgr->fl.prompt = 0;
	jlgr->fl.promptstring = NULL;
	if(program_name[0] == '!') {
		char *path = jl_mem_copy(jlgr->jl,program_name,
			strlen(program_name));
		return jl_fl_user_select_open_dir__(jlgr, path);
	}else{
		return jl_fl_user_select_open_dir__(jlgr,
			SDL_GetPrefPath("JL_Lib",program_name));
	}
}

static void jl_fl_user_select_up__(jlgr_t* jlgr) {
	if((jlgr->fl.cursor > 0) || jlgr->fl.cpage) jlgr->fl.cursor--;
}

static void jl_fl_user_select_dn__(jlgr_t* jlgr) {
	if(jlgr->fl.cursor + (jlgr->fl.cpage * (jlgr->fl.drawupto+1)) <
		cl_list_count(jlgr->fl.filelist) - 1)
	{
		jlgr->fl.cursor++;
	}
}

static void jl_fl_user_select_rt__(jlgr_t* jlgr) {
	int i;
	for(i = 0; i < 5; i++) jl_fl_user_select_dn__(jlgr);
}

static void jl_fl_user_select_lt__(jlgr_t* jlgr) {
	int i;
	for(i = 0; i < 5; i++) jl_fl_user_select_up__(jlgr);
}

static void jl_fl_user_select_dir__(jlgr_t* jlgr, jlgr_input_t input) {
	if(input.h == JLGR_INPUT_PRESS_JUST) {
		// TODO: Fix Input
		jl_fl_user_select_lt__(jlgr);
		jl_fl_user_select_rt__(jlgr);
		jl_fl_user_select_up__(jlgr);
		jl_fl_user_select_dn__(jlgr);
	}
}

static void jl_fl_open_file__(jlgr_t* jlgr, char *selecteditem) {
	char *newdir = jlgr_file_fullname__(jlgr,
		jlgr->fl.dirname, selecteditem);

	free(jlgr->fl.dirname);
	jlgr->fl.dirname = NULL;
	jl_fl_user_select_open_dir__(jlgr,newdir);
}

static void jl_fl_user_select_do__(jlgr_t* jlgr, jlgr_input_t input) {
	if(input.h == 1) {
		struct cl_list_iterator *iterator;
		int i;
		char *stringtoprint;

		iterator = cl_list_iterator_create(jlgr->fl.filelist);
		for(i = 0; i < cl_list_count(jlgr->fl.filelist); i++) {
			stringtoprint = cl_list_iterator_next(iterator);
			if(i ==
				jlgr->fl.cursor + //ON PAGE
				(jlgr->fl.cpage * (jlgr->fl.drawupto+1))) //PAGE
			{
				jlgr->fl.selecteditem = stringtoprint;
				cl_list_iterator_destroy(iterator);
				break;
			}
		}
		if(strcmp(jlgr->fl.selecteditem, "..") == 0) {
			for(i = strlen(jlgr->fl.dirname)-2; i > 0; i--) {
				if(jlgr->fl.dirname[i] == '/') break;
				else jlgr->fl.dirname[i] = '\0';
			}
			jl_fl_user_select_open_dir__(jlgr,jlgr->fl.dirname);
		}else if(strcmp(jlgr->fl.selecteditem, ".") == 0) {
			jlgr->fl.inloop = 0;
			jl_mode_switch(jlgr->jl, jlgr->jl->mode.which);
		}else{
			jl_fl_open_file__(jlgr, jlgr->fl.selecteditem);
		}
	}
}

/**
 * Run the file viewer.
 * @param jlgr: The jlgr library Context.
**/
void jlgr_openfile_loop(jlgr_t* jlgr) {
	struct cl_list_iterator *iterator;
	int i;
	char *stringtoprint;

	jlgr->fl.drawupto = ((int)(20.f * jl_gl_ar(jlgr))) - 1;

	iterator = cl_list_iterator_create(jlgr->fl.filelist);

	jlgr_fill_image_set(jlgr, jlgr->textures.icon, 16, 16, 1, 1.);
	jlgr_fill_image_draw(jlgr);
	jlgr_draw_text(jlgr, "Select File", (jl_vec3_t) { .02, .02, 0. },
		jlgr->font);

	jlgr_input_do(jlgr, JL_INPUT_JOYC, jl_fl_user_select_dir__, NULL);
	//Draw files
	for(i = 0; i < cl_list_count(jlgr->fl.filelist); i++) {
		stringtoprint = cl_list_iterator_next(iterator);
		if(strcmp(stringtoprint, "..") == 0) {
			stringtoprint = "//containing folder//";
		}else if(strcmp(stringtoprint, ".") == 0) {
			stringtoprint = "//this folder//";
		}
		if(i - (jlgr->fl.cpage * (jlgr->fl.drawupto+1)) >= 0)
			jlgr_draw_text(jlgr, stringtoprint, (jl_vec3_t) {
				.06,
				.08 + (jlgr->font.size *
					(i - (jlgr->fl.cpage * (
					jlgr->fl.drawupto+1)))), 0. },
				jlgr->font);
		if(i - (jlgr->fl.cpage * (jlgr->fl.drawupto+1)) >
			jlgr->fl.drawupto - 1)
		{
			break;
		 	cl_list_iterator_destroy(iterator);
	 	}
	}
	if(jlgr->fl.cursor > jlgr->fl.drawupto) {
		jlgr->fl.cursor = 0;
		jlgr->fl.cpage++;
	}
	if(jlgr->fl.cursor < 0) {
		jlgr->fl.cursor = jlgr->fl.drawupto;
		jlgr->fl.cpage--;
	}
	if(jlgr->fl.prompt) {
		if(jlgr_draw_textbox(jlgr, .02, jl_gl_ar(jlgr) - .06, .94, .02,
			jlgr->fl.promptstring))
		{
			char *name = jlgr_file_fullname__(jlgr,
				jlgr->fl.dirname,
				(char*)jlgr->fl.promptstring->data);
			name[strlen(name) - 1] = '\0';
			jl_file_save(jlgr->jl, jlgr->fl.newfiledata,
				name, jlgr->fl.newfilesize);
			jl_fl_user_select_open_dir__(jlgr, jlgr->fl.dirname);
			jlgr->fl.prompt = 0;
		}
	}else{
		jlgr_draw_text(jlgr, ">", (jl_vec3_t) {
			.02, .08 + (.04 * jlgr->fl.cursor), 0. },
			jlgr->font);
		jlgr_draw_text(jlgr, jlgr->fl.dirname,
			(jl_vec3_t) { .02, jl_gl_ar(jlgr) - .02, 0. },
			(jl_font_t) { jlgr->textures.icon, 0,
				jlgr->fontcolor, .02});
		jlgr_input_do(jlgr, JL_INPUT_SELECT, jl_fl_user_select_do__, NULL);
	}
	jlgr_sprite_loop(jlgr, &jlgr->fl.btns[0]);
	jlgr_sprite_loop(jlgr, &jlgr->fl.btns[1]);
}

/**
 * Get the results from the file viewer.
 * @param jl: Library Context.
 * @returns: If done, name of selected file.  If not done, NULL is returned.
**/
str_t jlgr_openfile_kill(jlgr_t* jlgr) {
	if(jlgr->fl.returnit)
		return jlgr->fl.dirname;
	else
		return NULL;
}

static void jl_fl_btn_makefile_press__(jlgr_t* jlgr, jlgr_input_t input) {
	jlgr->fl.prompt = 1;
}

static void jl_fl_btn_makefile_loop__(jl_t* jl, jl_sprite_t* sprite) {
	jlgr_t* jlgr = jl->jlgr;

	//TODO: make graphic: 0, 1, 1, 255
	if(jlgr->fl.newfiledata)
		jlgr_glow_button_draw(jlgr, &jlgr->fl.btns[0], "+ New File",
			jl_fl_btn_makefile_press__);
}

static void jl_fl_btn_makefile_draw__(jl_t* jl, uint8_t resize, void* ctx) {
	jlgr_t* jlgr = jl->jlgr;
	jl_rect_t rc = { 0., 0., jl_gl_ar(jlgr), jl_gl_ar(jlgr) };
	jl_vec3_t tr = { 0., 0., 0. };

	jlgr_vos_image(jlgr, &jlgr->gl.temp_vo, rc, jlgr->textures.icon, 1.);
	jl_gl_vo_txmap(jlgr, &jlgr->gl.temp_vo, 16, 16, 9);
	jlgr_draw_vo(jlgr, &jlgr->gl.temp_vo, &tr);
}

static void jl_fl_btn_makefolder_loop__(jl_t* jl, jl_sprite_t* sprite) {
	jlgr_t* jlgr = jl->jlgr;
	
	//TODO: make graphic: 0, 1, 2, 255,
	jlgr_glow_button_draw(jlgr, &jlgr->fl.btns[1], "+ New Folder",
		jl_fl_btn_makefile_press__);
}

static void jl_fl_btn_makefolder_draw__(jl_t* jl, uint8_t resize, void* ctx) {
	jlgr_t* jlgr = jl->jlgr;
	jl_rect_t rc = { 0., 0., jl_gl_ar(jlgr), jl_gl_ar(jlgr) };
	jl_vec3_t tr = { 0., 0., 0. };

	jlgr_vos_image(jlgr, &jlgr->gl.temp_vo, rc, jlgr->textures.icon, 1.);
	jl_gl_vo_txmap(jlgr, &jlgr->gl.temp_vo, 16, 16, 10);
	jlgr_draw_vo(jlgr, &jlgr->gl.temp_vo, &tr);
}

void jlgr_fl_init(jlgr_t* jlgr) {
	jl_rect_t rc1 = { .8, 0., .1, .1 };
	jl_rect_t rc2 = { .9, 0., .1, .1 };

	//Create the variables
	jlgr->fl.filelist = cl_list_create();
	jlgr->fl.inloop = 0;
	jlgr_sprite_init(jlgr, &jlgr->fl.btns[0], rc1,
		jl_fl_btn_makefile_loop__, jl_fl_btn_makefile_draw__,
		NULL, 0, NULL, 0);
	jlgr_sprite_init(jlgr, &jlgr->fl.btns[1], rc2,
		jl_fl_btn_makefolder_loop__, jl_fl_btn_makefolder_draw__,
		NULL, 0, NULL, 0);
	jlgr->jl->has.fileviewer = 1;
}

void jlgr_file_kill_(jlgr_t* jlgr) {
	if(jlgr->jl->has.fileviewer) {
		JL_PRINT_DEBUG(jlgr->jl, "killing fl....");
		cl_list_destroy(jlgr->fl.filelist);
		JL_PRINT_DEBUG(jlgr->jl, "killed fl!");
	}
}
