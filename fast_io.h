/*
 * Fast_IO (pre-release beta) Extension for PHP 8
 * https://github.com/commeta/fast_io
 * 
 * Copyright 2024 commeta <dcs-spb@ya.ru>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


extern zend_module_entry fast_io_module_entry;
#define phpext_fast_io_ptr &fast_io_module_entry

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(fast_io)
    zend_long buffer_size;
ZEND_END_MODULE_GLOBALS(fast_io)

#ifdef ZTS
#define FAST_IO_G(v) TSRMG(fast_io_globals_id, zend_fast_io_globals *, v)
#else
#define FAST_IO_G(v) (fast_io_globals.v)
#endif

#define SPECIAL_CHAR 127
