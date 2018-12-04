/*
 * A library of PTM routines.
 *
 * It reads PTMs in the following formats:
 *
 *   - PTM_FORMAT_RGB
 *   - PTM_FORMAT_LRGB
 *   - PTM_FORMAT_LUM (not tested for want of test images)
 *   - PTM_FORMAT_JPEG_RGB
 *   - PTM_FORMAT_JPEG_LRGB
 *
 * Prediction using motion compensation is not supported.
 *
 * TODO: Add specular enhancement and diffuse enhancement.
 *       Add JPEG2000 if we find PTMs using it in the wild.
 *
 * Author: Marcello Perathoner <marcello@perathoner.de>
 *
 * License: GPL3
 *
 * See: http://www.hpl.hp.com/research/ptm/downloads/PtmFormat12.pdf
 *      http://www.hpl.hp.com/research/ptm/papers/ptm.pdf
 *      http://www.cs.brandeis.edu/~gim/Papers/HPL-2000-143R2.pdf
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <float.h>

#include "ptmlib.h"

#include <cblas.h>
#include <lapacke.h>

/** Parameters of the supported formats.
    id, blocks, ptm_blocks, color_components, jpeg_streams, name
*/
const ptm_format_t ptm_formats[] = {
    { PTM_FORMAT_RGB,       3, 3, 0,  0, "PTM_FORMAT_RGB"       },
    { PTM_FORMAT_JPEG_RGB,  3, 3, 0, 18, "PTM_FORMAT_JPEG_RGB"  },
    { PTM_FORMAT_LRGB,      2, 1, 3,  0, "PTM_FORMAT_LRGB"      },
    { PTM_FORMAT_JPEG_LRGB, 2, 1, 3,  9, "PTM_FORMAT_JPEG_LRGB" },
    { PTM_FORMAT_LUM,       2, 1, 2,  0, "PTM_FORMAT_LUM"       },
    { 0,                    0, 0, 0,  0, NULL },
};

/** Decode the PTM coefficients from bytes to floats. */
#define UNSCALE(coeff,n) (ptm_header->scale[n] * (coeff - ptm_header->bias[n]))

/** Evaluate the polynomial. */
// FIXME use blas sdot ?
#define POLY(p) (UNSCALE (p->cu2, 0) * light.cu2 + \
                 UNSCALE (p->cv2, 1) * light.cv2 + \
                 UNSCALE (p->cuv, 2) * light.cuv + \
                 UNSCALE (p->cu,  3) * light.cu  + \
                 UNSCALE (p->cv,  4) * light.cv  + \
                 UNSCALE (p->c1,  5))

// This is gray only but at least it works.
float COLOR_MATRIX[] = {
    0,  1,  0,  0,
    0,  1,  0,  0,
    0,  1,  0,  0,
    0,  0,  0,  1
};

// FIXME: does not work.  If we could find just *one* example of a LUM PTM we
// could fix this.
float COLOR_MATRIX_[] = {
//   Cr       Y   Cb
     1.40200, 1,  0,          0,   // R
    -0.71414, 1, -0.34414,    0,   // G
     0,       1,  1.77200,    0,   // B
     0,       0,  0,          1
};

void set_coeffs (ptm_unscaled_coefficients_t *c, float f) {
    float *pc = (float *) c;
    for (int i = 0; i < PTM_COEFFICIENTS; ++i, ++pc) {
        *pc = f;
    }
}

void min_coeffs (ptm_unscaled_coefficients_t *a, const ptm_unscaled_coefficients_t *b) {
    float *pa = (float *) a;
    float *pb = (float *) b;
    for (int i = 0; i < PTM_COEFFICIENTS; ++i, ++pa, ++pb) {
        *pa = fminf (*pa, *pb);
    }
}

void max_coeffs (ptm_unscaled_coefficients_t *a, const ptm_unscaled_coefficients_t *b) {
    float *pa = (float *) a;
    float *pb = (float *) b;
    for (int i = 0; i < PTM_COEFFICIENTS; ++i, ++pa, ++pb) {
        *pa = fmaxf (*pa, *pb);
    }
}

size_t getline_trim (char **lineptr, size_t* n, FILE* fp) {
    int read = getline (lineptr, n, fp);
    if (read == -1)
        return read;

    char *line = *lineptr;
    char *end  = line + read;

    while (end > line && isspace (*--end))
        *end = '\0';

    return line - end;
}

void read_ints (FILE *fp, int n, int *buf) {
    for (int i = 0; i < n; ++i) {
        fscanf (fp, "%d ", buf + i);
    }
}

