/*
 *
 * Copyright  1990-2006 Sun Microsystems, Inc. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 only, as published by the Free Software Foundation. 
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details (a copy is
 * included at /legal/license.txt). 
 * 
 * You should have received a copy of the GNU General Public License
 * version 2 along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA 
 * 
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa
 * Clara, CA 95054 or visit www.sun.com if you need additional
 * information or have any questions. 
 */

/**
 * @file
 * @brief Content Handler Registry implementation based on POSIX file calls.
 */

#include <stdlib.h>
#include <string.h>

#include "javacall_chapi.h"
#include "inc/javautil_str.h"
#include "inc/javautil_storage.h"


#ifdef USE_NATIVE_REGISTRY
#include "inc/javacall_chapi_native.h"
#endif

#include "javacall_chapi_result.h"


/* Attention! Win32 specific implementation! */

/**
 * File path of the registry storage.
 */
static char* regFilePath = NULL;

#define MAX_PATH_LEN 256
#define FILE_SEPARATOR '/'

/* Opened registry data */
typedef struct {
    javautil_storage file; // opened registry file handle
    javacall_int32 fsize; // file size
    javacall_int32 cur;   // absolute position of handler data
    javacall_int32 hsize;            // handler size
    int offs[JAVACALL_CHAPI_FIELD_COUNT]; // field offsets for current handle
} REG;

/**
 * Macros that checks valid position settings for given field.
 * The result is boolean value: TRUE- success.
 */
#define SET_POS(reg_ptr, field)  (0 <= reg_ptr->cur && \
		 0 == javautil_storage_setpos(reg_ptr->file,(long) reg_ptr->cur + reg_ptr->offs[field], JUS_SEEK_SET))

#define CLEAN_HANDLER(id, suit, clas, flag) \
    if (id != NULL) { free(id); id = NULL; } \
	suit = 0; \
    if (clas != NULL) { free(clas); clas = NULL; } \
    flag = -1



/**
 * Reads integer value at given position.
 * @return integer, -1 if error.
 */
static int readInt(REG* reg, javacall_chapi_field field) {
    int val;

    if (reg->offs[field] <= 0  || 
       (!SET_POS(reg, field)) || 
      sizeof(int) != javautil_storage_read(reg->file, (void*)&val, sizeof(int))) {
        val = -1;
    }

    return val;
}

/**
 * Read string field of the current handler
 * Allocated memory for the string <code>str</code> should be freed after use.
 * @return loaded string size, -1 if error.
 */
static int readString(REG* reg, javacall_chapi_field field,
                                         /*OUT*/javacall_utf16** str) {
    int sz;
    javacall_utf16 *buf = NULL, c;

    do {
        if (reg->offs[field] <= 0) {
            sz = 0;
            break;
        }

        if ((!SET_POS(reg, field))  ||
            sizeof(c) != javautil_storage_read(reg->file, (void*)&c, sizeof(c) )) {
            sz = -1;
            break;
        }

        sz = c * sizeof(javacall_utf16);
        buf = (javacall_utf16*) malloc(sz);
        if (buf == NULL) {
            sz = -1;
            break;
        }

        if (sz != javautil_storage_read(reg->file, (void*)buf, sz)) {
            free(buf);
            sz = -1;
            break;
        }

        sz = c;
        *str = buf;
    } while (0);

    return sz;
}

/**
 * Read string-array field of the current handler.
 * The produced array structure is plain: string-by-string with string size 
 * element placed before string data (see schema below).
 *
 * index:     0          1..size_0  ...
 *  data:  [size_0] [ ... chars_0 ... ] [size_1] [ ... chars_1 ... ]
 *
 * Allocated memory for <code>array</code> should be freed after use.
 * @return number of loaded strings in array, -1 if error.
 */
static int readArray(REG* reg, javacall_chapi_field field,
                                         /*OUT*/javacall_utf16** array) {
    int n;
	int sz;
    javacall_utf16 *buf = NULL, c;

    do {
        if (reg->offs[field] <= 0) {
            n = 0;
            break;
        }

        if ((!SET_POS(reg, field))  ||
          sizeof(int) != 
          javautil_storage_read(reg->file,(void*)&sz, sizeof(int)) ||
          sizeof(javacall_utf16) != 
          javautil_storage_read(reg->file,(void*)&c, sizeof(javacall_utf16))) {
            n = -1;
            break;
        }

        buf = (javacall_utf16*) malloc(sz);
        if (buf == NULL) {
            n = -1;
            break;
        }

        if (sz != javautil_storage_read(reg->file,(void*)buf, sz)) {
            free(buf);
			buf = NULL;
            n = -1;
            break;
        }

        n = c;
        *array = buf;
    } while (0);

    return n;
}


