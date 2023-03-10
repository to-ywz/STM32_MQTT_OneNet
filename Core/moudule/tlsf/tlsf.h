/*
 * Two Levels Segregate Fit memory allocator (TLSF)
 * Version 2.4.6
 *
 * Written by Miguel Masmano Tello <mimastel@doctor.upv.es>
 *
 * Thanks to Ismael Ripoll for his suggestions and reviews
 *
 * Copyright (C) 2008, 2007, 2006, 2005, 2004
 *
 * This code is released using a dual license strategy: GPL/LGPL
 * You can choose the licence that better fits your requirements.
 *
 * Released under the terms of the GNU General Public License Version 2.0
 * Released under the terms of the GNU Lesser General Public License Version 2.1
 *
 */

#ifndef _TLSF_H_
#define _TLSF_H_

typedef unsigned int size_tlsf_t;

extern size_tlsf_t init_memory_pool(size_tlsf_t, void *);
extern size_tlsf_t get_used_size(void *);
extern size_tlsf_t get_max_size(void *);
extern void destroy_memory_pool(void *);
extern size_tlsf_t add_new_area(void *, size_tlsf_t, void *);
extern void *malloc_ex(size_tlsf_t, void *);
extern void free_ex(void *, void *);
extern void *realloc_ex(void *, size_tlsf_t, void *);
extern void *calloc_ex(size_tlsf_t, size_tlsf_t, void *);

extern void *tlsf_malloc(size_tlsf_t size);
extern void tlsf_free(void *ptr);
extern void *tlsf_realloc(void *ptr, size_tlsf_t size);
extern void *tlsf_calloc(size_tlsf_t nelem, size_tlsf_t elem_size);

#endif
