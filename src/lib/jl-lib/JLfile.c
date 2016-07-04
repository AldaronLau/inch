/*
 * JL_lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLfiles.c
 * 	This allows you to modify the file system.  It uses libzip.
 */
/** @cond **/
#include "JLprivate.h"
#include "SDL_filesystem.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// LIBZIP
#define ZIP_DISABLE_DEPRECATED //Don't allow the old functions.
#include "zip.h"

#define PKFMAX 10000000
#define JL_FL_PERMISSIONS ( S_IRWXU | S_IRWXG | S_IRWXO )

#if JL_PLAT == JL_PLAT_PHONE
	extern const char* JL_FL_BASE;
#endif

// This function converts linux filenames to native filnames
char* jl_file_convert__(jl_t* jl, const char* filename) {
	data_t src; jl_data_mkfrom_str(&src, filename);
	data_t converted; jl_data_init(jl, &converted, 0);

	if(jl_data_test_next(&src, "!")) {
		src.curs++; // ignore
	}else{
		src.curs++; // ignore
		jl_data_merg(jl, &converted, &jl->fl.separator);
	}
	while(1) {
		data_t append; jl_data_read_upto(jl, &append, &src, '/', 300); 
		if(append.data[0] == '\0') break;
		jl_data_merg(jl, &converted, &append);
		if(jl_data_byte(&src) == '/')
			jl_data_merg(jl, &converted, &jl->fl.separator);
		src.curs++; // Skip '/'
		jl_data_free(&append);
	}
	jl_data_free(&src);
	return jl_data_tostring(jl, &converted);
}

static int jl_file_save_(jl_t* jl, const void *file_data, const char *file_name,
	uint32_t bytes)
{
	int errsv;
	ssize_t n_bytes;
	int fd;
	
	if(file_name == NULL) {
		jl_print(jl, "Save[file_name]: is Null");
		exit(-1);
	}else if(strlen(file_name) == 0) {
		jl_print(jl, "Save[strlen]: file_name is Empty String");
		exit(-1);
	}else if(!file_data) {
		jl_print(jl, "Save[file_data]: file_data is NULL");
		exit(-1);
	}

	const char* converted_filename = jl_file_convert__(jl, file_name);
	fd = open(converted_filename, O_RDWR | O_CREAT, JL_FL_PERMISSIONS);

	if(fd <= 0) {
		errsv = errno;

		JL_PRINT("Save/Open: ");
		JL_PRINT("\tFailed to open file: \"%s\"",
			converted_filename);
		JL_PRINT("\tWrite failed: %s", strerror(errsv));
		exit(-1);
	}
	int at = lseek(fd, 0, SEEK_END);
	n_bytes = write(fd, file_data, bytes);
	if(n_bytes <= 0) {
		errsv = errno;
		close(fd);
		jl_print(jl, ":Save[write]: Write to \"%s\" failed:");
		jl_print(jl, "\"%s\"", strerror(errsv));
		exit(-1);
	}
	close(fd);
	return at;
}

static inline void jl_file_reset_cursor__(const char* file_name) {
	int fd = open(file_name, O_RDWR);
	lseek(fd, 0, SEEK_SET);
	close(fd);
}