/**
 * Loads current handler data.
 * Assumed that any handler should has non-empty ID string.
 * Also JVM handler should has valid suite ID and non-empty class name.
 * For platform (native) handler: suite ID and class name are always null.
 * Note! In this implementation native handler uses the class_name field 
 * for storing of the native application full path.
 * See implementation of @link javacall_chapi_execute_handler().
 */
static javacall_result loadHandler(REG* reg, 
        javacall_utf16** id, int* id_sz,
        int* suit,
        javacall_utf16** clas, int* clas_sz, int* flag) {

    if ((*id_sz = readString(reg, JAVACALL_CHAPI_FIELD_ID, id)) <= 0 || 
        (*flag = readInt(reg, JAVACALL_CHAPI_FIELD_FLAG)) < 0) {
        return JAVACALL_FAIL;
    }

    if (*flag < 2) { // JVM handler
        if ((*suit = 
                readInt(reg, JAVACALL_CHAPI_FIELD_SUITE)) <= 0 ||
            (*clas_sz = 
                readString(reg, JAVACALL_CHAPI_FIELD_CLASS, clas)) <= 0) {
        return JAVACALL_FAIL;
      }
    } else {
        *suit = *clas_sz = 0;
	}

    return JAVACALL_OK;
} 

/**
 * Write string to file.
 * @return Number of written bytes, -1 if error.
 */
static int writeString(REG* reg, const javacall_utf16_string str) {
    int sz = 0;
    javacall_utf16 c;

	if (str != NULL){
		sz = javautil_wcslen(str);
	}

    if (sz > 0) {
        c = sz;
        sz *= sizeof(javacall_utf16);

        if (sizeof(javacall_utf16) != 
          javautil_storage_write(reg->file, (void*)&c, sizeof(javacall_utf16)) ||
          sz != javautil_storage_write(reg->file, (void*)str, sz)) {
            sz = -1;
        } else {
			sz += sizeof(javacall_utf16);
		}
    }

    return sz;
}

/**
 * Write string-array.
 * The written array structure is correspond to described for 
 * @link readArray .
 *
 * @return Number of written bytes, -1 if error.
 */
static int writeArray(REG* reg, const javacall_utf16_string* array, int len) {
    int arr_sz = 0;

    if (len > 0 && array != NULL) {
        int i, str_sz;
        javacall_utf16 c;
        do {
            /* calculate total size */
            for (i = 0; i < len; i++) {
				str_sz = javautil_wcslen(array[i]);
                /* Only non-empty strings allowed */
                if (str_sz <= 0) {
                    arr_sz = -1;
                    break;
                }
                arr_sz += 1 + str_sz;
            }
    
            if (arr_sz <= 0) {
                break;
            }
    
            arr_sz *= sizeof(javacall_utf16);
            c = len;
            if (sizeof(int) != 
              javautil_storage_write(reg->file, (void*)&arr_sz, sizeof(int)) ||
              sizeof(javacall_utf16) != 
              javautil_storage_write(reg->file, (void*)&c, sizeof(javacall_utf16))) {
                arr_sz = -1;
                break;
            }
            arr_sz += sizeof(int) + sizeof(javacall_utf16);

            while (len--) {
                if (0 >= writeString(reg, *array++)) {
                    arr_sz = -1;
                    break;
                }
            }
        } while(0);
    }

    return arr_sz;
}

/**
 * Close registry file.
 */
static void regClose(REG* reg) {
    if (reg->file != JAVAUTIL_INVALID_STORAGE_HANDLE) {
        javautil_storage_close(reg->file);
    }
    reg->file = JAVAUTIL_INVALID_STORAGE_HANDLE;
    reg->fsize = reg->cur = -1;
}

/**
 * Loads CH structure for next handler.
 */
static javacall_result nextHandler(REG* reg) {
    javacall_result status;

    do {
        if (reg->cur < 0 || (reg->cur + reg->hsize) >= reg->fsize) {
            status = JAVACALL_END_OF_FILE;
            break;
        }

        status = javautil_storage_setpos(reg->file, reg->cur + reg->hsize, JUS_SEEK_SET);
        if (status!=JAVACALL_OK) {
            status = JAVACALL_IO_ERROR;
            break;
        }
		reg->cur += reg->hsize;

        if (sizeof(int) != 
            javautil_storage_read(reg->file,(void*)&(reg->hsize), sizeof(int)) ||
			reg->hsize == 0 ||
            sizeof(reg->offs) != javautil_storage_read(reg->file, (void*)&(reg->offs), sizeof(reg->offs))) {
            status = JAVACALL_IO_ERROR;
            break;
        }

        status = JAVACALL_OK;
    } while (0);

    if (status != JAVACALL_OK) {
        reg->cur = -1;
    }

    return status;
}

