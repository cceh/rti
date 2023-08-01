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

/** The no. of coefficients used by PTM to describe one pixel. */
#define PTM_COEFFICIENTS      6

/** The no. of extra channels used by PTM to describe one pixel in RGB color
    mode. */
#define RGB_COEFFICIENTS      3

/** The no. of extra channels used by PTM to describe one pixel in CBCR color
    mode. */
#define CBCR_COEFFICIENTS     2

/** The maximal no. of PTM coefficient blocks used in PTM. */
#define MAX_PTM_BLOCKS        3

/** The maximal no. of color components used by libjpeg in any mode. */
#define MAX_COLOR_COMPONENTS  RGB_COEFFICIENTS

/** The maximal no. of JPEG streams that a PTM file can contain. */
#define MAX_JPEG_STREAMS      RGB_COEFFICIENTS * PTM_COEFFICIENTS

/** Scaled PTM coefficients as expected by libjpeg for de/compression. */
typedef struct {
    /* little-endian cu2 first */
    JSAMPLE cu2;
    JSAMPLE cv2;
    JSAMPLE cuv;
    JSAMPLE cu;
    JSAMPLE cv;
    JSAMPLE c1;
} ptm_coefficients_t;

/** Unscaled PTM coefficients as expected by BLAS. */
typedef struct {
    float cu2;
    float cv2;
    float cuv;
    float cu;
    float cv;
    float c1;
} ptm_unscaled_coefficients_t;

/** RGB coefficients as found in PTMs. */
typedef struct {
    JSAMPLE r;
    JSAMPLE g;
    JSAMPLE b;
} rgb_coefficients_t;

/** YCbCr coefficients as expected by jpeglib. */
typedef struct {
    JSAMPLE y;
    JSAMPLE cb;
    JSAMPLE cr;
} ycbcr_coefficients_t;

/** CrCb coefficients as found in PTMs.  Note the inversion from above! */
typedef struct {
    JSAMPLE cr;
    JSAMPLE cb;
} crcb_coefficients_t;

/** An enumeration of the supported formats. */
typedef enum {
    PTM_FORMAT_RGB = 1,
    PTM_FORMAT_LUM,
    PTM_FORMAT_LRGB,
    PTM_FORMAT_JPEG_RGB,
    PTM_FORMAT_JPEG_LRGB
} ptm_formats_enum_t;

/** A struct that describes a supported format. */
typedef struct {
    ptm_formats_enum_t id; /**< The internally used format id */
    int blocks;            /**< The no. of blocks to allocate.  This is the
                                no. of PTM coefficient blocks plus zero or one
                                block for color components. */
    int ptm_blocks;        /**< The no. of PTM coefficient blocks, either 1 =
                                LRGB or 3 = RGB */
    int color_components;  /**< The no. of color channels that are not encoded
                                with PTM.  Either 0 = RGB, 2 = LUM, or 3 = LRGB */
    int jpeg_streams;      /**< The no. of JPEG streams if the file is
                                compressed else 0*/
    const char *name;      /**< The format name.  eg. "PTM_FORMAT_RGB" */
} ptm_format_t;

/** An array containing the PTM file formats we support. */
extern const ptm_format_t ptm_formats[6];

/** Holds information about the input images and other. */
typedef struct {
    size_t width;           /**< The width of the input images. */
    size_t height;          /**< The height of the input images. */
    size_t pixels;          /**< The no. of pixels in each of the input images. */
    size_t row_stride;      /**< The row stride in the image buffer, eg. how
                               much to jump to get from one row to the next. */
    size_t decoder_stride;  /**< The decoder stride in the image buffer, eg. how
                               much to jump to get from one image to the
                               next. */
    size_t n_decoders;      /**< The no. of decoders == no. of images. */
} ptm_image_info_t;

/** This struct contains all information that goes into the PTM header.

For a full description of the PTM file format see:
http://www.hpl.hp.com/research/ptm/downloads/PtmFormat12.pdf

*/
typedef struct {
    const ptm_format_t *format;  /**< The file format. */
    size_t dimen           [2];  /**< The PTM image dimensions (w, h). */
    float scale            [PTM_COEFFICIENTS];
    int bias               [PTM_COEFFICIENTS];

    /* The following are used only for compressed PTMs */
    int compression_param  [1];  /**< The JPEG compression quality. */
    int transforms         [MAX_JPEG_STREAMS];
    int motion_vector_x    [MAX_JPEG_STREAMS];
    int motion_vector_y    [MAX_JPEG_STREAMS];
    int order              [MAX_JPEG_STREAMS];
    int reference_planes   [MAX_JPEG_STREAMS];
    size_t compressed_size [MAX_JPEG_STREAMS];
    size_t side_info_sizes [MAX_JPEG_STREAMS];
} ptm_header_t;

/** An array holding one block of either scaled PTM coefficients or RGB
    values. */
typedef JSAMPLE *ptm_block_t;

/** Information pertaining to one input image file. */
typedef struct {
    FILE *fp;       /**< The handle of the open file. */
    char *filename; /**< The filename of the input image. */
    float u, v, w;  /**< The cartesian coordinates of the light that illuminated
                         this image. */
    struct jpeg_decompress_struct dinfo;  /**< Used by libjpeg. */
} decoder_t;

/** Clip against inter-sample overflow: While all samples may be in the range
    [0..255] the reconstructed curve may well go beyond that range.  Made an
    inline function instead of a macro to avoid multiple evaluation of POLY. */
inline JSAMPLE CLIP (float f) {
    return f > 255.0 ? 255.0 : (f < 0.0 ? 0.0 : f);
}

#define LUM(r,g,b) ((unsigned int) r + (unsigned int) g + (unsigned int) b)


