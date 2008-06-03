/*
* Copyright  1990-2008 Sun Microsystems, Inc. All Rights Reserved.
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

#include <javacall_dom.h>

javacall_result
javacall_dom_domimplementation_has_feature(javacall_handle handle,
                                           javacall_const_utf16_string feature,
                                           javacall_const_utf16_string version,
                                           /* OUT */ javacall_bool* retValue);

javacall_result
javacall_dom_domimplementation_create_document_type(javacall_handle handle,
                                                    javacall_const_utf16_string qualifiedName,
                                                    javacall_const_utf16_string publicId,
                                                    javacall_const_utf16_string systemId,
                                                    /* OUT */ javacall_handle* retValue,
                                                    /* OUT */ javacall_utf8_string exceptionValue);

javacall_result
javacall_dom_domimplementation_create_document(javacall_handle handle,
                                               javacall_const_utf16_string namespaceURI,
                                               javacall_const_utf16_string qualifiedName,
                                               javacall_handle doctype,
                                               /* OUT */ javacall_handle* retValue,
                                               /* OUT */ javacall_utf8_string exceptionValue);

javacall_result
javacall_dom_domimplementation_get_feature(javacall_handle handle,
                                           javacall_const_utf16_string feature,
                                           javacall_const_utf16_string version,
                                           /* OUT */ javacall_handle* retValue);