/**
 * Loads CH structure for next handler.
 */
static javacall_result nextAccessedHandler(REG* reg, 
                                    const javacall_utf16_string caller_id) {
    javacall_result status;
    javacall_utf16 *buf=0, *ptr;
    int n, test_sz, caller_sz = 0;

	if (caller_id)
		caller_sz = javautil_wcslen(caller_id);


    
    while ((status = nextHandler(reg)) == JAVACALL_OK && 
            caller_sz &&
            reg->offs[JAVACALL_CHAPI_FIELD_ACCESSES] != 0) {
        n = readArray(reg, JAVACALL_CHAPI_FIELD_ACCESSES, &buf);
        if (n < 0) {
            status = JAVACALL_FAIL;
            break;
        }
        for (ptr = buf; n > 0; ptr += test_sz, n--) {
            test_sz = *ptr++;
            if (test_sz <= caller_sz) {
				if (!javautil_wcsncmp(ptr, caller_id, test_sz)) break;
            }
        }
        if (buf) free(buf);
        if (n > 0) {
            break;
        }
    }

    return status;
}

/**
 * Opens registry file and initialize REG structure.
 */
static javacall_result regOpen(REG* reg, const char* regname, javacall_bool readOnly) {
    int ioFlag = (readOnly == JAVACALL_TRUE? JUS_O_RDONLY: JUS_O_RDWR | JUS_O_CREATE);
    reg->file = JAVAUTIL_INVALID_STORAGE_HANDLE;
    regClose(reg); // clean up

	if (regname == NULL) regname = regFilePath;

    if (regname == NULL) {
        return JAVACALL_FAIL;   // javacall_chapi_initialize() not called.
    }

	if (JAVACALL_OK != javautil_storage_open(regname,ioFlag,&reg->file)) {
		if (readOnly == JAVACALL_TRUE){
			reg->file = JAVAUTIL_INVALID_STORAGE_HANDLE;
			reg->fsize = 0;
		} else {
			return JAVACALL_IO_ERROR;
		}
	}  else {
		javautil_storage_getsize(reg->file,&reg->fsize);
		reg->cur = 0;
		reg->hsize = 0;
	}

    return JAVACALL_OK;
}

/**
 * Initializes content handler registry.
 *
 * @return JAVACALL_OK if content handler registry initialized successfully
 */
javacall_result javacall_chapi_initialize(void) {
    const char* fn = ".jsr211_reg";
	int fnlen = strlen(fn);
	int homelen,len;
	const char* home = getenv("HOME");
	if (!home) home=".";
	homelen = strlen(home);

	len = homelen + fnlen + 2;
	regFilePath = malloc(len);
	if (!regFilePath) return JAVACALL_FAIL;

	memcpy(regFilePath, home, homelen);
	regFilePath[homelen] = FILE_SEPARATOR;
	memcpy(&regFilePath[homelen+1], fn, fnlen);
	regFilePath[len] = 0;

    return JAVACALL_OK;
}

/**
 * Finalizes content handler registry.
 *
 * @return JAVACALL_OK if content handler registry finalized successfully
 */
javacall_result javacall_chapi_finalize(void) {
    if (regFilePath != NULL) {
        free(regFilePath);
        regFilePath = NULL;
    }
    return JAVACALL_OK;
}

// 3 auxiliary macros for code shrinking
#define _WRITE_CH_STRING(STR, OFF_IDX) \
    sz = writeString(&reg, STR); \
    if (sz < 0) { \
        status = JAVACALL_IO_ERROR; \
        break; \
    } \
    reg.offs[OFF_IDX] = (sz > 0? off: 0); \
    off += sz

#define _WRITE_CH_ARRAY(ARR, ARR_LEN, OFF_IDX) \
    sz = writeArray(&reg, ARR, ARR_LEN); \
    if (sz < 0) { \
        status = JAVACALL_IO_ERROR; \
        break; \
    } \
    reg.offs[OFF_IDX] = (sz > 0? off: 0); \
    off += sz

#define _WRITE_CH_INT(INT, OFF_IDX) \
		if (sizeof(int) != javautil_storage_write(reg.file, (void*)&INT, sizeof(int))) { \
            status = JAVACALL_IO_ERROR; \
            break; \
		}  \
        reg.offs[OFF_IDX] = off; \
        off += sizeof(int);



/**
 * Stores content handler information into a registry.
 *
 * @param id handler ID
 * @param suite_id suite ID
 * @param class_name handler class name
 * @param flag handler installation flag
 * @param types handler types array
 * @param nTypes length of types array
 * @param suffixes handler suffixes array
 * @param nSuffixes length of suffixes array
 * @param actions handler actions array
 * @param nActions length of actions array
 * @param locales handler locales array
 * @param nLocales length of locales array
 * @param action_names action names for every supported action 
 *                                  and every supported locale
 * @param nActionNames length of action names array. This value must be equal 
 * to @link nActions multiplied by @link nLocales .
 * @param accesses handler accesses array
 * @param nAccesses length of accesses array
 * @return operation status.
 */