/**
 * Get a format by name.
 *
 * @param format_name The name of the format.
 *
 * @returns A pointer to a ptm_format_t struct.
 */
const ptm_format_t *ptm_get_format (const char *format_name);

/**
 * Allocate a PTM header structure.  Free this structure with free().
 *
 * @returns A pointer to the allocated header struct.
 */
ptm_header_t *ptm_alloc_header ();

/**
 * Allocate a struct that will contain 1 or 3 pointers to PTM blocks followed by
 * zero or one pointer to a RGB block.  Free this struct with ptm_free_blocks().
 *
 * @param ptm_header A pointer to an initialized ptm_header_t struct.
 *
 * @return A pointer to the allocated block struct.
 */
ptm_block_t *ptm_alloc_blocks (ptm_header_t *ptm_header);

/**
 * Free the structure allocated by ptm_free_blocks().
 *
 * @param ptm_header A pointer to an initialized ptm_header_t struct.
 * @param blocks     The structure to free.
 */
void ptm_free_blocks (ptm_header_t *ptm_header, ptm_block_t *blocks);

/**
 * Read the header section of a PTM file.
 *
 * @param fp A file pointer to a file openfor reading.
 *
 * @returns A ptm_header_t struct filled with information.
 */
ptm_header_t *ptm_read_header (FILE *fp);

/**
 * Write the header section of a PTM file.
 *
 * @param fp         A file pointer to a file open for writing.
 * @param ptm_header A pointer to an initialized ptm_header_t struct.
 */
void ptm_write_header (FILE *fp, ptm_header_t *ptm_header);

/**
 * Read the blocks from a PTM file.
 *
 * Does automatic JPEG decoding if the PTM format requires it.
 *
 * @param fp         File pointer.
 * @param ptm_header A pointer to an initialized ptm_header_t struct.
 * @param blocks     A pointer to an allocated ptm_block_t struct.
 */
void ptm_read_ptm  (FILE *fp, ptm_header_t *ptm_header, ptm_block_t *blocks);

/**
 * Write the blocks to a PTM file.
 *
 * Does automatic JPEG encoding if the PTM format requires it.
 *
 * @param fp         File pointer.
 * @param ptm_header A pointer to an initialized ptm_header_t struct.
 * @param blocks     A pointer to an initialized ptm_block_t struct.
 */
void ptm_write_ptm (FILE *fp, ptm_header_t *ptm_header, ptm_block_t *blocks);

/**
 * Write a JPEG file from a PTM and lighting position.
 *
 * @param fp A file pointer open for writing.
 * @param ptm_header
 * @param blocks
 * @param u The u coordinate of the light.
 * @param v The v coordinate of the light.
 */
void ptm_write_jpeg (FILE *fp, ptm_header_t *ptm_header, ptm_block_t *blocks, float u, float v);

/**
 * Does the singular value decomposition.
 *
 * @param decoders   An array of decoders.
 * @param n_decoders The number of decoders.
 *
 * @returns An n_coeffs by n_lights matrix of floats.
 */
float *ptm_svd (decoder_t **decoders, int n_decoders);


/**
 * Do the polynomial fit for all pixels in the image.
 *
 * With pixel_stride set to 3 you can process one color component out of an RGB
 * buffer like the buffer returned by libjpeg.  The output PTM coefficients
 * buffer can contain one color component only (R, G, B, or Y).
 *
 * @param info
 * @param buffer       The input buffer (filled by libjpeg).
 * @param pixel_stride The spacing of the pixels in buffer.
 * @param M            The SVD matrix.
 * @param output       The output PTM coefficients.
 */
void ptm_fit_poly_jsample (const ptm_image_info_t *info,
                           const JSAMPLE *buffer,
                           size_t pixel_stride,
                           const float *M,
                           ptm_unscaled_coefficients_t *output);

void ptm_fit_poly_uint (const ptm_image_info_t *info,
                        const unsigned int *buffer,
                        size_t pixel_stride,
                        const float *M,
                        ptm_unscaled_coefficients_t *output);

/**
 * Find the average YCbCr values of a pixel in all images.
 *
 * Use this to find the best chroma for each pixel when generating an LRGB
 * image.  Here we assume that the chroma of a pixel does not change too much
 * between exposures.  This is true for diffuse objects.  If this is not the
 * case you'd better use RGB formats.
 *
 * @param info    An info struct containing the buffer size.
 * @param buffer  Source buffer of YCbCr values.
 * @param block   The destination buffer for the average YCbCr values.
 */
void ptm_cbcr_avg (const ptm_image_info_t *info,
                   const ycbcr_coefficients_t *buffer,
                   ycbcr_coefficients_t *block);

/**
 * Scale the float coefficients into unsigned chars.
 *
 * Scales the coefficients and also sets the parameters in the PTM header.
 *
 * See: [Malzbender2001]_ §3.3 Scale and Bias
 *
 * @param ptm_header The PTM header.
 * @param unscaled   The unscaled (float) coefficients.
 * @param scaled     The scaled (unsigned char) coefficients.
 */
void ptm_scale_coefficients (ptm_header_t *ptm_header,
                             const ptm_unscaled_coefficients_t *unscaled,
                             ptm_block_t *scaled);

/**
 * Find the surface normal from the PTM coefficients.
 *
 * @param [in] coeffs  The PTM coefficients at the point of interest.
 * @param [out] nu  normal
 * @param [out] nv  normal
 * @param [out] nw  normal
 */
void ptm_normal (const ptm_unscaled_coefficients_t *coeffs, float *nu, float *nv, float *nw);


/**
 * Prints a matrix for debugging.
 *
 * @param name
 * @param M
 * @param m
 * @param n
 */
void ptm_print_matrix (const char *name, float* M, int m, int n);


#endif
