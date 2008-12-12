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

#ifndef __JAVACALL_DOM_HTMLINPUTELEMENT_H_
#define __JAVACALL_DOM_HTMLINPUTELEMENT_H_

/**
 * @file javacall_dom_htmlinputelement.h
 * @ingroup JSR290DOM
 * @brief Javacall DOM interfaces for HTMLInputElement
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <javacall_dom.h>

/**
 * @defgroup JSR290DOM JSR290 DOM API
 *
 * The following API definitions are required by DOM part of the JSR-290.
 *
 * @{
 */

/**
 * Returns when the <code>type</code> attribute of the element has the value 
 * "radio" or "checkbox", this represents the current state of the form 
 * control, in an interactive user agent. Changes to this attribute 
 * change the state of the form control, but do not change the value of 
 * the HTML checked attribute of the INPUT element.During the handling 
 * of a click event on an input element with a type attribute that has 
 * the value "radio" or "checkbox", some implementations may change the 
 * value of this property before the event is being dispatched in the 
 * document. If the default action of the event is canceled, the value 
 * of the property may be changed back to its original value. This means 
 * that the value of this property during the handling of click events 
 * is implementation dependent.
 * 
 * @param handle Pointer to the object representing this htmlinputelement.
 * 
 * @return JAVACALL_OK if all done successfuly,
 *         JAVACALL_NOT_IMPLEMENTED when the stub was called
 */
javacall_result
javacall_dom_htmlinputelement_get_checked(javacall_handle handle,
                                          /* OUT */ javacall_bool* ret_value);

/**
 * Returns when the <code>type</code> attribute of the element has the value 
 * "text", "file" or "password", this represents the current contents of 
 * the corresponding form control, in an interactive user agent. 
 * Changing this attribute changes the contents of the form control, but 
 * does not change the value of the HTML value attribute of the element. 
 * When the <code>type</code> attribute of the element has the value 
 * "button", "hidden", "submit", "reset", "image", "checkbox" or 
 * "radio", this represents the HTML value attribute of the element. See 
 * the value attribute definition in HTML 4.01.
 * 
 * @param handle Pointer to the object representing this htmlinputelement.
 * 
 * @return JAVACALL_OK if all done successfuly,
 *         JAVACALL_OUT_OF_MEMORY if length of the returend string is more then 
 *                                specified in ret_value_len,
 *         JAVACALL_NOT_IMPLEMENTED when the stub was called
 */
javacall_result
javacall_dom_htmlinputelement_get_value(javacall_handle handle,
                                        /* OUT */ javacall_utf16_string ret_value,
                                        /* INOUT */ javacall_uint32* ret_value_len);

/**
 * Select the contents of the text area. For <code>INPUT</code> elements 
 * whose <code>type</code> attribute has one of the following values: 
 * "text", "file", or "password".
 * 
 * @param handle Pointer to the object representing this htmlinputelement.
 * 
 * @return JAVACALL_OK if all done successfuly,
 *         JAVACALL_NOT_IMPLEMENTED when the stub was called
 */
javacall_result
javacall_dom_htmlinputelement_select(javacall_handle handle);

/**
  * Sets when the <code>type</code> attribute of the element has the value 
  * "radio" or "checkbox", this represents the current state of the form 
  * control, in an interactive user agent. Changes to this attribute 
  * change the state of the form control, but do not change the value of 
  * the HTML checked attribute of the INPUT element.During the handling 
  * of a click event on an input element with a type attribute that has 
  * the value "radio" or "checkbox", some implementations may change the 
  * value of this property before the event is being dispatched in the 
  * document. If the default action of the event is canceled, the value 
  * of the property may be changed back to its original value. This means 
  * that the value of this property during the handling of click events 
  * is implementation dependent.
  * 
 * @param handle Pointer to the object representing this htmlinputelement.
 * 
 * @return JAVACALL_OK if all done successfuly,
 *         JAVACALL_NOT_IMPLEMENTED when the stub was called
 */
javacall_result
javacall_dom_htmlinputelement_set_checked(javacall_handle handle,
                                          javacall_bool checked);

/**
 * Sets when the <code>type</code> attribute of the element has the value 
 * "text", "file" or "password", this represents the current contents of 
 * the corresponding form control, in an interactive user agent. 
 * Changing this attribute changes the contents of the form control, but 
 * does not change the value of the HTML value attribute of the element. 
 * When the <code>type</code> attribute of the element has the value 
 * "button", "hidden", "submit", "reset", "image", "checkbox" or 
 * "radio", this represents the HTML value attribute of the element. See 
 * the value attribute definition in HTML 4.01.
 * 
 * @param handle Pointer to the object representing this htmlinputelement.
 * 
 * @return JAVACALL_OK if all done successfuly,
 *         JAVACALL_NOT_IMPLEMENTED when the stub was called
 */
javacall_result
javacall_dom_htmlinputelement_set_value(javacall_handle handle,
                                        javacall_const_utf16_string value);

/** 
 * Decrements ref counter of the native object specified number of times
 * 
 * @param handle Pointer to the object representing this node.
 * @param count number of times to decrement.
 * 
 * @return JAVACALL_OK if all done successfuly,
 *         JAVACALL_NOT_IMPLEMENTED when the stub was called
 */
javacall_result
javacall_dom_htmlinputelement_clear_references(javacall_handle handle, javacall_int32 count);


/** @} */

#ifdef __cplusplus
}
#endif

#endif /* ifndef __JAVACALL_DOM_HTMLINPUTELEMENT_H_ */