javacall_result javacall_chapi_register_java_handler(
        const javacall_utf16_string id,
        int suite_id,
        const javacall_utf16_string class_name,
        int flag, 
        const javacall_utf16_string* types,     int nTypes,
        const javacall_utf16_string* suffixes,  int nSuffixes,
        const javacall_utf16_string* actions,   int nActions,
        const javacall_utf16_string* locales,   int nLocales,
        const javacall_utf16_string* action_names, int nActionNames,
        const javacall_utf16_string* accesses,  int nAccesses) {
    javacall_result status;
    int sz, off;
    REG reg;

    if (JAVACALL_OK != regOpen(&reg, 0,  JAVACALL_FALSE)) {
        return JAVACALL_FAIL;
    }

    do {
        // Set file position at end and write fake REG records.
		status = javautil_storage_lock(reg.file);
		if (status != JAVACALL_OK) break;

        off = 0;
        if ((JAVACALL_OK != javautil_storage_setpos(reg.file, 0, JUS_SEEK_END)) ||
            sizeof(int) != javautil_storage_write(reg.file, (void*)&off, sizeof(int)) ||
            sizeof(reg.offs) != javautil_storage_write(reg.file, (void*)&(reg.offs), sizeof(reg.offs))) {
            status = JAVACALL_IO_ERROR;
            break;
        }

        off = sizeof(off) + sizeof(reg.offs);
        _WRITE_CH_STRING(id, JAVACALL_CHAPI_FIELD_ID);
        if (reg.offs[JAVACALL_CHAPI_FIELD_ID] == 0) {
            status = JAVACALL_INVALID_ARGUMENT; // wrong ID
            break;
        }

		_WRITE_CH_INT(flag, JAVACALL_CHAPI_FIELD_FLAG);
        _WRITE_CH_INT(suite_id, JAVACALL_CHAPI_FIELD_SUITE);
        _WRITE_CH_STRING(class_name, JAVACALL_CHAPI_FIELD_CLASS);
        _WRITE_CH_ARRAY(types, nTypes, JAVACALL_CHAPI_FIELD_TYPES);
        _WRITE_CH_ARRAY(suffixes, nSuffixes, JAVACALL_CHAPI_FIELD_SUFFIXES);
        _WRITE_CH_ARRAY(actions, nActions, JAVACALL_CHAPI_FIELD_ACTIONS);
        _WRITE_CH_ARRAY(locales, nLocales, JAVACALL_CHAPI_FIELD_LOCALES);
        _WRITE_CH_ARRAY(action_names, nActionNames, JAVACALL_CHAPI_FIELD_ACTION_MAP);
        _WRITE_CH_ARRAY(accesses, nAccesses, JAVACALL_CHAPI_FIELD_ACCESSES);

        // actual records of the handler concerned REG data
        if (javautil_storage_setpos(reg.file, -off, JUS_SEEK_CUR) != JAVACALL_OK ||
            sizeof(int) != javautil_storage_write(reg.file, (void*)&(off), sizeof(int)) ||
            sizeof(reg.offs) != javautil_storage_write(reg.file, (void*)&(reg.offs), sizeof(reg.offs))) {
			regClose(&reg);
			javacall_chapi_unregister_handler(id);
			return JAVACALL_IO_ERROR;
        }

		status = javautil_storage_flush(reg.file);
    } while (0);

	javautil_storage_unlock(reg.file);
	regClose(&reg);


#ifdef USE_NATIVE_REGISTRY
	if (status == JAVACALL_OK){
		// register handler in native registry
		javacall_chapi_native_register_handler(
			id,
			suite_id,
			class_name,
			flag,
			types,nTypes,
			suffixes, nSuffixes,
			actions, nActions, 
			locales, nLocales,
			action_names, nActionNames,
			accesses, nAccesses);
	}
#endif

    return status;
}

/**
 * Deletes content handler information from a registry.
 *
 * @param id content handler ID
 * @return operation status.
 */
