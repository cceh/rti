/*
 * Explodes one PTM into multiple JPEGs lighted from different angles.
 *
 * Usage: ptm-exploder filename.ptm filename.lp filename.out
 *
 * filename.lp should be of the same format used by the PTMFitter utility, ie. a
 * list of "filename u v w\n" strings.  The filename and the w part are not
 * used.  The filename is provided by the 3rd argument and is changed into into
 * filename-NNN.jpeg for each image.
 *
 * It reads PTMs in the following formats:
 *
 *   - PTM_FORMAT_RGB
 *   - PTM_FORMAT_LRGB
 *   - PTM_FORMAT_LUM (not tested for want of test images)
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

#include "ptmlib.h"

int main (int argc, char *argv[]) {
    if (argc != 4) {
        fprintf (stderr, "Usage: %s filename.ptm filename.lp filename.out\n", argv[0]);
        fprintf (stderr, "       Explodes a PTM into JPEGs\n");
        return 1;
    }

    const char *filename_ptm = argv[1];
    const char *filename_lp  = argv[2];
    const char *filename_out = argv[3];

    /* Open the PTM file and read the header. */
    FILE *fp;
    if ((fp = fopen (filename_ptm, "rb")) == NULL) {
        fprintf (stderr, "can't open %s\n", filename_ptm);
        return 1;
    }

    /* Read the PTM header. */
    ptm_header_t *ptm = ptm_read_header (fp);
    if (ptm == NULL) {
        return 1; /* not a PTM */
    }

    /* Read the PTM data. */
    ptm_block_t *blocks = ptm_alloc_blocks (ptm);
    ptm_read_ptm (fp, ptm, blocks);
    fclose (fp);

    /* Open the light points file */
    FILE *fp_lp;
    if ((fp_lp = fopen (filename_lp, "rb")) == NULL) {
        fprintf (stderr, "can't open %s\n", filename_lp);
        return 1;
    }

    float u = 0.0, v = 0.0;
    int n = 0;
    char *filename = malloc (strlen (filename_out) + 20);
    strcpy (filename, filename_out);
    char *ext = strrchr (filename, '.');
    if (!ext)
        ext = filename + strlen (filename_out);

    char *line = NULL;
    size_t len = 0;
    while (getline (&line, &len, fp_lp) != -1) {
        char *buffer = malloc (len + 1);
        if (sscanf (line, "%s %f %f", buffer, &u, &v) == 3) {
            free (buffer);
            ++n;
            sprintf (ext, "%03d.jpeg", n);

            fprintf (stderr, "writing %s %f %f ...\n", filename, (double) u, (double) v);
            fflush (stderr);

            FILE *fp_out;
            if ((fp_out = fopen (filename, "wb")) == NULL) {
                fprintf (stderr, "can't open %s\n", filename);
                return 1;
            }

            /* Create output jpeg. */
            ptm_write_jpeg (fp_out, ptm, blocks, u, v);

            fclose (fp_out);
        }
    }
    free (filename);

    /* Cleanup */
    ptm_free_blocks (ptm, blocks);
    free (ptm);
}