void read_sizes (FILE *fp, int n, size_t *buf) {
    for (int i = 0; i < n; ++i) {
        fscanf (fp, "%lu ", buf + i);
    }
}

void read_floats (FILE *fp, int n, float *buf) {
    for (int i = 0; i < n; ++i) {
        fscanf (fp, "%f ", buf + i);
    }
}

void write_ints (FILE *fp, int n, int *buf) {
    for (int i = 0; i < n; ++i) {
        if (i > 0)
            fprintf (fp, " ");
        fprintf (fp, "%d", buf[i]);
    }
    fprintf (fp, "\n");
}

void write_sizes (FILE *fp, int n, size_t *buf) {
    for (int i = 0; i < n; ++i) {
        if (i > 0)
            fprintf (fp, " ");
        fprintf (fp, "%lu", buf[i]);
    }
    fprintf (fp, "\n");
}

void write_floats (FILE *fp, int n, float *buf) {
    for (int i = 0; i < n; ++i) {
        if (i > 0)
            fprintf (fp, " ");
        fprintf (fp, "%f", buf[i]);
    }
    fprintf (fp, "\n");
}

const ptm_format_t *ptm_get_format (const char *format_name) {
    const ptm_format_t *format = ptm_formats;
    while (format->name) {
        if (!strcmp (format_name, format->name)) {
            return format;
        }
        ++format;
    }
    return NULL;
}

ptm_header_t *ptm_read_header (FILE *fp) {
    char *line = NULL;
    size_t len = 0;

    getline_trim (&line, &len, fp);
    if (strcmp (line, "PTM_1.2")) {
        fprintf (stderr, "not a PTM file\n");
        return NULL;
    }

    getline_trim (&line, &len, fp);
    ptm_header_t *ptm = calloc (sizeof (*ptm), 1);

    ptm->format = ptm_get_format (line);

    if (ptm->format == NULL) {
        fprintf (stderr, "unsupported PTM format: %s\n", line);
        return NULL;
    }

    read_sizes  (fp, 2, ptm->dimen);
    read_floats (fp, PTM_COEFFICIENTS, ptm->scale);
    read_ints   (fp, PTM_COEFFICIENTS, ptm->bias);

    if (ptm->format->jpeg_streams) {
        int n = ptm->format->jpeg_streams;
        read_ints  (fp, 1, ptm->compression_param);
        read_ints  (fp, n, ptm->transforms);
        read_ints  (fp, n, ptm->motion_vector_x);
        read_ints  (fp, n, ptm->motion_vector_y);
        read_ints  (fp, n, ptm->order);
        read_ints  (fp, n, ptm->reference_planes);
        read_sizes (fp, n, ptm->compressed_size);
        read_sizes (fp, n, ptm->side_info_sizes);
    } else {
        ptm->compression_param[0] = 90;
    }

    free (line);
    return ptm;
}

void ptm_write_header (FILE *fp, ptm_header_t *ptm_header) {
    fprintf (fp, "PTM_1.2\n");
    fprintf (fp, "%s\n", ptm_header->format->name);
    // RTIViewer needs newline between w and h?
    fprintf (fp, "%lu\n%lu\n", ptm_header->dimen[0], ptm_header->dimen[1]);
    write_floats (fp, PTM_COEFFICIENTS, ptm_header->scale);
    write_ints   (fp, PTM_COEFFICIENTS, ptm_header->bias);
    if (ptm_header->format->id == PTM_FORMAT_LUM) {
        write_floats (fp, 16, COLOR_MATRIX);
    }
    if (ptm_header->format->jpeg_streams) {
        int n = ptm_header->format->jpeg_streams;
        write_ints  (fp, 1, ptm_header->compression_param);
        write_ints  (fp, n, ptm_header->transforms);
        write_ints  (fp, n, ptm_header->motion_vector_x);
        write_ints  (fp, n, ptm_header->motion_vector_y);
        write_ints  (fp, n, ptm_header->order);
        write_ints  (fp, n, ptm_header->reference_planes);
        write_sizes (fp, n, ptm_header->compressed_size);
        write_sizes (fp, n, ptm_header->side_info_sizes);
    }
}

int order_to_component (int ord, int *order, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (order[i] == ord)
            return i;
    }
    return -1;
}

void combine (struct jpeg_decompress_struct *dinfo, JSAMPARRAY dest, JSAMPARRAY src, boolean invert_src) {
    size_t row_stride = dinfo->output_width * dinfo->output_components;
    for (size_t y = 0; y < dinfo->output_height; y++) {
        JSAMPROW d = dest[y];
        JSAMPROW s = src[y];
        for (size_t x = 0; x < row_stride; x++) {
            *d += (invert_src ? 255 - *s : *s) - 128;
            s++;
            d++;
        }
    }
}