javacall_result javacall_chapi_unregister_handler(
                            const javacall_utf16_string id) {
    javacall_result status;
    REG reg;
    javacall_utf16* test_id;
    int id_sz, test_sz, found = 0, g_found = 0;
	char* buf = 0;
	int bufsz = 0;
	char tmpname[MAX_STORAGE_NAME]; 
	int tnlen = MAX_STORAGE_NAME;
	javautil_storage tmpfile;

    id_sz = javautil_wcslen(id);
    if (id_sz <= 0) {
        return JAVACALL_FAIL;
    }

	if (JAVACALL_OK != javautil_storage_gettmpname(tmpname, &tnlen) ) return JAVACALL_IO_ERROR;
	if (JAVACALL_OK != javautil_storage_open(tmpname,JUS_O_RDWR | JUS_O_CREATE, &tmpfile)) return JAVACALL_IO_ERROR;


    status = regOpen(&reg, 0, JAVACALL_TRUE);

    while ((status == JAVACALL_OK) && (status = nextHandler(&reg)) == JAVACALL_OK) {

        if (reg.hsize > bufsz) {
            bufsz = reg.hsize;
            if (buf) {
				void *tmp = realloc(buf, bufsz);
				if (!tmp) {
					status = JAVACALL_OUT_OF_MEMORY;
					break;
				}
				buf = tmp;
			} else {
				buf = malloc(bufsz);
				if (!buf) {
					status = JAVACALL_OUT_OF_MEMORY;
					break;
				}
			}
        }

		if ((reg.hsize-sizeof(int) - sizeof(reg.offs)) != javautil_storage_read(reg.file, buf, reg.hsize-sizeof(int) - sizeof(reg.offs))){
			status = JAVACALL_IO_ERROR;
			break;
		}

		test_sz = *((short*)(buf+reg.offs[JAVACALL_CHAPI_FIELD_ID]-sizeof(int) - sizeof(reg.offs)));
		test_id = (javacall_utf16_string)(buf+reg.offs[JAVACALL_CHAPI_FIELD_ID]+sizeof(short)-sizeof(int) - sizeof(reg.offs));

		found = (id_sz == test_sz  && 0 == javautil_wcsincmp(id, test_id, id_sz));
		g_found |= found;

        if (!found) {
			// copy handle
			if (sizeof(int) != javautil_storage_write(tmpfile, (void*)&reg.hsize, sizeof(int)) ||
				sizeof(reg.offs) != javautil_storage_write(tmpfile, (void*)&(reg.offs), sizeof(reg.offs)) ||
				(reg.hsize-sizeof(int) - sizeof(reg.offs)) !=  javautil_storage_write(tmpfile, buf, reg.hsize-sizeof(int) - sizeof(reg.offs))) {
				status = JAVACALL_IO_ERROR;
				break;
			}
        }
    }

	if (buf) free(buf);
    regClose(&reg);
	javautil_storage_close(tmpfile);

    if (status == JAVACALL_END_OF_FILE) {
		if (!g_found) {
			status = JAVACALL_FILE_NOT_FOUND;
		} else {
			status = JAVACALL_OK;
		}
    }

	if (status == JAVACALL_OK){
		// aquire mutex
		javautil_storage_remove(regFilePath);
		javautil_storage_rename(tmpname, regFilePath);
		// release mutex
	}


#ifdef USE_NATIVE_REGISTRY
	if (status == JAVACALL_OK){
		javacall_chapi_native_unregister_handler(id);
	}
#endif

    return status;
}

/**
 * Searches content handler using specified key and value.
 *
 * @param caller_id calling application identifier
 * @param key search field id. Valid keys are: <ul> 
 *   <li>JAVACALL_CHAPI_FIELD_TYPES, <li>JAVACALL_CHAPI_FIELD_SUFFIXES, 
 *   <li>JAVACALL_CHAPI_FIELD_ACTIONS. </ul>
 * The special case of JAVACALL_CHAPI_FIELD_ID is used for testing new handler ID.
 * @param value search value
 * @param result the buffer for Content Handlers result array. 
 *  <br>Use the @link javautil_chapi_appendHandler() javautil_chapi_appendHandler function to fill this structure.
 * @return status of the operation
 */
