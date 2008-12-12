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
#ifndef __JAVACALL_ODD_H_
#define __JAVACALL_ODD_H_

/**
 * @file javacall_odd.h
 * @ingroup OnDeviceDebug
 * @brief Javacall interfaces for On Device Debug
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "javacall_defs.h"

/**
 * @defgroup OnDeviceDebug API
 * @ingroup JTWI
 *
 * OnDeviceDebug APIs define the functionality for:
 * 
 * - Enabling the ODTAgent midlet
 * 
 *  @{
 */

/**
 * The platform calls this function to inform VM that
 * ODTAgent midlet must be enabled.
 */
void javanotify_enable_odd(void);

javacall_result javacall_odt_initialize();

javacall_result javacall_odt_open_channel(int port, void **pHandle);

javacall_result javacall_odt_close_channel(javacall_handle handle);

javacall_result javacall_odt_is_available(javacall_handle handle, int *pBytesAvailable);

javacall_result javacall_odt_write_bytes(javacall_handle handle, char *pData, int len, int *pBytesWritten);

javacall_result javacall_odt_read_bytes(javacall_handle handle, unsigned char *pData, int len, int *pBytesRead);


/** @} */

#ifdef __cplusplus
}
#endif

#endif