void apply_side_info (struct jpeg_decompress_struct *dinfo,
                      JSAMPARRAY component, unsigned char *side_info, size_t size) {
    size_t row_stride = dinfo->output_width * dinfo->output_components;
    size_t component_size = row_stride * dinfo->output_height;
    unsigned char *i   = side_info;
    unsigned char *end = i + size;
    while (i < end) {
        size_t offset = *i++;
        offset <<= 8;
        offset += *i++;
        offset <<= 8;
        offset += *i++;
        offset <<= 8;
        offset += *i++;
        JSAMPLE sample = *i++;

        if (offset < component_size) {
            int y = offset / row_stride;
            int x = offset % row_stride;
            component[y][x] = sample;
        }
    }
}

int get_sample_size (ptm_header_t *ptm_header, int n_block) {
    // Always assume 3 color components because the extra buffer space is very
    // convenient for YCbCr conversion etc.
    return (n_block < ptm_header->format->ptm_blocks) ? PTM_COEFFICIENTS : RGB_COEFFICIENTS;
}

ptm_header_t *ptm_alloc_header () {
    return (ptm_header_t *) calloc (sizeof (ptm_header_t), 1);
}

ptm_block_t *ptm_alloc_blocks (ptm_header_t *ptm_header) {
    ptm_block_t *blocks = calloc (ptm_header->format->blocks, sizeof (void *));
    int image_size = ptm_header->dimen[0] * ptm_header->dimen[1];
    for (int i = 0; i < ptm_header->format->blocks; ++i) {
        blocks[i] = malloc (get_sample_size (ptm_header, i) * image_size);
    }
    return blocks;
}

void ptm_free_blocks (ptm_header_t *ptm_header, ptm_block_t *blocks) {
    for (int i = 0; i < ptm_header->format->blocks; ++i) {
        free (blocks[i]);
    }
    free (blocks);
}

/**
 * Read the blocks from an uncompressed PTM file.
 *
 * @param fp         File pointer.
 * @param ptm_header A pointer to an initialized ptm_header_t struct.
 * @param blocks     A pointer to an allocated ptm_block_t struct.
 */
void ptm_read_uncompressed_blocks (FILE *fp, ptm_header_t *ptm_header, ptm_block_t *blocks) {
    int image_size = ptm_header->dimen[0] * ptm_header->dimen[1];
    for (int i = 0; i < ptm_header->format->blocks; ++i) {
        fread (blocks[i], get_sample_size (ptm_header, i), image_size, fp);
    }
}

/**
 * Write the blocks to an uncompressed PTM file.
 *
 * @param fp         File pointer.
 * @param ptm_header A pointer to an initialized ptm_header_t struct.
 * @param blocks     A pointer to an initialized ptm_block_t struct.
 */
void ptm_write_uncompressed_blocks (FILE *fp, ptm_header_t *ptm_header, ptm_block_t *blocks) {
    size_t image_size = ptm_header->dimen[0] * ptm_header->dimen[1];
    if (ptm_header->format->id == PTM_FORMAT_LUM) {
        ptm_coefficients_t   *coeffs = (ptm_coefficients_t *)   blocks[0];
        ycbcr_coefficients_t *ycbcr  = (ycbcr_coefficients_t *) blocks[1];
        struct {
            ptm_coefficients_t c;
            crcb_coefficients_t crcb;
        } tmp;
        for (size_t i = 0; i < image_size; ++i) {
            tmp.c = *coeffs;
            tmp.crcb.cr = ycbcr->cr;
            tmp.crcb.cb = ycbcr->cb;
            fwrite (&tmp, sizeof (tmp), 1, fp);
            ++ycbcr;
            ++coeffs;
        }
    } else {
        for (int i = 0; i < ptm_header->format->blocks; ++i) {
            fwrite (blocks[i], get_sample_size (ptm_header, i), image_size, fp);
        }
    }

}

/**
 * Read the blocks from a compressed PTM file.
 *
 * This function does automatic JPEG decoding.
 *
 * @param fp         File pointer.
 * @param ptm_header A pointer to an initialized ptm_header_t struct.
 * @param blocks     A pointer to an allocated ptm_block_t struct.
 */