static inline void jl_file_get_root__(jl_t * jl) {
	data_t root_path;

#if JL_PLAT == JL_PLAT_PHONE
	data_t root_dir;

	JL_PRINT_DEBUG(jl, "Get external storage directory.");
	jl_data_mkfrom_str(&root_path, JL_FL_BASE);
	JL_PRINT_DEBUG(jl, "Append JL_ROOT_DIR.");
	jl_data_mkfrom_str(&root_dir, JL_ROOT_DIR);
	JL_PRINT_DEBUG(jl, "Merging root_path and root_dir.");
	jl_data_merg(jl, &root_path, &root_dir);
	JL_PRINT_DEBUG(jl, "Free root_dir.");
	jl_data_free(&root_dir);
#elif JL_PLAT_RPI
	data_t root_dir;

	JL_PRINT_DEBUG(jl, "Get external storage directory.");
	jl_data_mkfrom_str(&root_path, "/home/pi/.local/share/");
	JL_PRINT_DEBUG(jl, "Append JL_ROOT_DIR.");
	jl_data_mkfrom_str(&root_dir, JL_ROOT_DIR);
	JL_PRINT_DEBUG(jl, "Merging root_path and root_dir.");
	jl_data_merg(jl, &root_path, &root_dir);
	JL_PRINT_DEBUG(jl, "Free root_dir.");
	jl_data_free(&root_dir);
#else
	// Get the operating systems prefered path
	char* pref_path = SDL_GetPrefPath(JL_ROOT_DIRNAME, "\0");

	if(!pref_path) {
		jl_print(jl, "This platform has no pref path!");
		exit(-1);
	}
	// Erase extra non-needed '/'s
	pref_path[strlen(pref_path) - 1] = '\0';
	// Set root path to pref path
	jl_data_mkfrom_str(&root_path, pref_path);
	// Free the pointer to pref path
	SDL_free(pref_path);
#endif
	// Make "-- JL_ROOT_DIR"
	if(jl_file_dir_mk(jl, (char*) root_path.data) == 2) {
		jl_print(jl, (char*) root_path.data);
		jl_print(jl, ": mkdir : Permission Denied");
		exit(-1);
	}
	// Set paths.root & free root_path
	jl->fl.paths.root = jl_data_tostring(jl, &root_path);
	JL_PRINT_DEBUG(jl, "Root Path=\"%s\"", jl->fl.paths.root);
}

static inline void jl_file_get_errf__(jl_t * jl) {
	data_t fname; jl_data_mkfrom_str(&fname, "errf.txt");
	// Add the root path
	data_t errfs; jl_data_mkfrom_str(&errfs, jl->fl.paths.root);

	// Add the file name
	jl_data_merg(jl, &errfs, &fname);
	// Free fname
	jl_data_free(&fname);
	// Set paths.errf & free errfs
	jl->fl.paths.errf = jl_data_tostring(jl, &errfs);
}

// NON-STATIC Library Dependent Functions

/** @endcond **/

/**
 * Print text to a file.
 * @param jl: The library context.
 * @param fname: The name of the file to print to.
 * @param msg: The text to print.
**/
void jl_file_print(jl_t* jl, const char* fname, const char* msg) {
	// Write to the errf logfile
	if(jl->has.filesys && fname) jl_file_save_(jl, msg, fname, strlen(msg));
}

/**
 * Check whether a file or directory exists.
 * @param jl: The library context.
 * @param path: The path to the file to check.
 * @returns 0: If the file doesn't exist.
 * @returns 1: If the file does exist and is a directory.
 * @returns 2: If the file does exist and isn't a directory.
 * @returns 3: If the file exists and the user doesn't have permissions to open.
 * @returns 255: This should never happen.
**/
uint8_t jl_file_exist(jl_t* jl, const char* path) {
	DIR *dir;
	if ((dir = opendir (path)) == NULL) {
		//Couldn't open Directory
		int errsv = errno;
		if(errsv == ENOTDIR) { //Not a directory - is a file
			return 2;
		}else if(errsv == ENOENT) { // Directory Doesn't Exist
			return 0;
		}else if(errsv == EACCES) { // Doesn't have permission
			return 3;
		}else if((errsv == EMFILE) || (errsv == ENFILE) ||
			(errsv == ENOMEM)) //Not enough memory!
		{
			jl_print(jl, "jl_file_exist: Out of Memory!");
			exit(-1);
		}else{ //Unknown error
			jl_print(jl, "jl_file_exist: Unknown Error!");
			exit(-1);
		}
	}else{
		return 1; // Directory Does exist
	}
	return 255;
}

/**
 * Delete a file.
 * @param jl: The library context.
 * @param filename: The path of the file to delete.
**/
void jl_file_rm(jl_t* jl, const char* filename) {
	const char* converted_filename = jl_file_convert__(jl, filename);

	unlink(converted_filename);
}

/**
 * Save A File To The File System.  Save Data of "bytes" bytes in "file" to
 * file "name"
 * @param jl: Library Context
 * @param file: Data To Save To File
 * @param name: The Name Of The File to save to
 * @param bytes: Size of "File"
 */
void jl_file_save(jl_t* jl, const void *file, const char *name, uint32_t bytes){
	// delete file
	jl_file_rm(jl, name);
	// make file
	jl_file_save_(jl, file, name, bytes);
}

/**
 * Load a File from the file system.  Returns bytes loaded from "file_name"
 * @param jl: Library Context
 * @param load: Location to store loaded data.
 * @param file_name: file to load
 * @returns A readable "strt" containing the bytes from the file.
 */
