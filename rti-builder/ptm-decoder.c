/*
 * A simple command-line PTM to JPEG decoder.
 *
 * Usage: ptm-decoder filename.ptm [U V] > filename.jpg
 *
 * Set the light by specifying U and V.  U and V are the coordinates of the
 * normal projection from the unity sphere (the dome) onto the object
 * plane. (U^2 + V^2 <= 1)
 *
 * It reads PTMs in the following formats:
 *
 *   - PTM_FORMAT_RGB
 *   - PTM_FORMAT_LRGB
 *   - PTM_FORMAT_LUM (not tested for want of test images)
 *   - PTM_FORMAT_JPEG_RGB
 *   - PTM_FORMAT_JPEG_LRGB
 *
 * Prediction using motion compensation is not supported.  Output is to stdout,
 * so you can easily use this on a web server too.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "jpeglib.h"

#define PTM_COEFFICIENTS      6
#define RGB_COEFFICIENTS      3
#define CBCR_COEFFICIENTS     2
#define MAX_PTM_BLOCKS        3
#define MAX_COLOR_COMPONENTS  RGB_COEFFICIENTS
#define MAX_JPEG_STREAMS      RGB_COEFFICIENTS * PTM_COEFFICIENTS

typedef struct {
    int id;               /* internal format id */
    int blocks;           /* no. of blocks to allocate */
    int ptm_blocks;       /* no. of PTM coefficient blocks, either 1 or 3 */
    int color_components; /* no. of color channels that are not encoded with
                             PTM.  Either 0 = RGB, 2 = LUM, or 3 = LRGB */
    int jpeg_streams;     /* no. of JPEG streams (if file is compressed else 0)*/
    const char *name;
} ptm_format_t;

const ptm_format_t ptm_formats[] = {
    { 1, 3, 3, 0,  0, "PTM_FORMAT_RGB"       },
    { 2, 2, 1, 2,  0, "PTM_FORMAT_LUM"       },
    { 3, 2, 1, 3,  0, "PTM_FORMAT_LRGB"      },
    { 4, 3, 3, 0, 18, "PTM_FORMAT_JPEG_RGB"  },
    { 5, 2, 1, 3,  9, "PTM_FORMAT_JPEG_LRGB" },
};

typedef struct {
    const ptm_format_t *format; /* the file format */
    int dimen             [2];  /* w, h image dimensions */
    float scale           [PTM_COEFFICIENTS];
    int bias              [PTM_COEFFICIENTS];
    int compression_param [1];
    /* The following are used only for compressed PTMs */
    int transforms        [MAX_JPEG_STREAMS];
    int motion_vector_x   [MAX_JPEG_STREAMS];
    int motion_vector_y   [MAX_JPEG_STREAMS];
    int order             [MAX_JPEG_STREAMS];
    int reference_planes  [MAX_JPEG_STREAMS];
    int compressed_size   [MAX_JPEG_STREAMS];
    int side_info_sizes   [MAX_JPEG_STREAMS];
} ptm_header_t;

typedef struct {
    float cu2;
    float cv2;
    float cuv;
    float cu;
    float cv;
    float c1;
} ptm_coefficients_t;

/* Decode the PTM coefficients stored as bytes into floats */
#define UNSCALE(p,c) (ptm.scale[c] * ((*p++) - ptm.bias[c]))

/* Calculate the polynomial */
#define POLY(p) (UNSCALE (p, 0) * setup.cu2 + \
                 UNSCALE (p, 1) * setup.cv2 + \
                 UNSCALE (p, 2) * setup.cuv + \
                 UNSCALE (p, 3) * setup.cu  + \
                 UNSCALE (p, 4) * setup.cv  + \
                 UNSCALE (p, 5))

/* Clipping against inter-sample overflow: While all samples may be in the range
   [0..255] the reconstructed curve may well go beyond that range.  Made an
   inline function instead of a macro to avoid multiple evaluation of POLY. */