void ptm_read_compressed_blocks (FILE *fp, ptm_header_t *ptm_header, ptm_block_t *blocks) {
    JSAMPARRAY components[MAX_JPEG_STREAMS];
    void *side_infos[MAX_JPEG_STREAMS];

    struct jpeg_error_mgr jerr;
    struct jpeg_decompress_struct dinfo;
    dinfo.err = jpeg_std_error (&jerr);

    jpeg_create_decompress (&dinfo);

    for (int i = 0; i < ptm_header->format->jpeg_streams; ++i) {
        JOCTET *compressed_buffer = malloc (ptm_header->compressed_size[i]);
        fread (compressed_buffer, 1, ptm_header->compressed_size[i], fp);

        jpeg_mem_src (&dinfo, compressed_buffer, ptm_header->compressed_size[i]);

        (void) jpeg_read_header (&dinfo, TRUE);
        assert (dinfo.image_width  == ptm_header->dimen[0]);
        assert (dinfo.image_height == ptm_header->dimen[1]);
        assert (dinfo.num_components == 1);

        /* fprintf (stderr, "at %08x found jpeg stream: %d %d %d\n",
                 ftell (fp), dinfo.image_width, dinfo.image_height, dinfo.num_components);
           fflush (stderr); */

        jpeg_calc_output_dimensions (&dinfo);

        /* Make buffer for uncompressed jpeg stream. */
        size_t row_stride = dinfo.output_width * dinfo.output_components;
        components[i] = (*dinfo.mem->alloc_sarray)
            ((j_common_ptr) &dinfo, JPOOL_PERMANENT, row_stride, dinfo.output_height);

        /* Eventually make buffer for side information. */
        side_infos[i] = NULL;
        if (ptm_header->side_info_sizes[i] > 0) {
            side_infos[i] = (*dinfo.mem->alloc_large)
                ((j_common_ptr) &dinfo, JPOOL_PERMANENT, ptm_header->side_info_sizes[i]);
            fread (side_infos[i], 1, ptm_header->side_info_sizes[i], fp);
        }

        /* Uncompress into buffer */
        (void) jpeg_start_decompress (&dinfo);
        while (dinfo.output_scanline < dinfo.output_height) {
            jpeg_read_scanlines (&dinfo, components[i] + dinfo.output_scanline, dinfo.output_height);
        }
        (void) jpeg_finish_decompress (&dinfo);

        free (compressed_buffer); /* jpeg_finish still reads from the buffer ! */
    }

    /* Apply corrections to components */

    for (int i = 0; i < ptm_header->format->jpeg_streams; ++i) {
        int component_index = order_to_component (i, ptm_header->order, ptm_header->format->jpeg_streams);
        assert (0 <= component_index && component_index < ptm_header->format->jpeg_streams);
        int reference_index = ptm_header->reference_planes[component_index];
        assert (-1 <= reference_index && reference_index < ptm_header->format->jpeg_streams);
        if (reference_index > -1) {
            /* this component was 'predicted' from another component */
            combine (&dinfo, components[component_index],
                     components[reference_index],
                     (ptm_header->transforms[component_index] & 1) > 0);
        }
        if (side_infos[component_index]) {
            apply_side_info (&dinfo, components[component_index],
                             side_infos[component_index],
                             ptm_header->side_info_sizes[component_index]);
        }
    }

    /* Copy into blocks.  The compressed PTM has a completely different layout
       than the uncompressed PTM.  Uncompressed PTMs store all coefficients in
       one block, but compressed PTMs store each coefficient in a separate
       grayscale JFIF stream.  We now transform from the compressed into the
       uncompressed layout. */

    for (int i = 0; i < ptm_header->format->jpeg_streams; ++i) {
        int b = i / PTM_COEFFICIENTS;
        int coeff = i % PTM_COEFFICIENTS;
        JSAMPLE *block = blocks[b];
        int sample_size = get_sample_size (ptm_header, b);
        size_t row_stride = ptm_header->dimen[0] * sample_size;
        for (size_t y = 0; y < ptm_header->dimen[1]; ++y) {
            JSAMPLE *row = block + y * row_stride + coeff;
            for (size_t x = 0; x < ptm_header->dimen[0]; ++x) {
                row[x * sample_size] = components[i][y][x];
            }
        }
    }

    jpeg_destroy_decompress (&dinfo);
}


/**
 * JPEG encode the blocks.
 *
 * @param ptm_header An initialized header struct.
 * @param blocks     An initialized block struct.
 * @param streams    Pointers to JPEG streams.
 */