void jl_file_load(jl_t* jl, data_t* load, const char* file_name) {
	jl_file_reset_cursor__(file_name);
	unsigned char *file = malloc(MAXFILELEN);
	const char* converted_filename = jl_file_convert__(jl, file_name);
	int fd = open(converted_filename, O_RDWR);
	
	//Open Block FLLD
	jl_print_function(jl, "jl_file_load");
	
	if(fd <= 0) {
		int errsv = errno;

		jl_print(jl, "jl_file_load/open: ");
		jl_print(jl, "\tFailed to open file: \"%s\"", converted_filename);
		jl_print(jl, "\tLoad failed because: %s", strerror(errsv));
		if(errsv == ENOENT) {
			// Doesn't exist
			jl_print_return(jl, "jl_file_load");
		}else{
			// Is a Directory
			exit(-1);
		}
		jl->errf = JL_ERR_FIND;
		return;
	}
	int Read = read(fd, file, MAXFILELEN);
	jl->info = Read;

	JL_PRINT_DEBUG(jl, "jl_file_load(): read %d bytes", jl->info);
	close(fd);

	if(jl->info) jl_data_mkfrom_data(jl, load, jl->info, file);
	
	jl_print_return(jl, "jl_file_load"); //Close Block "FLLD"
}

/**
 * Save file "filename" with contents "data" of size "dataSize" to package
 * "packageFileName"
 * @param jl: Library Context
 * @param packageFileName: Name of package to Save to
 * @param fileName: the file to Save to within the package.
 * @param data: the data to save to the file
 * @param dataSize: the # of bytes to save from the data to the file.
 * @returns 0: On success
 * @returns 1: If File is unable to be made.
 */
char jl_file_pk_save(jl_t* jl, const char* packageFileName,
	const char* fileName, void *data, uint64_t dataSize)
{
	const char* converted = jl_file_convert__(jl, packageFileName);

	jl_print_function(jl, "jl_file_pk_save");
	JL_PRINT_DEBUG(jl, "opening \"%s\"....", converted);
	struct zip *archive = zip_open(converted, ZIP_CREATE 
		| ZIP_CHECKCONS, NULL);
	if(archive == NULL) {
		jl_print_return(jl, "FL_PkSave");
		return 1;
	}else{
		JL_PRINT_DEBUG(jl, "opened package, \"%d\".", converted);
	}

	struct zip_source *s;
	if ((s=zip_source_buffer(archive, (void *)data, dataSize, 0)) == NULL) {
		zip_source_free(s);
		JL_PRINT_DEBUG(jl, "src null error[replace]: %s",
			(char *)zip_strerror(archive));
		jl_print_return(jl, "FL_PkSave");
		exit(-1);
	}
//	JL_PRINT("%d,%d,%d\n",archive,sb.index,s);
	if(zip_file_add(archive, fileName, s, ZIP_FL_OVERWRITE)) {
		JL_PRINT_DEBUG(jl, "add/err: \"%s\"", zip_strerror(archive));
	}else{
		JL_PRINT_DEBUG(jl, "added \"%s\" to file sys.", fileName);
	}
	zip_close(archive);
	JL_PRINT_DEBUG(jl, "DONE!");
	jl_print_return(jl, "jl_file_pk_save");
	return 0;
}

static void jl_file_pk_compress_iterate__(jl_t* jl, void* data) {
	char* name = jl_mem_temp(jl, NULL);
	data_t read; jl_file_load(jl, &read, data);

	jl_file_pk_save(jl, name, data + strlen(name)-4, read.data, read.size);
	jl_print(jl, "\t%s", data);
	jl_mem_temp(jl, name);
	jl_data_free(&read);
}

/**
 * Compress a folder in a package.
 * @param jl: The library context.
 * @param folderName: Name of the folder to compress.
 * @returns: Name of package, needs to be free'd.
**/
char* jl_file_pk_compress(jl_t* jl, const char* folderName) {
	// If last character is '/', then overwrite.
	uint32_t cursor = strlen(folderName);
	if(folderName[cursor - 1] == '/') cursor--;
	// Name+.zip\0
	char* pkName = jl_memi(jl, cursor + 5);
	jl_mem_copyto(folderName, pkName, cursor);
	pkName[cursor] = '.';
	cursor++;
	pkName[cursor] = 'z';
	cursor++;
	pkName[cursor] = 'i';
	cursor++;
	pkName[cursor] = 'p';
	cursor++;
	pkName[cursor] = '\0';
	// Overwrite any existing package with same name
	jl_file_rm(jl, pkName);
	//
	jl_mem_temp(jl, pkName);
	// Find Filse
	struct cl_list * filelist = jl_file_dir_ls(jl, folderName, 1);
	// Save Files Into Package
	jl_clump_list_iterate(jl, filelist, jl_file_pk_compress_iterate__);
	// Free 
	cl_list_destroy(filelist);
	return pkName;
}

