/*
 * OpenCMR2 movie playback, implemented in src/video: a decoder of our own for
 * the game's Bink 1 files (revision 'i'), with no external library.
 *
 * It follows Bink's playback model, which the game's movie loop is written
 * for: decode the current frame, show it, advance, and wait until it is time
 * for the next one. The sound track plays through the audio layer.
 */
#ifndef OPENCMR2_PORT_MOVIE_H
#define OPENCMR2_PORT_MOVIE_H

#include "port/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Movie Movie;

/* Opens a game path; audioTrack selects the sound track (language). */
Movie *Movie_Open(const char *path, int audioTrack);
void Movie_Close(Movie *movie);

DWORD Movie_GetWidth(Movie *movie);
DWORD Movie_GetHeight(Movie *movie);
/* Frame numbers count from 1, as Bink's FrameNum. */
DWORD Movie_GetFrameNumber(Movie *movie);
DWORD Movie_GetFrameCount(Movie *movie);

/* Decodes the current frame (BinkDoFrame). */
void Movie_DecodeFrame(Movie *movie);
/* Moves to the next frame (BinkNextFrame). */
void Movie_NextFrame(Movie *movie);
/* Returns 1 while it is too early to show the next frame (BinkWait). */
BOOL Movie_Wait(Movie *movie);

/* Draws the decoded frame stretched into the rectangle (back buffer pixels),
   with black around it, and presents. */
void Movie_Present(Movie *movie, int x, int y, int width, int height);

#ifdef __cplusplus
}
#endif

#endif