void ptm_compress_blocks (ptm_header_t *ptm_header, ptm_block_t *blocks, JOCTET **streams) {

    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < ptm_header->format->jpeg_streams; ++i) {

        struct jpeg_error_mgr jerr;
        struct jpeg_compress_struct cinfo;
        cinfo.err = jpeg_std_error (&jerr);

        jpeg_create_compress (&cinfo);

        cinfo.image_width  = ptm_header->dimen[0];
        cinfo.image_height = ptm_header->dimen[1];
        cinfo.input_components = 1;
        cinfo.in_color_space = JCS_GRAYSCALE;
        jpeg_set_defaults (&cinfo);

        jpeg_set_quality (&cinfo, ptm_header->compression_param[0],
                          TRUE /* limit to baseline-JPEG values */);

        int b = i / PTM_COEFFICIENTS;
        int coeff = i % PTM_COEFFICIENTS;
        JSAMPLE *block = blocks[b];
        int sample_size = get_sample_size (ptm_header, b);
        size_t row_stride = ptm_header->dimen[0] * sample_size;

        /* Make temporary buffer for one line of uncompressed jpeg stream,
           because we have to de-interleave bytes. */
        JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
            ((j_common_ptr) &cinfo, JPOOL_IMAGE, cinfo.image_width, 1);

        unsigned long outsize = 0;
        unsigned char *outbuffer = NULL;
        jpeg_mem_dest (&cinfo, &outbuffer, &outsize);

        jpeg_start_compress (&cinfo, TRUE);

        while (cinfo.next_scanline < cinfo.image_height) {
            JSAMPLE *src = block + cinfo.next_scanline * row_stride + coeff;
            JSAMPLE *dest = buffer[0];
            for (size_t x = 0; x < ptm_header->dimen[0]; ++x) {
                *dest++ = *src;
                src += sample_size;
            }
            (void) jpeg_write_scanlines (&cinfo, &buffer[0], 1);
        }
        jpeg_finish_compress (&cinfo);

        streams[i] = malloc (outsize);
        memcpy (streams[i], outbuffer, outsize);
        ptm_header->compressed_size[i] = outsize;

        jpeg_destroy_compress (&cinfo);
    }
}

void ptm_read_ptm (FILE *fp, ptm_header_t *ptm_header, ptm_block_t *blocks) {
    if (ptm_header->format->jpeg_streams > 0) {
        ptm_read_compressed_blocks (fp, ptm_header, blocks);
    } else {
        ptm_read_uncompressed_blocks (fp, ptm_header, blocks);
    }
}

void ptm_write_ptm (FILE *fp, ptm_header_t *ptm_header, ptm_block_t *blocks) {
    if (ptm_header->format->jpeg_streams > 0) {
        ptm_header->compression_param[0] = 90; // quality
        JOCTET **streams = malloc (sizeof (JOCTET *) * ptm_header->format->jpeg_streams);
        ptm_compress_blocks (ptm_header, blocks, streams);
        for (int i = 0; i < ptm_header->format->jpeg_streams; ++i) {
            ptm_header->order[i] = i;
            ptm_header->reference_planes[i] = -1;
        }
        ptm_write_header (fp, ptm_header);
        for (int i = 0; i < ptm_header->format->jpeg_streams; ++i) {
            fwrite (streams[i], ptm_header->compressed_size[i], 1, fp);
        }
        free (streams);
    } else {
        ptm_write_header (fp, ptm_header);
        ptm_write_uncompressed_blocks (fp, ptm_header, blocks);
    }
}


