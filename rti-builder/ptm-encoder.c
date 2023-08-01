/*
 * Encodes multiple JPEG files into a PTM file.
 *
 * Usage: ptm-encoder -o filename.ptm filename.lp
 *
 * filename.lp is of the same format used by the PTMFitter utility, ie. a list
 * of "filename u v w\n" strings.  The filename may contain a path which is
 * resolved relative to filename.lp.
 *
 * It builds PTMs in the following formats:
 *
 *   - PTM_FORMAT_RGB
 *   - PTM_FORMAT_LRGB
 *   - PTM_FORMAT_JPEG_RGB
 *   - PTM_FORMAT_JPEG_LRGB
 *
 * Author: Marcello Perathoner <marcello@perathoner.de>
 *
 * License: GPL3
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <assert.h>
#include <argp.h>
#include <time.h>
#include <math.h>
#include <omp.h>
#include <cblas.h>

#include "ptmlib.h"

const char *argp_program_version = "PTM Encoder 0.1";

static struct argp_option options[] = {
    { "format",  'f', "FORMAT", 0, "Which PTM format to output (default: PTM_FORMAT_JPEG_RGB).", 0},
    { "list",    'l', 0,        0, "List supported PTM formats.",                                0},
    { "output",  'o', "FILE",   0, "Output to FILE instead of STDOUT.",                          1},
    { "verbose", 'v', 0,        0, "Produce verbose output.",                                    2},
    { 0 }
};

struct arguments {
    const ptm_format_t *format;
    const char *filename_lp;
    const char *filename_ptm;
    int verbose;
};

#define TIME(...)                                                       \
    end = clock ();                                                     \
    if (arguments.verbose) {                                            \
        fprintf (stderr, __VA_ARGS__,                                   \
                 (end - start) * 1000 / CLOCKS_PER_SEC);                \
        fflush (stderr);                                                \
    }                                                                   \
    start = clock ()


static error_t parse_opt (int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
    switch (key) {
    case 'f':
        arguments->format = ptm_get_format (arg);
        if (arguments->format == NULL) {
            fprintf (stderr, "No format by that name: %s\n", arg);
            exit (0);
        }
        break;
    case 'l':
        printf ("Supported formats:\n");
        const ptm_format_t *format = ptm_formats;
        while (format->name) {
            printf ("%s\n", format->name);
            ++format;
        }
        exit (0);
        break;
    case 'o':
        arguments->filename_ptm = arg;
        break;
    case 'v':
        arguments->verbose = 1;
        break;
    case ARGP_KEY_ARG:
        if (state->arg_num >= 1)
            /* Too many arguments. */
            argp_usage (state);
        arguments->filename_lp = arg;
        break;
    case ARGP_KEY_END:
        if (state->arg_num < 1)
            /* Not enough arguments. */
            argp_usage (state);
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {
    options,
    parse_opt,
    "FILENAME.LP",
    "Encode a series of JPEG files into a PTM file.",
    NULL,
    NULL,
    NULL
};

