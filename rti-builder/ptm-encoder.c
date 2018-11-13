/*
 * Encodes multiple JPEG files into a PTM file.
 *
 * Usage: ptm-encoder filename.lp filename.ptm
 *
 * filename.lp is of the same format used by the PTMFitter utility, ie. a list
 * of "filename u v w\n" strings.  The filename may contain a path.
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
#include <time.h>
#include <omp.h>

#include "ptmlib.h"

int main (int argc, char *argv[]) {
    clock_t start, end;
    start = clock ();

    if (argc != 3) {
        fprintf (stderr, "Usage: %s filename.lp filename.ptm\n", argv[0]);
        fprintf (stderr, "       Encodes JPEGs into a PTM\n");
        return 1;
    }

    const char *filename_lp  = argv[1];
    const char *filename_ptm = argv[2];

    /* Open the light points file and build one JPEG decoder per input file.
       Store decoders into an array (for easy access with opencl). */

    FILE *fp_lp;
    if ((fp_lp = fopen (filename_lp, "rb")) == NULL) {
        fprintf (stderr, "can't open %s\n", filename_lp);
        return 1;
    }
    char *dirname_lp = strdup (filename_lp);
    dirname_lp = dirname (dirname_lp);

    size_t max_decoders = 64; // start with 64, maybe double them later
    decoder_t **decoders = malloc (max_decoders * sizeof (decoder_t *));
    size_t n_decoders = 0;

    struct jpeg_error_mgr jerr;

    char *line = NULL;
    size_t len = 0;
    while (getline (&line, &len, fp_lp) != -1) {
        char *filename = NULL;
        float u = 0.0, v = 0.0, w = 0.0;
        if (sscanf (line, "%ms %f %f %f", &filename, &u, &v, &w) >= 3) {

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
            free (filename);
            free (path);

            decoder_t *decoder = malloc (sizeof (decoder_t));
            struct jpeg_decompress_struct *dinfo = &decoder->dinfo;
            decoder->fp = fp_jpeg;
            decoder->filename = strdup (filename);
            decoder->u = u;
            decoder->v = v;
            decoder->w = w;

            dinfo->err = jpeg_std_error (&jerr);
            jpeg_create_decompress (dinfo);
            jpeg_stdio_src (dinfo, decoder->fp);
            (void) jpeg_read_header (dinfo, TRUE);
            (void) jpeg_start_decompress (dinfo);

            decoders[n_decoders] = decoder;
            ++n_decoders;
            if (n_decoders >= max_decoders) {
                max_decoders *= 2;
                decoders = realloc (decoders, max_decoders * sizeof (decoder_t *));
            }
        }
    }
    free (line);
    free (dirname_lp);
    fclose (fp_lp);

    if (n_decoders < 12) {
        fprintf (stderr, "not enough jpegs %lu < %d\n", n_decoders, 12);
        return 1;
    }

    fprintf (stderr, "%lu JPEGs opened ...\n", n_decoders);
    fflush (stderr);

    /* Do the SVD */
    float *M = ptm_svd (decoders, n_decoders);
    if (M == NULL) {
        fprintf (stderr, "Error in Singular Value Decomposition\n");
        return 1;
    }

    fprintf (stderr, "done SVD\n");
    fflush (stderr);

    struct jpeg_decompress_struct *dinfo = &decoders[0]->dinfo;

    for (size_t i = 0; i < n_decoders; ++i) {
        struct jpeg_decompress_struct *dinfo2 = &decoders[i]->dinfo;
        assert (dinfo2->output_width      == dinfo->output_width);
        assert (dinfo2->output_height     == dinfo->output_height);
        assert (dinfo2->output_components == dinfo->output_components);
    }

    const size_t height         = dinfo->output_height;
    const size_t width          = dinfo->output_width;
    const size_t row_stride     = width * dinfo->output_components;
    const size_t decoder_stride = height * row_stride;

    ptm_header_t * const ptm_header = ptm_alloc_header ();

    ptm_header->format = ptm_formats + 3; // PTM_FORMAT_JPEG_RGB
    ptm_header->dimen[0] = width;
    ptm_header->dimen[1] = height;

    JSAMPLE * const buffer = calloc (n_decoders * decoder_stride, sizeof (JSAMPLE));

    /* Parallel decode all JPEGs into huge buffer. */
    #pragma omp parallel for schedule(dynamic)
    for (size_t n = 0; n < n_decoders; ++n) {
        struct jpeg_decompress_struct *dinfo2 = &decoders[n]->dinfo;
        JSAMPROW *row_pointer = calloc (height, sizeof (JSAMPROW));
        for (size_t y = 0; y < height; ++y) {
            // flip the image vertically
            size_t flipped_y = (height - y - 1);
            row_pointer[y] = buffer + (n * decoder_stride) + (flipped_y * row_stride);
        }
        while (dinfo2->output_scanline < dinfo2->output_height) {
            (void) jpeg_read_scanlines (dinfo2, row_pointer + dinfo2->output_scanline, height);
        }
        (void) jpeg_finish_decompress (dinfo2);
        (void) jpeg_destroy_decompress (dinfo2);
        fclose (decoders[n]->fp);
        free (row_pointer);
    }

    end = clock ();
    fprintf (stderr, "time for decoding %lu JPEGs = %lums\n", n_decoders, (end - start) * 1000 / CLOCKS_PER_SEC);
    fflush (stderr);
    start = clock ();

    /* Parallel fit the polynomials. */
    // float[rgb][y][x][coeffs]
    ptm_unscaled_coefficients_t *coeffs = calloc (ptm_header->format->ptm_blocks * height * width,
                                                  sizeof (ptm_unscaled_coefficients_t));
    for (int r = 0; r < ptm_header->format->ptm_blocks; ++r) {
        ptm_fit_poly_rgb (ptm_header,
                          buffer + r,
                          n_decoders,
                          M,
                          coeffs + (r * height * width));
    }

    end = clock ();
    fprintf (stderr, "time for ptm_fit_poly_rgb = %lums\n", (end - start) * 1000 / CLOCKS_PER_SEC);
    fflush (stderr);

    free (buffer);
    ptm_calc_scale (ptm_header);

    start = clock ();
    ptm_block_t *blocks = ptm_alloc_blocks (ptm_header);
    ptm_scale_coefficients (ptm_header, blocks, coeffs);
    end = clock ();
    fprintf (stderr, "time for ptm_scale_coefficients = %lums\n", (end - start) * 1000 / CLOCKS_PER_SEC);

    /* Write the PTM file */
    FILE *fp_ptm;
    if ((fp_ptm = fopen (filename_ptm, "wb")) == NULL) {
        fprintf (stderr, "can't open %s\n", filename_ptm);
        return 1;
    }
    start = clock ();
    ptm_write_ptm (fp_ptm, ptm_header, blocks);
    end = clock ();
    fprintf (stderr, "time for ptm_write_ptm = %lums\n", (end - start) * 1000 / CLOCKS_PER_SEC);
    fclose (fp_ptm);

    /* Cleanup */

    free (M);
    ptm_free_blocks (ptm_header, blocks);
    for (size_t i = 0; i < n_decoders; ++i) {
        decoder_t *decoder = decoders[i];
        free (decoder->filename);
        free (decoder);
    };
    free (decoders);
    free (ptm_header);
}