void ptm_write_jpeg (FILE *fp, ptm_header_t *ptm_header, ptm_block_t *blocks, float u, float v) {

    /* Lighting setup */
    ptm_unscaled_coefficients_t light;
    light.cu2 = u * u;
    light.cv2 = v * v;
    light.cuv = u * v;
    light.cu  = u;
    light.cv  = v;
    light.c1  = 1.0;

    /* compress */
    struct jpeg_error_mgr jerr;
    struct jpeg_compress_struct cinfo;
    cinfo.err = jpeg_std_error (&jerr);
    jpeg_create_compress (&cinfo);
    jpeg_stdio_dest (&cinfo, fp);
    cinfo.image_width  = ptm_header->dimen[0];      /* image width and height, in pixels */
    cinfo.image_height = ptm_header->dimen[1];
    cinfo.input_components = 3;             /* # of color components per pixel */
    cinfo.in_color_space = (ptm_header->format->color_components > 0) ? JCS_YCbCr : JCS_RGB;  /* colorspace of input image */
    jpeg_set_defaults (&cinfo);
    jpeg_set_quality (&cinfo, ptm_header->compression_param[0], TRUE /* limit to baseline-JPEG values */);

    JSAMPARRAY out_samples = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, ptm_header->dimen[0] * RGB_COEFFICIENTS, 1);

    jpeg_start_compress (&cinfo, TRUE);

    /* Compute the polynomial and put the result into an RGB interleaved buffer.
       Flip the picture vertically.  Encode the buffer.  Write the JPEG. */

    size_t yoffs = ptm_header->dimen[1] * ptm_header->dimen[0];
    for (size_t y = 0; y < ptm_header->dimen[1]; ++y) {
        yoffs -= ptm_header->dimen[0];
        JSAMPLE *p_out = out_samples[0];
        if (ptm_header->format->color_components == 3) {  /* PTM_FORMAT_*_LRGB */
            ptm_coefficients_t *l   = (ptm_coefficients_t *) blocks[0] + yoffs;
            rgb_coefficients_t *rgb = (rgb_coefficients_t *) blocks[1] + yoffs;
            for (size_t x = 0; x < ptm_header->dimen[0]; ++x, ++l, ++rgb) {
                float L = POLY (l) / 255.0;
                *p_out++ = CLIP (L * rgb->r);
                *p_out++ = CLIP (L * rgb->g);
                *p_out++ = CLIP (L * rgb->b);
            }
        }
        if (ptm_header->format->color_components == 2) {  /* PTM_FORMAT_LUM not tested !*/
            ptm_coefficients_t  *l    = (ptm_coefficients_t *)  blocks[0] + yoffs;
            crcb_coefficients_t *crcb = (crcb_coefficients_t *) blocks[1] + yoffs;
            for (size_t x = 0; x < ptm_header->dimen[0]; ++x, ++l, ++crcb) {
                *p_out++ = CLIP (POLY (l));
                *p_out++ = CLIP (crcb->cb);
                *p_out++ = CLIP (crcb->cr);
            }
        }
        if (ptm_header->format->color_components == 0) {  /* PTM_FORMAT_*_RGB */
            ptm_coefficients_t *r = (ptm_coefficients_t *) blocks[0] + yoffs;
            ptm_coefficients_t *g = (ptm_coefficients_t *) blocks[1] + yoffs;
            ptm_coefficients_t *b = (ptm_coefficients_t *) blocks[2] + yoffs;
            for (size_t x = 0; x < ptm_header->dimen[0]; ++x, ++r, ++g, ++b) {
                *p_out++ = CLIP (POLY (r));
                *p_out++ = CLIP (POLY (g));
                *p_out++ = CLIP (POLY (b));
            }
        }
        (void) jpeg_write_scanlines (&cinfo, out_samples, 1);
    }

    jpeg_finish_compress (&cinfo);

    /* cleanup */

    jpeg_destroy_compress (&cinfo);
}

float *ptm_svd (decoder_t **decoders, int n_decoders) {
    lapack_int n_lights = n_decoders;
    lapack_int n_coeffs = PTM_COEFFICIENTS;

    ptm_unscaled_coefficients_t *A = calloc (n_lights, sizeof (ptm_unscaled_coefficients_t));
    float *U  = calloc (n_lights * n_coeffs, sizeof (float));
    float *S  = calloc (n_coeffs * n_coeffs, sizeof (float));
    float *V  = calloc (n_coeffs * n_coeffs, sizeof (float));
    float *Sv = calloc (n_coeffs,            sizeof (float));

    ptm_unscaled_coefficients_t *a = A;
    for (int i = 0; i < n_lights; ++i, ++a) {
        decoder_t *decoder = decoders[i];
        float u = decoder->u,
              v = decoder->v;
        a->cu2 = u * u;
        a->cv2 = v * v;
        a->cuv = u * v;
        a->cu = u;
        a->cv = v;
        a->c1 = 1.0;
    }

    // ptm_print_matrix ("A", (float *) A, n_lights, n_coeffs);

    /* 'S' = do a thin SVG */
    lapack_int info;
    info = LAPACKE_sgesdd (LAPACK_ROW_MAJOR, 'S', n_lights, n_coeffs,
                           (float *) A, n_coeffs,
                           Sv,
                           U, n_coeffs,
                           V, n_coeffs);
    if (info > 0) {
        return NULL;
    }

    // ptm_print_matrix ("U", U,  n_lights, n_coeffs);
    // ptm_print_matrix ("S", Sv, 1, n_coeffs);
    // ptm_print_matrix ("V", V,  n_coeffs, n_coeffs);

    for (int i = 0; i < n_coeffs; ++i) {
        S[i * n_coeffs + i] = 1.0 / Sv[i];
    }

    /* M = V * diag (1 ./ diag (S)) * U' */
    float *M1 = calloc (n_coeffs * n_coeffs, sizeof (float));
    float *M  = calloc (n_coeffs * n_lights, sizeof (float));
    cblas_sgemm (CblasRowMajor, CblasTrans, CblasNoTrans, n_coeffs, n_coeffs, n_coeffs,
                 1.0, V, n_coeffs,
                 S, n_coeffs,
                 0.0, M1, n_coeffs);
    cblas_sgemm (CblasRowMajor, CblasNoTrans, CblasTrans, n_coeffs, n_lights, n_coeffs,
                 1.0, M1, n_coeffs,
                 U, n_coeffs,
                 0.0, M, n_lights);

    // ptm_print_matrix ("M", M, n_coeffs, n_lights);

    return M;
}