javacall_result javacall_chapi_find_handler(
        const javacall_utf16_string caller_id,
        javacall_chapi_field key,
        const javacall_utf16_string value,
        /*OUT*/ javacall_chapi_result_CH_array result) {
    javacall_result status;
    REG reg;
    javacall_utf16 *id = NULL;
    int suit = 0;
    javacall_utf16 *clas = NULL;
    int id_sz, clas_sz, flag = -1;
    int n, sz;
    int mode;
    int value_sz;
    javacall_utf16 *buf, *ptr;

    switch (key) {
        case JAVACALL_CHAPI_FIELD_TYPES:
        case JAVACALL_CHAPI_FIELD_SUFFIXES:
            mode = 1;   // case-insensitive
            break;
        case JAVACALL_CHAPI_FIELD_ID:
        case JAVACALL_CHAPI_FIELD_ACTIONS:
            mode = 0;   // case-sensitive
            break;
        default:
            return JAVACALL_INVALID_ARGUMENT; // wrong parameter 'key'
    }

    if (JAVACALL_OK != regOpen(&reg, 0, JAVACALL_TRUE)) {
        return JAVACALL_FAIL;
    }

	value_sz = javautil_wcslen(value);
    do {
        status = nextAccessedHandler(&reg, caller_id);
        if (JAVACALL_OK != status) {
            break;
        }

        if (reg.offs[key] == 0) {
            continue;
        }

        if (key == JAVACALL_CHAPI_FIELD_ID) {
            sz = readString(&reg, JAVACALL_CHAPI_FIELD_ID, &buf);
            if (sz < 0) {
                status = JAVACALL_FAIL;
                break;
            }
            if (sz > value_sz) {
                sz = value_sz;
            }
			n = javautil_wcsncmp(buf, value, sz) ? 0 : 1;
        } else {
            n = readArray(&reg, key, &buf);
            if (n < 0) {
                status = JAVACALL_FAIL;
                break;
            }
    
            for (ptr = buf; n > 0; ptr += sz, n--) {
                sz = *ptr++;
                if (sz == value_sz) {
					int cmp = 1;
					if (mode == 0){
						cmp = javautil_wcsincmp(ptr, value, sz);
					} else {
						cmp = javautil_wcsncmp(ptr, value, sz);
					}
                    if (!cmp) break;
                }
            }
        }

        free(buf);

        if (n > 0) {
            status = loadHandler(&reg, &id, &id_sz, &suit, 
                                                    &clas, &clas_sz, &flag);
            if (status == JAVACALL_OK) {
                status = javautil_chapi_appendHandler(id, id_sz, 
                                suit, clas, clas_sz, flag, result);
            }
            CLEAN_HANDLER(id, suit, clas, flag);
        }
    } while (status == JAVACALL_OK);

    regClose(&reg);
    if (status == JAVACALL_END_OF_FILE) {
        status = JAVACALL_OK;
    }

#ifdef USE_NATIVE_REGISTRY
	if (status == JAVACALL_OK){
		javacall_chapi_native_find_handler(caller_id,key,value,result);
	}
#endif

    return status;
}
                        
/**
 * Fetches handlers registered for the given suite.
 *
 * @param suite_id requested suite Id.
 * @param result the buffer for Content Handlers result array. 
 *  <br>Use the @link javautil_chapi_appendHandler() or 
 * @link javautil_chapi_appendHandler function to fill this structure.
 * @return status of the operation
 */
javacall_result javacall_chapi_find_for_suite(
                        int suite_id,
                        /*OUT*/ javacall_chapi_result_CH_array result) {
    javacall_result status;
    REG reg;
    javacall_utf16 *id = NULL;
    int suit = 0;
    javacall_utf16 *clas = NULL;
    int id_sz, clas_sz, flag;

    if (JAVACALL_OK != regOpen(&reg,  0, JAVACALL_TRUE)) {
        return JAVACALL_FAIL;
    }

    do {
        status = nextHandler(&reg);
        if (JAVACALL_OK != status) {
            break;
        }
		suit = readInt(&reg, JAVACALL_CHAPI_FIELD_SUITE);
		flag = (suit == suite_id);

        if (flag) {
            status = loadHandler(&reg, &id, &id_sz, &suit, 
                                                &clas, &clas_sz, &flag);
            if (status == JAVACALL_OK) {
                status = javautil_chapi_appendHandler(id, id_sz, suit, 
                                                clas, clas_sz, flag, result);
            }
            CLEAN_HANDLER(id, suit, clas, flag);
        }
    } while (JAVACALL_OK == status);

    regClose(&reg);
    if (status == JAVACALL_END_OF_FILE) {
        status = JAVACALL_OK;
    }

#ifdef USE_NATIVE_REGISTRY
	if (status == JAVACALL_OK){
		javacall_chapi_native_find_for_suite(suite_id,result);
	}
#endif

    return status;
}

/**
 * Searches content handler using content URL. This function MUST define
 * content type and return default handler for this type if any.
 *
 * @param caller_id calling application identifier
 * @param url content URL
 * @param action requested action
 * @param handler output parameter - the handler conformed with requested URL 
 * and action.
 *  <br>Use the @link javautil_chapi_fillHandler() javautil_chapi_fillHandler function to fill this structure.
 * @return status of the operation
 */
javacall_result javacall_chapi_handler_by_URL(
        const javacall_utf16_string caller_id,
        const javacall_utf16_string url,
        const javacall_utf16_string action,
        /*OUT*/ javacall_chapi_result_CH handler) {
		javacall_result status = JAVACALL_NOT_IMPLEMENTED;
#ifdef USE_NATIVE_REGISTRY
	status = javacall_chapi_native_handler_by_URL(caller_id, url, action, handler);
#endif
    return status;
}

