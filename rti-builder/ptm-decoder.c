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

#include "ptmlib.h"

int main (int argc, char *argv[]) {
    if (argc != 2 && argc != 4) {
        fprintf (stderr, "Usage: %s filename.ptm [u v]\n", argv[0]);
        fprintf (stderr, "       Outputs JPEG to stdout\n");
        return 1;
    }

    const char *filename = argv[1];
    float u = 0.0;
    float v = 0.0;
    if (argc == 4) {
        sscanf (argv[2], "%f", &u);
        sscanf (argv[3], "%f", &v);
    }

    /* Open the PTM file and read the header. */
    FILE *fp;
    if ((fp = fopen (filename, "rb")) == NULL) {
        fprintf (stderr, "can't open %s\n", filename);
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

    /* Create output jpeg. */
    ptm_write_jpeg (stdout, ptm, blocks, u, v);

    /* Cleanup */
    ptm_free_blocks (ptm, blocks);
    free (ptm);
}