void ptm_fit_poly_jsample (const ptm_image_info_t *info,
                           const JSAMPLE *buffer,
                           size_t pixel_stride,
                           const float *M,
                           ptm_unscaled_coefficients_t *output) {

    // buffer = JSAMPLE[image][y][x][rgb]
    // output = float[y][x][cu²..c1]

    const size_t row_stride   = info->width  * pixel_stride;
    const size_t image_stride = info->height * row_stride;

    #pragma omp parallel for schedule(dynamic)
    for (size_t y = 0; y < info->height; ++y) {
        ptm_unscaled_coefficients_t *bl = output + (y * info->width);
        float *b = calloc (info->n_decoders, sizeof (float)); // vector of samples as floats

        for (size_t x = 0; x < info->width; ++x) {
            const JSAMPLE *buf = buffer + (y * row_stride) + (x * pixel_stride);
            // copy vector b into floats
            for (size_t n = 0; n < info->n_decoders; ++n) {
                b[n] = (float) *buf;
                buf += image_stride;
            }
            // X = M * b
            cblas_sgemv (CblasRowMajor, CblasNoTrans, PTM_COEFFICIENTS, info->n_decoders,
                         1.0, M, info->n_decoders,
                         b, 1,
                         0.0, (float *) bl, 1);
            ++bl;
        }
        free (b);
    }
}

void ptm_fit_poly_uint (const ptm_image_info_t *info,
                        const unsigned int *buffer,
                        size_t pixel_stride,
                        const float *M,
                        ptm_unscaled_coefficients_t *output) {

    // buffer = JSAMPLE[image][y][x][L]
    // output = float[y][x][cu²..c1]

    const size_t row_stride   = info->width  * pixel_stride;
    const size_t image_stride = info->height * row_stride;

    #pragma omp parallel for schedule(dynamic)
    for (size_t y = 0; y < info->height; ++y) {
        ptm_unscaled_coefficients_t *bl = output + (y * info->width);
        float *b = calloc (info->n_decoders, sizeof (float)); // vector of samples

        for (size_t x = 0; x < info->width; ++x) {
            const unsigned int *buf = buffer + (y * row_stride) + (x * pixel_stride);
            // copy vector b into floats
            for (size_t n = 0; n < info->n_decoders; ++n) {
                b[n] = (float) *buf;
                buf += image_stride;
            }
            // X = M * b
            cblas_sgemv (CblasRowMajor, CblasNoTrans, PTM_COEFFICIENTS, info->n_decoders,
                         1.0, M, info->n_decoders,
                         b, 1,
                         0.0, (float *) bl, 1);
            ++bl;
        }
        free (b);
    }
}

void ptm_cbcr_avg (const ptm_image_info_t *info,
                   const ycbcr_coefficients_t *buffer,
                   ycbcr_coefficients_t *block) {

    typedef struct {
        unsigned int y;
        unsigned int cb;
        unsigned int cr;
    } cbcr_sums_t;

    cbcr_sums_t *sums = calloc (info->pixels, sizeof (cbcr_sums_t));

    #pragma omp parallel for schedule(dynamic)
    for (size_t y = 0; y < info->height; ++y) {
        for (size_t d = 0; d < info->n_decoders; ++d) {
            const ycbcr_coefficients_t *row = buffer + (d * info->pixels) + (y * info->width);
            cbcr_sums_t *s = sums + (y * info->width);
            for (size_t i = 0; i < info->width; ++i) {
                s->y  += row->y;
                s->cb += row->cb;
                s->cr += row->cr;
                ++row;
                ++s;
            }
        }
    }

    #pragma omp parallel for schedule(dynamic)
    for (size_t y = 0; y < info->height; ++y) {
        cbcr_sums_t *s = sums + (y * info->width);
        ycbcr_coefficients_t *bl = block + (y * info->width);
        for (size_t i = 0; i < info->width; ++i) {
            bl->y  = s->y  / info->n_decoders;
            bl->cb = s->cb / info->n_decoders;
            bl->cr = s->cr / info->n_decoders;
            ++s;
            ++bl;
        }
    }
    free (sums);
}