static void jl_file_pk_load_quit__(jl_t* jl) {
	jl_print_return(jl, "FL_PkLd"); //Close Block "FL_PkLd"
}

/**
 * Load a zip package from memory.
 * @param jl: The library context.
 * @param rtn: An empty data_t structure to return to.  Needs to be freed.
 * @param data: The data that contains the zip file.
 * @param file_name: The name of the file to load.
**/
void jl_file_pk_load_fdata(jl_t* jl, data_t* rtn, data_t* data,
	const char* file_name)
{
	zip_error_t ze; ze.zip_err = ZIP_ER_OK;
	zip_source_t *file_data;
	int zerror = 0;

	file_data = zip_source_buffer_create(data->data, data->size, 0, &ze);

	if(ze.zip_err != ZIP_ER_OK) {
		jl_print(jl, "couldn't make pckg buffer!");
		//zip_error_init_with_code(&ze, ze.zip_err);
		jl_print(jl, "because: \"%s\"", zip_error_strerror(&ze));
		exit(-1);
	}

	JL_PRINT_DEBUG(jl, "error check 2.");
	struct zip *zipfile = zip_open_from_source(file_data,
		ZIP_CHECKCONS | ZIP_RDONLY, &ze);

	if(ze.zip_err != ZIP_ER_OK) {
		zip_error_init_with_code(&ze, ze.zip_err);
		jl_print(jl, "couldn't load pckg file");
		jl_print(jl, "because: \"%s\"", zip_error_strerror(&ze));
//		char name[3]; name[0] = data->data[0]; name[1] = '\0';
//		jl_print(jl, "First character = %s %d", data->data, data->data[0]);
		exit(-1);
	}

//	struct zip *zipfile = zip_open(converted, ZIP_CHECKCONS, &zerror);
	JL_PRINT_DEBUG(jl, "error check 3.");
	if(zipfile == NULL) {
		jl_print(jl, "couldn't load zip because:");
		if(zerror == ZIP_ER_INCONS) {
			jl_print(jl, "\tcorrupt file");
		}else if(zerror == ZIP_ER_NOZIP) {
			jl_print(jl, "\tnot a zip file");
		}else{
			jl_print(jl, "\tunknown error");
		}
		exit(-1);
	}
	JL_PRINT_DEBUG(jl, "error check 4.");
	JL_PRINT_DEBUG(jl, (char *)zip_strerror(zipfile));
	JL_PRINT_DEBUG(jl, "loaded package.");
	unsigned char *fileToLoad = malloc(PKFMAX);
	JL_PRINT_DEBUG(jl, "opening file in package....");
	struct zip_file *file = zip_fopen(zipfile, file_name, ZIP_FL_UNCHANGED);
	JL_PRINT_DEBUG(jl, "call pass.");
	if(file == NULL) {
		jl_print(jl, "couldn't open up file: \"%s\" in package:",
			file_name);
		jl_print(jl, "because: %s", (void *)zip_strerror(zipfile));
		jl_print(jl, zip_get_name(zipfile, 0, ZIP_FL_UNCHANGED));
		jl->errf = JL_ERR_NONE;
		return;
	}
	JL_PRINT_DEBUG(jl, "opened file in package / reading opened file....");
	if((jl->info = zip_fread(file, fileToLoad, PKFMAX)) == -1) {
		jl_print(jl, "file reading failed");
		exit(-1);
	}
	if(jl->info == 0) {
		JL_PRINT_DEBUG(jl, "empty file, returning NULL.");
		return;
	}
	JL_PRINT_DEBUG(jl, "jl_file_pk_load: read %d bytes", jl->info);
	zip_close(zipfile);
	JL_PRINT_DEBUG(jl, "closed file.");
	// Make a data_t* from the data.
	if(jl->info) jl_data_mkfrom_data(jl, rtn, jl->info, fileToLoad);
	JL_PRINT_DEBUG(jl, "done.");
	jl->errf = JL_ERR_NERR;
}

