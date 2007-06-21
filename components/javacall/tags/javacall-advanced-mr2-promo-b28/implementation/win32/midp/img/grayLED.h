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

/* This image of a gray LED is the unlit network indicator light shown
 * on the phone skin when no network activity is underway */

struct _PackedGrayDib {
    BITMAPINFOHEADER hdr;
    DWORD            info[300];
} _grayLED_dib_data = {
    {
        sizeof(BITMAPINFOHEADER), /* biSize */
        20,			  /* biWidth */
        20,			  /* biHeight */
        1,			  /* biPlanes */
        24,			  /* biBitCount */
        0,			  /* biCompression */
        1200,		  	  /* biSizeImage */
        3780,			  /* biXPelsPerMeter */
        3780,			  /* biYPelsPerMeter */
        0,			  /* biClrUsed */
        0,			  /* biClrImportant */
    }, {
	0xac5147ac, 0x47aa5447, 0x4f45a952, 0xa94c44a7, 
	0x46aa4c45, 0x4c43a64e, 0xa24b43a7, 0x3fa2493e, 
	0x493fa148, 0xa25449ab, 0x43a74a40, 0x5146ac4b, 
	0xaa4e46ac, 0x41a35145, 0x443b974c, 0xa74e45ab, 
	0x43a94b41, 0x4e43aa4c, 0xa64e43a8, 0x40a34c41, 
	0x4c42a44d, 0xa84942a6, 0x46aa4b42, 0x4d46a84d, 
	0xa84d47a8, 0x41a34e47, 0x453ea147, 0xa1443ea1, 
	0x43a4473e, 0x4d45a84c, 0xaa4d45af, 0x3ea44e42, 
	0x4a3ea34a, 0xa64b40a5, 0x46ab4940, 0x4e47a94f, 
	0xac4a44a7, 0x41a4524a, 0x5349aa48, 0xab544aaa, 
	0x4aab5349, 0x534aab53, 0xa85149aa, 0x48a75347, 
	0x5146a753, 0xaa463ca6, 0x45ae4d41, 0x4c43a94f, 
	0xa94c45ab, 0x41a44d45, 0x5349ab4b, 0xaa4f46a8, 
	0x48a95348, 0x5149ab4f, 0xad544bab, 0x4bac564c, 
	0x544aab55, 0xa95149ab, 0x48a85347, 0x5347a554, 
	0xa84842ac, 0x46ac4940, 0x5145aa50, 0xb05046ac, 
	0x5ab2554d, 0x7a72bc65, 0xca8984c6, 0x92cc948d, 
	0x9c97cf97, 0xc59a95cd, 0x49ab8a82, 0x5349ab53, 
	0xa85149aa, 0x48a85348, 0x5348a654, 0xac5045ac, 
	0x48ae5245, 0x5347ad54, 0xa84e44a4, 0x53964d44, 
	0x3c563d5a, 0x2c1f2b1f, 0x332b2938, 0x18231829, 
	0xd52b332f, 0x89c9aca8, 0x4d47aa8d, 0xa84b47ab, 
	0x41a24f47, 0x4e45a34a, 0xae5148b0, 0x49ae5448, 
	0x5448ae53, 0x7b443a8e, 0x4333494c, 0x4f714f33, 
	0x4e415c41, 0x64574963, 0x2a3b2a53, 0x1b161f16, 
	0xa4d31b20, 0x837cbfa9, 0xa14a42a4, 0x44a64c41, 
	0x5046a850, 0xac5148b0, 0x45aa5245, 0x4c43a54f, 
	0x4c2e2969, 0x815e4c5d, 0x6691665e, 0x6c6d9b6f, 
	0x8f646c99, 0x6b856f64, 0x153c4c3d, 0x5157151e, 
	0x958dc94b, 0x9e4a3fa1, 0x41a24a3f, 0x4d40a04d, 
	0xab4e45ad, 0x4bb04d44, 0x41398e53, 0x5429274f, 
	0x5d4f5454, 0xd2eedc4f, 0x7e486248, 0xc7aa7eb4, 
	0x7da7839e, 0x2868816c, 0x23192738, 0x9994cd19, 
	0xa54c44a8, 0x47a74d44, 0x5148a84e, 0xb04f48b1, 
	0x49af5348, 0x3d348053, 0x682e2d48, 0xffff6868, 
	0xdcf3e5fa, 0x3ec4e4ce, 0xb8813e50, 0x7baf7f81, 
	0x39628c62, 0x2b1f3951, 0x9891cb1f, 0xa04e45a5, 
	0x46a34e44, 0x584fab4f, 0xb04f48b1, 0x49af5348, 
	0x40388453, 0x7d2d2d43, 0xffff7d83, 0xe0f6e9fa, 
	0x92ffffff, 0x554192c4, 0x7eb47e41, 0x446b986b, 
	0x2e224460, 0x928bc721, 0xa7564ba8, 0x4aa5564a, 
	0x5449a555, 0xad4c44ad, 0x47ad4f45, 0x4a409951, 
	0x831c1a30, 0x5f5c8383, 0xc2e3cd5c, 0x6d56745a, 
	0x674c6d98, 0x7fb57f4c, 0x606d9b6d, 0x49355b76, 
	0x857dc135, 0xa7554ba9, 0x4ba5564b, 0x5449a555, 
	0xae423da7, 0x4eb54e46, 0x5249ac56, 0xb7100e23, 
	0x827fb7c6, 0x4d4e4d7f, 0x42393e39, 0x84634252, 
	0x79aa7963, 0x3271a071, 0x63813243, 0x7269b65c, 
	0xa6534aa8, 0x4ba5554a, 0x5449a655, 0xa94f47b4, 
	0x3fa7473f, 0x4c45ae46, 0x1f23204e, 0xd8c41313, 
	0x8d988dc4, 0x48626762, 0x574f4848, 0x80ac804f, 
	0x572b352b, 0x4da9506c, 0x554da655, 0x9f433d9c, 
	0x50aa4b42, 0x4d44a157, 0xa6433ba5, 0x41aa473c, 
	0x4c3fa84d, 0x4a483da1, 0x0815201c, 0xc5dcc50a, 
	0x79939b93, 0x6c69797c, 0x51555169, 0x8e485463, 
	0x43a44138, 0x5248aa4c, 0xa7554db0, 0x45a35248, 
	0x4e46a54d, 0xb14a41ad, 0x47b15145, 0x4b41aa51, 
	0xa7493ea5, 0x21544c3f, 0x110f2528, 0x31120f29, 
	0x173d1512, 0x2821551a, 0x8f392f74, 0x43a4473b, 
	0x4e41a151, 0x9e453d9c, 0x43a14b40, 0x5147a550, 
	0xa54944ae, 0x3ea9423b, 0x5047b146, 0xae5049b3, 
	0x3ea44d46, 0x463fa447, 0xb44e46ae, 0x4cb4574d, 
	0x5147ac55, 0xa04a41a4, 0x3ea0463d, 0x473e9e47, 
	0xa25047a8, 0x419f4e44, 0x4d409c4d, 0xaa4b43ad, 
	0x41a94a40, 0x4a40a74a, 0xb45048af, 0x48ad544e, 
	0x4841a552, 0x9f423aa0, 0x41a6433b, 0x534bae48, 
	0xb85a53b5, 0x51b25c55, 0x534cab58, 0xab453f9e, 
	0x46a5544b, 0x4e43a04f, 0xb04f47b0, 0x48b05347, 
	0x5347ae54, 0xac5146ac, 0x47ac4f46, 0x5449ac53, 
	0xab5349ae, 0x48ab5348, 0x5448aa51, 0xa95348a9, 
	0x47a65247, 0x5046a550, 0xa84b43a2, 0x47a3544b, 
	0x5048a54f, 0xaf5146af, 0x46af5246, 0x5146ae53, 
	0xab5245ac, 0x45aa5045, 0x5247a951, 0xaa5146ab, 
	0x48ab5347, 0x554aab52, 0xad564bab, 0x4bad574c, 
	0x554cab55, 0xa6544ba9, 0x4ba5564a, 0x534aa556,     }
};
BITMAPINFOHEADER *grayLED_dib = &(_grayLED_dib_data.hdr);