/**
 * Returns all found values for specified field. Tha allowed fields are: <ul>
 *    <li> JAVACALL_CHAPI_FIELD_ID, <li> JAVACALL_CHAPI_FIELD_TYPES, <li> JAVACALL_CHAPI_FIELD_SUFFIXES,
 *    <li> and JAVACALL_CHAPI_FIELD_ACTIONS. </ul>
 * Values should be selected only from handlers accessible for given caller_id.
 *
 * @param caller_id calling application identifier.
 * @param field search field id
 * @param result output structure where result is placed to.
 *  <br>Use the @link javautil_chapi_appendString() javautil_chapi_appendString function to fill this structure.
 * @return status of the operation
 */
javacall_result javacall_chapi_get_all(
        const javacall_utf16_string caller_id,
        javacall_chapi_field field, 
        /*OUT*/ javacall_chapi_result_str_array result) {
    javacall_result status;
    int mode; // string/array field requested by 'field' parameter
    REG reg;
    javacall_utf16 *ptr, *str = NULL;
    int n, sz;

    switch (field) {
        case JAVACALL_CHAPI_FIELD_ID:
            mode = -1;   // string field requested
            break;
        case JAVACALL_CHAPI_FIELD_TYPES:
        case JAVACALL_CHAPI_FIELD_SUFFIXES:
            mode = 0;   // array field, case-insensitive
            break;
        case JAVACALL_CHAPI_FIELD_ACTIONS:
            mode = 1;   // array field, case-sensitive
            break;
        default:
            return JAVACALL_INVALID_ARGUMENT;
    }

    if (JAVACALL_OK != regOpen(&reg,  0, JAVACALL_TRUE)) {
        return JAVACALL_FAIL;
    }

    do {
        status = nextAccessedHandler(&reg, caller_id);
        if (JAVACALL_OK != status) {
            break;
        }

        if (reg.offs[field] == 0) {
            continue;
        }

        if (mode < 0) {
            sz = readString(&reg, field, &str);
            if (sz > 0) {
                status = javautil_chapi_appendUniqueString(str, sz,JAVACALL_TRUE, result);
            }
        } else {
            n = readArray(&reg, field, &str);
            if (n < 0) {
                status = JAVACALL_FAIL;
                break;
            }

            for (ptr = str; n-- > 0 && status == JAVACALL_OK; ptr += sz) {
                sz = *ptr++;
                status = javautil_chapi_appendUniqueString(ptr, sz, mode > 0 ? JAVACALL_TRUE: JAVACALL_FALSE, result);
            }
        }
        if (str != NULL) {
            free(str);
        }
    } while (status == JAVACALL_OK);

    regClose(&reg);
    if (status == JAVACALL_END_OF_FILE) {
        status = JAVACALL_OK;
    }

#ifdef USE_NATIVE_REGISTRY
	if ( status == JAVACALL_OK){
		javacall_chapi_native_get_all(caller_id,field, result);
	}
#endif

    return status;
}

/**
 * Gets the registered content handler for the ID.
 * The query can be for an exact match or for the handler
 * matching the prefix of the requested ID.
 *  <BR>Only a content handler which is visible to and accessible to the 
 * given @link caller_id should be returned.
 *
 * @param caller_id calling application identifier.
 * @param id handler ID.
 * @param flag indicating whether exact or prefixed search mode should be 
 * performed.
 * @param handler output value - requested handler.
 *  <br>Use the @link javautil_chapi_fillHandler() javautil_chapi_fillHandler function to fill this structure.
 * @return status of the operation
 */
javacall_result javacall_chapi_get_handler(
        const javacall_utf16_string caller_id,
        const javacall_utf16_string id,
        javacall_chapi_search_flag mode,
        /*OUT*/ javacall_chapi_result_CH result) {
    javacall_result status;
    REG reg;
    javacall_utf16 *id_ = NULL;
    int suit;
    javacall_utf16 *clas = NULL;
    int id_sz, clas_sz, flag = -1;
    javacall_utf16 *test = NULL;
    int test_sz, found = JAVACALL_FALSE;

	id_sz = javautil_wcslen(id);

    if (id_sz <= 0) {
        return JAVACALL_FAIL;
    }

    if (JAVACALL_OK != regOpen(&reg,  0, JAVACALL_TRUE)) {
        return JAVACALL_FAIL;
    }

    do {
        status = nextAccessedHandler(&reg, caller_id);
        if (JAVACALL_OK != status) {
            break;
        }

        test_sz = readString(&reg, JAVACALL_CHAPI_FIELD_ID, &test);
        if (test_sz <= 0) {
            status = JAVACALL_FAIL;
            break;
        }

        if (test_sz == id_sz ||
            (mode == JAVACALL_CHAPI_SEARCH_PREFIX && test_sz < id_sz)) {
            if (javautil_wcsincmp(id, test, test_sz) == 0) {
                status = loadHandler(&reg, &id_, &id_sz, &suit,
                                                    &clas, &clas_sz, &flag);
                if (status == JAVACALL_OK) {
                    status = javautil_chapi_fillHandler(id_, id_sz, 
                                suit, clas, clas_sz, flag, result);
                }
                CLEAN_HANDLER(id_, suit, clas, flag);
                found = JAVACALL_TRUE;
            }
        }

        free(test);
    } while (found == JAVACALL_FALSE);

    regClose(&reg);

	    
	if (status != JAVACALL_OK && status !=JAVACALL_END_OF_FILE) {
		return status;
	}

#ifdef USE_NATIVE_REGISTRY
	if (JAVACALL_OK == javacall_chapi_native_get_handler(caller_id,
        id,
        mode,
        result)){
		return JAVACALL_OK;
	}
#endif

    return status;
}