/**
 * Load file "filename" in package "packageFileName" & Return contents
 * May return NULL.  If it does jl->errf will be set.
 * -ERR:
 *	-ERR_NERR:	File is empty.
 *	-ERR_NONE:	Can't find filename in packageFileName. [ DNE ]
 *	-ERR_FIND:	Can't find packageFileName. [ DNE ]
 * @param jl: Library Context
 * @param rtn: Data structure to return.
 * @param packageFileName: Package to load file from
 * @param filename: file within package to load
 * @returns: contents of file ( "filename" ) in package ( "packageFileName" )
*/
void jl_file_pk_load(jl_t* jl, data_t* rtn, const char *packageFileName,
	const char *filename)
{
	const char* converted = jl_file_convert__(jl, packageFileName);

	jl->errf = JL_ERR_NERR;
	jl_print_function(jl, "FL_PkLd");

	JL_PRINT_DEBUG(jl, "loading package:\"%s\"...", converted);

	data_t data; jl_file_load(jl, &data, converted);
	JL_PRINT_DEBUG(jl, "error check 1.");
	if(jl->errf == JL_ERR_FIND) {
		JL_PRINT_DEBUG(jl, "!Package File doesn't exist!");
		jl_file_pk_load_quit__(jl);
		return;
	}
	jl_file_pk_load_fdata(jl, rtn, &data, filename);
	if(jl->errf) exit(-1);
	jl_file_pk_load_quit__(jl);
	return;
}

/**
 * Create a folder (directory)
 * @param jl: library context
 * @param pfilebase: name of directory to create
 * @returns 0: Success
 * @returns 1: If the directory already exists.
 * @returns 2: Permission Error
 * @returns 255: Never.
*/
uint8_t jl_file_dir_mk(jl_t* jl, const char* path) {
	uint8_t rtn = 255;

	jl_print_function(jl, "FL_MkDir");
	if(mkdir(path, JL_FL_PERMISSIONS)) {
		int errsv = errno;
		if(errsv == EEXIST) {
			rtn = 1;
		}else if((errsv == EACCES) || (errsv == EROFS)) {
			rtn = 2;
		}else{
			jl_print(jl, "couldn't mkdir:%s", strerror(errsv));
			exit(-1);
		}
	}else{
		rtn = 0;
	}
	jl_print_return(jl, "FL_MkDir");
	// Return
	return rtn;
}

static int8_t jl_file_dirls__(jl_t* jl,const char* filename,uint8_t recursive,
	struct cl_list * filelist)
{
	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir (filename)) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL) {
			char *element = malloc(strlen(filename) +
				strlen(ent->d_name) + 3);
			element[0] = '!';
			memcpy(1 + element, filename, strlen(filename));
			element[1 + strlen(filename)] = '/';
			memcpy(1 + element + strlen(filename) + 1, ent->d_name,
				strlen(ent->d_name));
			element[1 + strlen(ent->d_name) + strlen(filename) + 1]
				= '\0';
			if(ent->d_name[0] == '.') continue;
			// Open And Read if directory
			if(jl_file_dirls__(jl, element+1, recursive, filelist)||
				!recursive)
			{
				// If wasn't directory nor recursive add file.
				cl_list_add(filelist, element);
			}
		}
		closedir(dir);
		return 0;
	}
	//Couldn't open Directory
	int errsv = errno;
	if(errsv == ENOTDIR) {
//		JL_PRINT_DEBUG(jl, "Can't Open Directory: Is a File!");
	}else if(errsv == ENOENT) {
//		JL_PRINT_DEBUG(jl, "Can't Open Directory: Doesn't Exist!");
	}else if(errsv == EACCES) {
//		JL_PRINT_DEBUG(jl, "Can't Open Directory: No Permission!");
	}else if((errsv == EMFILE) || (errsv == ENFILE) ||
		(errsv == ENOMEM)) //Not enough memory!
	{
		JL_PRINT("Can't Open Directory: Not Enough Memory!\n");
		exit(-1);
	}else{ //Unknown error
		JL_PRINT("Can't Open Directory: Unknown Error!\n");
		exit(-1);
	}
	return -1;
}

