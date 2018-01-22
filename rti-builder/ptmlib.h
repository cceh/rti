/*
 * A library of PTM routines.
 *
 * Author: Marcello Perathoner <marcello@perathoner.de>
 *
 * License: GPL3
 */

#ifndef PTMLIB_H
#define PTMLIB_H

#include <stdio.h>            /* size_t, FILE */

#include "jpeglib.h"          /* JSAMPLE */

#define PTM_COEFFICIENTS      6
#define RGB_COEFFICIENTS      3
#define CBCR_COEFFICIENTS     2
#define MAX_PTM_BLOCKS        3
#define MAX_COLOR_COMPONENTS  RGB_COEFFICIENTS
#define MAX_JPEG_STREAMS      RGB_COEFFICIENTS * PTM_COEFFICIENTS

typedef struct {
    /* little-endian cu2 first */
    JSAMPLE cu2;
    JSAMPLE cv2;
    JSAMPLE cuv;
    JSAMPLE cu;
    JSAMPLE cv;
    JSAMPLE c1;
} ptm_coefficients_t;

typedef struct {
    float cu2;
    float cv2;
    float cuv;
    float cu;
    float cv;
    float c1;
} ptm_unscaled_coefficients_t;

typedef struct {
    JSAMPLE r;
    JSAMPLE g;
    JSAMPLE b;
} rgb_coefficients_t;

typedef struct {
    JSAMPLE cb;
    JSAMPLE cr;
} cbcr_coefficients_t;

typedef struct {
    int id;               /* internal format id */
    int blocks;           /* no. of blocks to allocate */
    int ptm_blocks;       /* no. of PTM coefficient blocks, either 1 or 3 */
    int color_components; /* no. of color channels that are not encoded with
                             PTM.  Either 0 = RGB, 2 = LUM, or 3 = LRGB */
    int jpeg_streams;     /* no. of JPEG streams (if file is compressed else 0)*/
    const char *name;
} ptm_format_t;

const ptm_format_t ptm_formats[6];  /* the PTM file formats we read */

typedef struct {
    const ptm_format_t *format; /* the file format */
    size_t dimen           [2];  /* w, h image dimensions */
    float scale            [PTM_COEFFICIENTS];
    int bias               [PTM_COEFFICIENTS];
    /* The following are used only for compressed PTMs */
    int compression_param  [1];
    int transforms         [MAX_JPEG_STREAMS];
    int motion_vector_x    [MAX_JPEG_STREAMS];
    int motion_vector_y    [MAX_JPEG_STREAMS];
    int order              [MAX_JPEG_STREAMS];
    int reference_planes   [MAX_JPEG_STREAMS];
    size_t compressed_size [MAX_JPEG_STREAMS];
    size_t side_info_sizes [MAX_JPEG_STREAMS];
    /* auxiliary fields not used in the PTM file */
    ptm_unscaled_coefficients_t min_coefficients;
    ptm_unscaled_coefficients_t max_coefficients;
    float inv_scale        [PTM_COEFFICIENTS];
} ptm_header_t;

typedef JSAMPLE *ptm_block_t;

typedef struct {
    FILE *fp;
    char *filename;
    float u, v, w;
    struct jpeg_decompress_struct dinfo;
} decoder_t;

ptm_header_t *ptm_alloc_header ();
ptm_block_t *ptm_alloc_blocks (ptm_header_t *ptm);
void ptm_free_blocks (ptm_header_t *ptm, ptm_block_t *blocks);

ptm_header_t *ptm_read_header (FILE *fp);
void ptm_write_header (FILE *fp, ptm_header_t *ptm);

void ptm_read_ptm  (FILE *fp, ptm_header_t *ptm, ptm_block_t *blocks);
void ptm_write_ptm (FILE *fp, ptm_header_t *ptm, ptm_block_t *blocks);

void ptm_write_jpeg (FILE *fp, ptm_header_t *ptm, ptm_block_t *blocks, float u, float v);

void ptm_print_matrix (const char *name, float* M, int m, int n);
float *ptm_svd (decoder_t **decoders, int n_decoders);

void ptm_fit_poly_rgb (ptm_header_t *ptm_header,
                       const JSAMPLE *buffer,
                       size_t n_decoders,
                       float *M, ptm_unscaled_coefficients_t *block);

void ptm_calc_scale (ptm_header_t *ptm_header);
void ptm_scale_coefficients (ptm_header_t *ptm_header,
                             ptm_block_t *scaled,
                             ptm_unscaled_coefficients_t *unscaled);


#endif