/**
 * Loads the handler's data field. Allowed fields are: <UL>
 *  <LI> JAVACALL_CHAPI_FIELD_TYPES, <LI> JAVACALL_CHAPI_FIELD_SUFFIXES, 
 *  <LI> JAVACALL_CHAPI_FIELD_ACTIONS, <LI> JAVACALL_CHAPI_FIELD_LOCALES, 
 *  <LI> JAVACALL_CHAPI_FIELD_ACTION_MAP, <LI> and JAVACALL_CHAPI_FIELD_ACCESSES. </UL>
 *
 * @param id requested handler ID.
 * @param field_id requested field.
 * @param result output structure where requested array is placed to.
 *  <br>Use the @link javautil_chapi_appendString() javautil_chapi_appendString function to fill this structure.
 * @return status of the operation
 */
javacall_result javacall_chapi_get_handler_field(
        const javacall_utf16_string id,
        javacall_chapi_field key, 
        /*OUT*/ javacall_chapi_result_str_array result) {
    javacall_result status;
    REG reg;
    javacall_utf16 *buf = NULL, *ptr;
    int sz, n, found;
    int id_sz;

	id_sz = javautil_wcslen(id);

    if (id_sz <= 0) {
        return JAVACALL_FAIL;
    }
    
    if (JAVACALL_OK != regOpen(&reg,  0, JAVACALL_TRUE)) {
        return JAVACALL_FAIL;
    }

    while ((status = nextHandler(&reg)) == JAVACALL_OK) {
        sz = readString(&reg, JAVACALL_CHAPI_FIELD_ID, &buf);
		if (sz == id_sz){
			found = (javautil_wcsincmp(id, buf, sz) == 0);
		}

        free(buf);
        if (found) {
            if (reg.offs[key] == 0) {
                break;
            }
            buf = NULL;
            n = readArray(&reg, key, &buf);
            if (n < 0) {
                status = JAVACALL_FAIL;
                break;
            }
            for (ptr = buf; n-- > 0 && status == JAVACALL_OK; ptr += sz) {
                sz = *ptr++;
                status = javautil_chapi_appendString(ptr, sz, result);
            }
            free(buf);
            break;
        }
    }

    regClose(&reg);

#ifdef USE_NATIVE_REGISTRY
    if (status == JAVACALL_END_OF_FILE) {
		status = javacall_chapi_native_get_handler_field(
        id,key,result);
		if (status != JAVACALL_OK) status = JAVACALL_FILE_NOT_FOUND;
    }
#endif

    return status;
}

/**
 * Transforms javacall_utf16 string to wchar_t zero-terminated string.
 * @param str transformated string
 * @param str_size transformated string size
 * @return the result null-terminated string or NULL if no memory.
 */
static wchar_t* strzdup(const javacall_utf16* str, int str_size) {
    wchar_t* res;

    res = (wchar_t*)malloc((str_size + 1) * sizeof(wchar_t));
    if (res != NULL) {
        memcpy(res, str, str_size * sizeof(wchar_t));
        res[str_size] = 0;
    }

    return res;
}

/**
 * Attention! Win32 specific implementation!
 * Executes specified non-java content handler.
 * @param id content handler ID
 * @param invoc invocation parameters
 * @param exec_status handler execution status:
 *  <ul>
 *  <li> 0  - handler is succefully launched,
 *  <li> 1  - handler will be launched after JVM exits.
 *  </ul>
 *
 * @return status of the operation
 */
javacall_result javacall_chapi_execute_handler(
            const javacall_utf16_string id, 
            javacall_chapi_invocation* invoc, 
            /*OUT*/ javacall_chapi_launch_result* exec_status) {

#ifdef USE_NATIVE_REGISTRY
	return javacall_chapi_native_execute_handler(id,invoc,exec_status);
#else
	return JAVACALL_NOT_IMPLEMENTED;
#endif
}