/**
 * List all of the files in a directory.
 * @param jl: The library context.
 * @param dirname: The name of the directory.
 * @param recursive: 0: list files and directories inside of "dirname", 1: list
 *	all files inside of "dirname" and inside of all other directories under
 *	dirname.
 * @returns: List of files inside of "dirname", needs to be freed with
 *	cl_list_destroy()
**/
struct cl_list * jl_file_dir_ls(jl_t* jl,const char* dirname,uint8_t recursive){
	struct cl_list * filelist = cl_list_create();
	char* converted_dirname = jl_file_convert__(jl, dirname);
	char* end_character = &converted_dirname[strlen(converted_dirname) - 1];
	if(*end_character == '/') *end_character = '\0';

	if(jl_file_dirls__(jl, converted_dirname, recursive, filelist)) {
		cl_list_destroy(filelist);
		return NULL;
	}else{
		return filelist;
	}
}

/**
 * Get the designated location for a resource file. Resloc = Resource Location
 * @param jl: Library Context.
 * @param prg_folder: The name of the folder for all of the program's resources.
 *	For a company "PlopGrizzly" with game "Super Game":
 *		Pass: "PlopGrizzly_SG"
 *	For an individual game developer "Jeron Lau" with game "Cool Game":
 *		Pass: "JeronLau_CG"
 *	If prg_folder is NULL, uses the program name from jl_start.
 * @param fname: Name Of Resource Pack
 * @returns: The designated location for a resouce pack
*/
char* jl_file_get_resloc(jl_t* jl, const char* prg_folder, const char* fname) {
	data_t filesr; jl_data_mkfrom_str(&filesr, JL_FILE_SEPARATOR);
	data_t pfstrt; jl_data_mkfrom_str(&pfstrt, prg_folder);
	data_t fnstrt; jl_data_mkfrom_str(&fnstrt, fname);
	data_t resloc; jl_data_mkfrom_str(&resloc, jl->fl.paths.root);
	char * rtn = NULL;
	
	//Open Block "FLBS"
	jl_print_function(jl, "FL_Base");
	
	JL_PRINT_DEBUG(jl, "Getting Resource Location....");
	// Append 'prg_folder' onto 'resloc'
	jl_data_merg(jl, &resloc, &pfstrt);
	// Append 'filesr' onto 'resloc'
	jl_data_merg(jl, &resloc, &filesr);
	// Make 'prg_folder' if it doesn't already exist.
	if( jl_file_dir_mk(jl, (char*) resloc.data) == 2 ) {
		jl_print(jl, "jl_file_get_resloc: couldn't make \"%s\"",
			(char*) resloc.data);
		jl_print(jl, "mkdir : Permission Denied");
		exit(-1);
	}
	// Append 'fname' onto 'resloc'
	jl_data_merg(jl, &resloc, &fnstrt);
	// Set 'rtn' to 'resloc' and free 'resloc'
	rtn = jl_data_tostring(jl, &resloc);
	// Free pfstrt & fnstrt & filesr
	jl_data_free(&pfstrt),jl_data_free(&fnstrt),jl_data_free(&filesr);
	// Close Block "FLBS"
	jl_print_return(jl, "FL_Base");
	//jl_print(jl, "finished resloc w/ \"%s\"", rtn); 
	return rtn;
}

void jl_file_init_(jl_t * jl) {
	jl_print_function(jl, "FL_Init");
	// Find out the native file separator.
	jl_data_mkfrom_str(&jl->fl.separator, "/");
	// Get ( and if need be, make ) the directory for everything.
	JL_PRINT_DEBUG(jl, "Get/Make directory for everything....");
	jl_file_get_root__(jl);
	JL_PRINT_DEBUG(jl, "Complete!");
	// Get ( and if need be, make ) the error file.
	JL_PRINT_DEBUG(jl, "Get/Make directory error logfile....");
	jl_file_get_errf__(jl);
	JL_PRINT_DEBUG(jl, "Complete!");
	//
	jl->has.filesys = 1;

	const char* pkfl = jl_file_get_resloc(jl, JL_MAIN_DIR, JL_MAIN_MEF);
	remove(pkfl);

	truncate(jl->fl.paths.errf, 0);
	JL_PRINT_DEBUG(jl, "Starting....");
	JL_PRINT_DEBUG(jl, "finished file init");
	jl_print_return(jl, "FL_Init");
}
