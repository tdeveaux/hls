#ifndef hls_h
#define hls_h

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MASTER_PLAYLIST 0
#define MEDIA_PLAYLIST 1

enum Method {
  NONE,
  AES_128,
  SAMPLE_AES
};

struct Key {
  enum Method method;
  char *uri;
  uint8_t iv[16];
};

struct Segment {
  struct Key key;
  float duration;
  char *uri;
};

struct MediaPlaylist {
  struct Segment *segments;
  int len;
};

struct Stream {
  int bandwidth;
  char *resolution;
  char *name;
  char *uri;
};

struct MasterPlaylist {
  struct Stream *streams;
  int len;
};

int get_playlist_type(char *src);
int parse_master_playlist(char *src, struct MasterPlaylist *master_playlist);
int parse_media_playlist(char *src, struct MediaPlaylist *media_playlist);
void master_playlist_cleanup(struct MasterPlaylist *master_playlist);
void media_playlist_cleanup(struct MediaPlaylist *media_playlist);

#endif