/**
 * See: [Malzbender2001] equations 16 and 17.
 */
void ptm_normal (const ptm_unscaled_coefficients_t *coeffs, float *nu, float *nv, float *nw) {
    float divisor = 4 * coeffs->cu2 * coeffs->cv2 - coeffs->cuv * coeffs->cuv;

    *nu = (coeffs->cuv * coeffs->cv - 2 * coeffs->cv2 * coeffs->cu) / divisor;
    *nv = (coeffs->cuv * coeffs->cu - 2 * coeffs->cu2 * coeffs->cv) / divisor;
    *nw = sqrt (1 - *nu * *nu + *nv * *nv);
}

void ptm_scale_coefficients (ptm_header_t *ptm_header,
                             const ptm_unscaled_coefficients_t *unscaled,
                             ptm_block_t *scaled) {

    const size_t image_size = ptm_header->dimen[1] * ptm_header->dimen[0];

    // get the minimum and maximum coefficients
    ptm_unscaled_coefficients_t min_coefficients;
    ptm_unscaled_coefficients_t max_coefficients;

    set_coeffs (&min_coefficients,  FLT_MAX);
    set_coeffs (&max_coefficients, -FLT_MAX);

    #pragma omp parallel for schedule(dynamic)
    for (size_t y = 0; y < ptm_header->dimen[1]; ++y) {
        const ptm_unscaled_coefficients_t *c = unscaled + (y * ptm_header->dimen[0]);
        ptm_unscaled_coefficients_t min, max;
        set_coeffs (&min,  FLT_MAX);
        set_coeffs (&max, -FLT_MAX);
        for (size_t x = 0; x < ptm_header->dimen[0]; ++x) {
            min_coeffs (&min, c);
            max_coeffs (&max, c);
            ++c;
        }
        #pragma omp critical (min_max_coeffs)
        {
            // update the global min/max coefficients
            min_coeffs (&min_coefficients, &min);
            max_coeffs (&max_coefficients, &max);
        }
    }

    float *min = (float *) &min_coefficients;
    float *max = (float *) &max_coefficients;

    // mul is faster than div on many cpus
    float inv_scale [PTM_COEFFICIENTS];

    // ptm_print_matrix ("max_coeffs", max, 1, PTM_COEFFICIENTS);
    // ptm_print_matrix ("min_coeffs", min, 1, PTM_COEFFICIENTS);

    for (int i = 0; i < PTM_COEFFICIENTS; ++i) {
        // we have to fit the floating point range into the range 0-255
        ptm_header->scale[i] = (max[i] - min[i]) / 256.0;
        ptm_header->bias[i]  = -256.0 / (max[i] - min[i]) * min[i];
        inv_scale[i] = 1.0 / ptm_header->scale[i];
    }

    int *bias = ptm_header->bias;

    for (int i = 0; i < ptm_header->format->ptm_blocks; ++i) {
        // we are more probably memory-bound than cpu-bound here
        #pragma omp parallel for schedule(dynamic)
        for (size_t y = 0; y < ptm_header->dimen[1]; ++y) {
            JSAMPLE *s = scaled[i] + (y * ptm_header->dimen[0] * PTM_COEFFICIENTS);
            float *u = (float *) (unscaled + (i * image_size) + (y * ptm_header->dimen[0]));
            for (size_t x = 0; x < ptm_header->dimen[0]; ++x) {
                for (int n = 0; n < PTM_COEFFICIENTS; ++n, ++s, ++u) {
                    /* Encode the PTM coefficients from floats to bytes */
                    *s = CLIP ((*u * inv_scale[n]) + bias[n]);
                }
            }
        }
    }
}

void ptm_print_matrix (const char *name, float* M, int m, int n) {
    int i, j;
    fprintf (stderr, "\n%s = [", name);
    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++)
            fprintf (stderr, " %3.6f", M[i * n + j]);
        fprintf (stderr, ";\n");
    }
    fprintf (stderr, "]\n");
}


void print_scale_bias_coefficients (const ptm_header_t *ptm_header) {
    ptm_print_matrix ("scale", (float *) &ptm_header->scale, 1, PTM_COEFFICIENTS);

    fprintf (stderr, "bias  = [");
    for (int i = 0; i < PTM_COEFFICIENTS; ++i) {
        fprintf (stderr, " %d", ptm_header->bias[i]);
    }
    fprintf (stderr, " ]\n");
}