int main (int argc, char *argv[]) {
    struct arguments arguments;

    arguments.verbose      = 0;
    arguments.format       = ptm_get_format ("PTM_FORMAT_JPEG_RGB");
    arguments.filename_ptm = "-";

    argp_parse (&argp, argc, argv, 0, 0, &arguments);

    clock_t start;
    clock_t end;
    start = clock ();

    /* Open the light points file and build one JPEG decoder per input file.
       Store decoders into an array (for easy access with opencl). */

    FILE *fp_lp;
    if ((fp_lp = fopen (arguments.filename_lp, "rb")) == NULL) {
        fprintf (stderr, "can't open %s\n", arguments.filename_lp);
        return 1;
    }
    char *dirname_lp = strdup (arguments.filename_lp);
    dirname_lp = dirname (dirname_lp);

    ptm_header_t * const ptm_header = ptm_alloc_header ();
    ptm_header->format = arguments.format;

    size_t max_decoders = 64; // start with 64, maybe double them later
    decoder_t **decoders = malloc (max_decoders * sizeof (decoder_t *));
    ptm_image_info_t info;
    info.n_decoders = 0;

    struct jpeg_error_mgr jerr;

    char *line = NULL;
    size_t len = 0;
    while (getline (&line, &len, fp_lp) != -1) {
        char *filename = malloc (len + 1);
        float u = 0.0f;
        float v = 0.0f;
        float w = 0.0f;
        if (sscanf (line, "%s %f %f %f", filename, &u, &v, &w) >= 3) {

            /* fprintf (stderr, "reading %s %f %f %f ...\n",
                        filename, (double) u, (double) v, (double) w);
               fflush (stderr); */

            char *path = malloc (strlen (dirname_lp) + strlen (filename) + 2);
            sprintf (path, "%s/%s", dirname_lp, filename);

            FILE *fp_jpeg;
            if ((fp_jpeg = fopen (path, "rb")) == NULL) {
                fprintf (stderr, "can't open %s\n", path);
                return 1;
            }
            free (path);

            decoder_t *decoder = malloc (sizeof (decoder_t));
            struct jpeg_decompress_struct *dinfo = &decoder->dinfo;
            decoder->fp = fp_jpeg;
            decoder->filename = filename;
            decoder->u = u;
            decoder->v = v;
            decoder->w = w;

            dinfo->err = jpeg_std_error (&jerr);
            jpeg_create_decompress (dinfo);
            jpeg_stdio_src (dinfo, decoder->fp);
            (void) jpeg_read_header (dinfo, TRUE);

            // use YCbCr color space for PTM_LUM and PTM_LRGB files
            // use RGB color space for PTM_RGB files
            dinfo->out_color_space = (ptm_header->format->color_components > 0) ? JCS_YCbCr : JCS_RGB;

            (void) jpeg_start_decompress (dinfo);

            decoders[info.n_decoders] = decoder;
            ++info.n_decoders;
            if (info.n_decoders >= max_decoders) {
                max_decoders *= 2;
                decoders = realloc (decoders, max_decoders * sizeof (decoder_t *));
            }
        }
    }
    free (line);
    free (dirname_lp);
    fclose (fp_lp);

    if (info.n_decoders < 12) {
        fprintf (stderr, "not enough jpegs %u < %d\n", info.n_decoders, 12);
        return 1;
    }

    if (arguments.verbose) {
        fprintf (stderr, "%u JPEGs opened ...\n", info.n_decoders);
        fflush (stderr);
    }

    /* Do the SVD */
    float *M = ptm_svd (decoders, info.n_decoders);
    if (M == NULL) {
        fprintf (stderr, "Error in Singular Value Decomposition\n");
        return 1;
    }

    if (arguments.verbose) {
        fprintf (stderr, "done SVD\n");
        fflush (stderr);
    }

    const struct jpeg_decompress_struct *dinfo = &decoders[0]->dinfo;

    info.height         = dinfo->output_height;
    info.width          = dinfo->output_width;
    info.pixels         = info.height * info.width;
    info.row_stride     = info.width * dinfo->output_components;
    info.decoder_stride = info.height * info.row_stride;

    ptm_header->dimen[0] = info.width;
    ptm_header->dimen[1] = info.height;

    // sanity check: all images must be the same size
    for (size_t i = 0; i < info.n_decoders; ++i) {
        const struct jpeg_decompress_struct *dinfo2 = &decoders[i]->dinfo;
        assert (dinfo2->output_width      == dinfo->output_width);
        assert (dinfo2->output_height     == dinfo->output_height);
        assert (dinfo2->output_components == dinfo->output_components);
    }

    JSAMPLE * const buffer = calloc (info.n_decoders * info.decoder_stride, sizeof (JSAMPLE));

    /* Parallel decode all JPEGs into huge buffer. */
    #pragma omp parallel for schedule(dynamic)
    for (size_t n = 0; n < info.n_decoders; ++n) {
        struct jpeg_decompress_struct *dinfo2 = &decoders[n]->dinfo;
        JSAMPROW *row_pointer = calloc (info.height, sizeof (JSAMPROW));
        for (size_t y = 0; y < info.height; ++y) {
            // flip the image vertically
            size_t flipped_y = (info.height - y - 1);
            row_pointer[y] = buffer + (n * info.decoder_stride) + (flipped_y * info.row_stride);
        }
        while (dinfo2->output_scanline < dinfo2->output_height) {
            (void) jpeg_read_scanlines (dinfo2, row_pointer + dinfo2->output_scanline, info.height);
        }
        (void) jpeg_finish_decompress (dinfo2);
        (void) jpeg_destroy_decompress (dinfo2);
        fclose (decoders[n]->fp);
        free (row_pointer);
    }

    TIME ("time for decoding %u JPEGs = %lums\n", info.n_decoders);

    // float[rgb][y][x][coeffs]
    ptm_unscaled_coefficients_t *coeffs = NULL;
    ptm_block_t *blocks = ptm_alloc_blocks (ptm_header);

    if (ptm_header->format->color_components == 0) {
        // a PTM_RGB format
        coeffs = calloc (RGB_COEFFICIENTS * info.pixels, sizeof (ptm_unscaled_coefficients_t));

        // fit each of the color channels to the polynomes
        for (int r = 0; r < ptm_header->format->ptm_blocks; ++r) {
            ptm_fit_poly_jsample (&info,
                                  buffer + r,
                                  RGB_COEFFICIENTS,
                                  M,
                                  coeffs + (r * info.pixels));
        }
    }

    if (ptm_header->format->color_components == 2) {
        // a PTM_LUM format
        //
        // N.B. the PTM_LUM formats are largely undocumented.  The following is
        // based on some educated guess but is probably not quite correct.

        coeffs = calloc (info.pixels, sizeof (ptm_unscaled_coefficients_t));

        // fit polynomes to the Y (of YCbCr)
        ptm_fit_poly_jsample (&info,
                              buffer,
                              3,
                              M,
                              coeffs);

        // then average Cb and Cr over all images
        ptm_cbcr_avg (&info,
                      (const ycbcr_coefficients_t *) buffer,
                      (ycbcr_coefficients_t *) blocks[ptm_header->format->ptm_blocks]);
    }

    if (ptm_header->format->color_components == 3) {
        // an LRGB format
        //
        // N.B. the LRGB formats are largely undocumented.  The following is
        // based on some educated guess but is probably not quite correct.

        coeffs = calloc (info.pixels, sizeof (ptm_unscaled_coefficients_t));

        // fit polynomes to the Y (of YCbCr)
        ptm_fit_poly_jsample (&info,
                              buffer,
                              3,
                              M,
                              coeffs);

        // then average Cb and Cr over all images
        ptm_cbcr_avg (&info,
                      (const ycbcr_coefficients_t *) buffer,
                      (ycbcr_coefficients_t *) blocks[ptm_header->format->ptm_blocks]);

        const ycbcr_coefficients_t *ycbcr = (ycbcr_coefficients_t *) blocks[ptm_header->format->ptm_blocks];
        rgb_coefficients_t *rgb = (rgb_coefficients_t *) blocks[ptm_header->format->ptm_blocks];
        ptm_unscaled_coefficients_t *cfs = coeffs;

        for (size_t i = 0; i < info.pixels; ++i) {
            float y  = ycbcr->y;
            float cb = ycbcr->cb - CENTERJSAMPLE;
            float cr = ycbcr->cr - CENTERJSAMPLE;
            // See: https://github.com/libjpeg-turbo/libjpeg-turbo/blob/master/jdcolor.c
            // See: https://en.wikipedia.org/wiki/YCbCr#JPEG_conversion
            rgb->r = CLIP (y                 + 1.40200f * cr);
            rgb->g = CLIP (y - 0.34414f * cb - 0.71414f * cr);
            rgb->b = CLIP (y + 1.77200f * cb);

            // finally fix the polynome factors so that Y * R' ~ R
            float norm = 256.0f / y;
            cfs->cu2 *= norm;
            cfs->cv2 *= norm;
            cfs->cuv *= norm;
            cfs->cu  *= norm;
            cfs->cv  *= norm;
            cfs->c1  *= norm;

            ++rgb;
            ++ycbcr;
            ++cfs;
        }
    }

    if (ptm_header->format->color_components == 666) {
        // an LRGB format
        //
        // The algorithm alluded to in [Zhang2012]_ uses the median, which is a
        // bear to compute.

        coeffs = calloc (info.pixels, sizeof (ptm_unscaled_coefficients_t));

        // calculate L = R + G + B for all pixels in all images
        unsigned int *L = calloc (info.n_decoders * info.pixels, sizeof (int));
        {
            const rgb_coefficients_t *rgb = (rgb_coefficients_t *) buffer;
            unsigned int *l = L;
            for (size_t i = 0; i < info.n_decoders * info.pixels; ++i) {
                *l = rgb->r + rgb->g + rgb->b;
                ++rgb;
                ++l;
            }
        }
        // fit polynomes to the L
        ptm_fit_poly_uint (&info,
                           L,
                           1,
                           M,
                           coeffs);

        // then average RGB over all images
        // FIXME should use median here!
        ptm_cbcr_avg (&info,
                      (const ycbcr_coefficients_t *) buffer,
                      (ycbcr_coefficients_t *) blocks[ptm_header->format->ptm_blocks]);

        // scale RGB according to L
        rgb_coefficients_t *rgb = (rgb_coefficients_t *) blocks[ptm_header->format->ptm_blocks];
        const unsigned int *l = L;
        for (size_t i = 0; i < info.pixels; ++i) {
            rgb->r = 256 * (int) rgb->r / *l;
            rgb->g = 256 * (int) rgb->g / *l;
            rgb->b = 256 * (int) rgb->b / *l;
            ++rgb;
            ++l;
        }

        free (L);
    }

    free (buffer);

    TIME ("time for ptm_fit_poly_jsample = %lums\n");

    ptm_scale_coefficients (ptm_header, coeffs, blocks);

    TIME ("time for ptm_scale_coefficients = %lums\n");

    /* Write the PTM file */
    FILE *fp_ptm;
    if (!strcmp (arguments.filename_ptm, "-")) {
        fp_ptm = stdout;
    } else {
        if ((fp_ptm = fopen (arguments.filename_ptm, "wb")) == NULL) {
            fprintf (stderr, "can't open %s\n", arguments.filename_ptm);
            return 1;
        }
    }

    ptm_write_ptm (fp_ptm, ptm_header, blocks);

    TIME ("time for ptm_write_ptm = %lums\n");

    fclose (fp_ptm);

    /* Cleanup */

    free (M);
    ptm_free_blocks (ptm_header, blocks);
    for (size_t i = 0; i < info.n_decoders; ++i) {
        decoder_t *decoder = decoders[i];
        free (decoder->filename);
        free (decoder);
    }
    free (decoders);
    free (ptm_header);
}