inline JSAMPLE CLIP (float f) {
    return f > 255 ? 255 : (f < 0 ? 0 : f);
}

ssize_t getline_trim (char **lineptr, size_t* n, FILE* fp) {
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

void read_floats (FILE *fp, int n, float *buf) {
    for (int i = 0; i < n; ++i) {
        fscanf (fp, "%f ", buf + i);
    }
}

FILE *read_ptm_header (char * filename, ptm_header_t *ptm) {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    if ((fp = fopen (filename, "rb")) == NULL) {
        fprintf (stderr, "can't open %s\n", filename);
        exit (1);
    }

    read = getline_trim (&line, &len, fp);
    if (strcmp (line, "PTM_1.2")) {
        fprintf (stderr, "not a PTM file: %s\n", filename);
        exit (1);
    }

    read = getline_trim (&line, &len, fp);
    memset (ptm, 0, sizeof (*ptm));

    for (int i = 0; i < sizeof (ptm_formats) / sizeof (ptm_format_t); ++i) {
        const ptm_format_t *format = &ptm_formats[i];
        if (!strcmp (line, format->name)) {
            ptm->format = format;
            break;
        }
    }
    if (ptm->format == NULL) {
        fprintf (stderr, "unsupported PTM format: %s\n", line);
        exit (1);
    }

    read_ints   (fp, 2, ptm->dimen);
    read_floats (fp, PTM_COEFFICIENTS, ptm->scale);
    read_ints   (fp, PTM_COEFFICIENTS, ptm->bias);

    if (ptm->format->jpeg_streams) {
        int n = ptm->format->jpeg_streams;
        read_ints (fp, 1, ptm->compression_param);
        read_ints (fp, n, ptm->transforms);
        read_ints (fp, n, ptm->motion_vector_x);
        read_ints (fp, n, ptm->motion_vector_y);
        read_ints (fp, n, ptm->order);
        read_ints (fp, n, ptm->reference_planes);
        read_ints (fp, n, ptm->compressed_size);
        read_ints (fp, n, ptm->side_info_sizes);
    } else {
        ptm->compression_param[0] = 90;
    }
    return fp;
}

int order_to_component (int ord, int *order, size_t size) {
    for (int i = 0; i < size; i++) {
        if (order[i] == ord)
            return i;
    }
    return -1;
}

void combine (struct jpeg_decompress_struct *dinfo, JSAMPARRAY dest, JSAMPARRAY src, boolean invert_src) {
    size_t row_stride = dinfo->output_width * dinfo->output_components;
    for (int y = 0; y < dinfo->output_height; y++) {
        JSAMPROW d = dest[y];
        JSAMPROW s = src[y];
        for (int x = 0; x < row_stride; x++) {
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

int decompress_ptm (FILE *fp_ptm, JSAMPLE **blocks, ptm_header_t *ptm) {
    JSAMPARRAY components[MAX_JPEG_STREAMS];
    void *side_infos[MAX_JPEG_STREAMS];

    struct jpeg_error_mgr jerr;
    struct jpeg_decompress_struct dinfo;
    dinfo.err = jpeg_std_error (&jerr);

    jpeg_create_decompress (&dinfo);

    for (int i = 0; i < ptm->format->jpeg_streams; ++i) {
        JOCTET *compressed_buffer = malloc (ptm->compressed_size[i]);
        fread (compressed_buffer, 1, ptm->compressed_size[i], fp_ptm);

        jpeg_mem_src (&dinfo, compressed_buffer, ptm->compressed_size[i]);

        (void) jpeg_read_header (&dinfo, TRUE);
        assert (dinfo.image_width  == ptm->dimen[0]);
        assert (dinfo.image_height == ptm->dimen[1]);
        assert (dinfo.num_components == 1);

        /* fprintf (stderr, "at %08x found jpeg stream: %d %d %d\n",
                 ftell (fp_ptm), dinfo.image_width, dinfo.image_height, dinfo.num_components);
           fflush (stderr); */

        jpeg_calc_output_dimensions (&dinfo);

        /* Make buffer for uncompressed jpeg stream. */
        size_t row_stride = dinfo.output_width * dinfo.output_components;
        components[i] = (*dinfo.mem->alloc_sarray)
            ((j_common_ptr) &dinfo, JPOOL_PERMANENT, row_stride, dinfo.output_height);

        /* Eventually make buffer for side information. */
        side_infos[i] = NULL;
        if (ptm->side_info_sizes[i] > 0) {
            side_infos[i] = (*dinfo.mem->alloc_large)
                ((j_common_ptr) &dinfo, JPOOL_PERMANENT, ptm->side_info_sizes[i]);
            fread (side_infos[i], 1, ptm->side_info_sizes[i], fp_ptm);
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

    for (int i = 0; i < ptm->format->jpeg_streams; ++i) {
        int component_index = order_to_component (i, ptm->order, ptm->format->jpeg_streams);
        assert (0 <= component_index && component_index < ptm->format->jpeg_streams);
        int reference_index = ptm->reference_planes[component_index];
        assert (-1 <= reference_index && reference_index < ptm->format->jpeg_streams);
        if (reference_index > -1) {
            /* this component was 'predicted' from another component */
            combine (&dinfo, components[component_index],
                     components[reference_index],
                     (ptm->transforms[component_index] & 1) > 0);
        }
        if (side_infos[component_index]) {
            apply_side_info (&dinfo, components[component_index],
                             side_infos[component_index],
                             ptm->side_info_sizes[component_index]);
        }
    }

    /* Copy into blocks.  The compressed PTM has a completely different layout
       than the uncompressed PTM.  Uncompressed PTMs store all coefficients in
       one block, but compressed PTMs store each coefficient in a separate
       grayscale JFIF stream.  We now transform from the compressed into the
       uncompressed layout. */

    for (int i = 0; i < ptm->format->jpeg_streams; ++i) {
        int b = i / PTM_COEFFICIENTS;
        int coeff = i % PTM_COEFFICIENTS;
        JSAMPLE *block = blocks[b];
        int sample_size = (b < ptm->format->ptm_blocks) ? PTM_COEFFICIENTS : ptm->format->color_components;
        size_t row_stride = ptm->dimen[0] * sample_size;
        for (int y = 0; y < ptm->dimen[1]; ++y) {
            JSAMPLE *row = block + y * row_stride + coeff;
            for (int x = 0; x < ptm->dimen[0]; ++x) {
                row[x * sample_size] = components[i][y][x];
            }
        }
    }

    jpeg_destroy_decompress (&dinfo);
}

int main (int argc, char *argv[]) {
    if (argc != 2 && argc != 4) {
        fprintf (stderr, "Usage: %s filename.ptm [u v]\n", argv[0]);
        fprintf (stderr, "       Outputs JPEG to stdout\n");
        return 1;
    }

    /* Open the PTM file and read the parameters section. */

    ptm_header_t ptm;
    FILE *fp_ptm = read_ptm_header (argv[1], &ptm);
    int image_size = ptm.dimen[0] * ptm.dimen[1];

    /* Read all streams in the PTM file. */

    JSAMPLE **blocks = calloc (ptm.format->blocks, sizeof (void *));
    for (int i = 0; i < ptm.format->blocks; ++i) {
        int sample_size = i < ptm.format->ptm_blocks ? PTM_COEFFICIENTS : ptm.format->color_components;
        blocks[i] = malloc (sample_size * image_size);
    }

    if (ptm.format->jpeg_streams > 0) {
        decompress_ptm (fp_ptm, blocks, &ptm);
    } else {
        for (int i = 0; i < ptm.format->blocks; ++i) {
            int sample_size = i < ptm.format->ptm_blocks ? PTM_COEFFICIENTS : ptm.format->color_components;
            fread (blocks[i], sample_size, image_size, fp_ptm);
        }
    }
    fclose (fp_ptm);

    /* Setup lighting */

    ptm_coefficients_t setup;
    setup.cu = 0.0;
    setup.cv = 0.0;
    setup.c1 = 1.0;
    if (argc == 4) {
        sscanf (argv[2], "%f", &setup.cu);
        sscanf (argv[3], "%f", &setup.cv);
    }
    setup.cu2 = setup.cu * setup.cu;
    setup.cv2 = setup.cv * setup.cv;
    setup.cuv = setup.cu * setup.cv;

    /* Create output jpeg. */

    struct jpeg_error_mgr jerr;
    struct jpeg_compress_struct cinfo;
    cinfo.err = jpeg_std_error (&jerr);
    jpeg_create_compress (&cinfo);
    jpeg_stdio_dest (&cinfo, stdout);
    cinfo.image_width  = ptm.dimen[0];      /* image width and height, in pixels */
    cinfo.image_height = ptm.dimen[1];
    cinfo.input_components = 3;             /* # of color components per pixel */
    cinfo.in_color_space = (ptm.format->color_components == 2) ? JCS_YCbCr : JCS_RGB;  /* colorspace of input image */
    jpeg_set_defaults (&cinfo);
    jpeg_set_quality (&cinfo, ptm.compression_param[0], TRUE /* limit to baseline-JPEG values */);

    JSAMPARRAY out_samples = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, ptm.dimen[0] * RGB_COEFFICIENTS, 1);

    jpeg_start_compress (&cinfo, TRUE);

    /* Compute the polynomial and put the result into an RGB interleaved buffer.
       Flip the picture vertically.  Encode the buffer.  Write the JPEG. */

    for (int y = ptm.dimen[1] - 1; y >= 0; --y) {
        JSAMPLE *p_out = out_samples[0];
        if (ptm.format->color_components == 3) {  /* PTM_FORMAT_*_LRGB */
            JSAMPLE *p   = blocks[0] + (y * ptm.dimen[0] * PTM_COEFFICIENTS);
            JSAMPLE *rgb = blocks[1] + (y * ptm.dimen[0] * RGB_COEFFICIENTS);
            size_t rgb_offset = y * ptm.dimen[0] * RGB_COEFFICIENTS;
            for (int x = 0; x < ptm.dimen[0]; ++x) {
                float L = POLY (p) / 255.0;
                *p_out++ = CLIP (L * *rgb++);
                *p_out++ = CLIP (L * *rgb++);
                *p_out++ = CLIP (L * *rgb++);
            }
        }
        if (ptm.format->color_components == 2) {  /* PTM_FORMAT_LUM not tested !*/
            JSAMPLE *p = blocks[0] + (y * ptm.dimen[0] * PTM_COEFFICIENTS);
            JSAMPLE *c = blocks[1] + (y * ptm.dimen[0] * CBCR_COEFFICIENTS);
            for (int x = 0; x < ptm.dimen[0]; ++x) {
                *p_out++ = CLIP (POLY (p));
                *p_out++ = CLIP (*c++);
                *p_out++ = CLIP (*c++);
            }
        }
        if (ptm.format->color_components == 0) {  /* PTM_FORMAT_*_RGB */
            JSAMPLE *r = blocks[0] + (y * ptm.dimen[0] * PTM_COEFFICIENTS);
            JSAMPLE *g = blocks[1] + (y * ptm.dimen[0] * PTM_COEFFICIENTS);
            JSAMPLE *b = blocks[2] + (y * ptm.dimen[0] * PTM_COEFFICIENTS);
            for (int x = 0; x < ptm.dimen[0]; ++x) {
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
    for (int i = 0; i < ptm.format->blocks; ++i) {
        free (blocks[i]);
    }
    free (blocks);
}
