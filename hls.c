#include "hls.h"

int get_playlist_type(char *src) {
  if (strncmp("#EXTM3U", src, 7)) {
    return -1;
  }
  if (strstr(src, "#EXT-X-STREAM-INF")) {
    return MASTER_PLAYLIST;
  }
  return MEDIA_PLAYLIST;
}

static int get_playlist_len(char *src) {
  int len = 0;
  while ((src = strchr(src, '\n'))) {
    src += 1;
    if (*src == '\r' || *src == '\n' || *src == '#') {
      continue;
    }
    if (*src == '\0') {
      break;
    }
    len += 1;
  }
  return len;
}

static int get_next_attr(char **src, char **name, char **value) {
  char *pos = strchr(*src, '=');
  if (!pos) {
    return 0;
  }
  *pos = '\0';
  *name = *src;
  *src = pos + 1;
  if (**src != '"') {
    pos = strchr(*src, ',');
    if (!pos) {
      *value = *src;
      return 1;
    }
    *pos = '\0';
    *value = *src;
    *src = pos + 1;
  } else {
    *src = pos + 2;
    pos = strchr(*src, '"');
    *pos = '\0';
    *value = *src;
    *src = pos + 1;
    if (**src == ',') {
      *src = pos + 2;
    }
  }
  if (*value) {
    return 1;
  }
  return 0;
}

int parse_master_playlist(char *src, struct MasterPlaylist *master_playlist) {
  char *pos;
  char *name;
  char *value;
  int i = 0;
  if (strncmp(src, "#EXTM3U", 7)) {
    return 1;
  }
  src += 7;
  if (!strstr(src, "#EXT-X-STREAM-INF")) {
    return 1;
  }
  master_playlist->len = get_playlist_len(src);
  master_playlist->streams = malloc(sizeof(struct Stream) * master_playlist->len);
  while ((pos = strchr(src, '\n'))) {
    *pos = '\0';
    if (*src == '\0' || *src == '\r') {
      src = pos + 1;
      continue;
    }
    if (!strncmp(src, "#EXT-X-STREAM-INF:", 18)) {
      src += 18;
      while (get_next_attr(&src, &name, &value)) {
        if (!strcmp(name, "BANDWIDTH")) {
          master_playlist->streams[i].bandwidth = strtol(value, NULL, 10);
        }
        if (!strcmp(name, "RESOLUTION")) {
          master_playlist->streams[i].resolution = value;
        }
        if (!strcmp(name, "NAME")) {
          master_playlist->streams[i].name = value;
        }
      }
      src = pos + 1;
      continue;
    }
    if (*src != '#') {
      src[strcspn(src, "\r")] = '\0';
      master_playlist->streams[i].uri = src;
      i += 1;
    }
    src = pos + 1;
  }
  return 0;
}

int parse_media_playlist(char *src, struct MediaPlaylist *media_playlist) {
  char *pos;
  char *name;
  char *value;
  int i = 0;
  if (strncmp(src, "#EXTM3U", 7)) {
    return 1;
  }
  src += 7;
  if (strstr(src, "#EXT-X-STREAM-INF")) {
    return 1;
  }
  media_playlist->len = get_playlist_len(src);
  media_playlist->segments = malloc(sizeof(struct Segment) * media_playlist->len);
  while ((pos = strchr(src, '\n'))) {
    *pos = '\0';
    if (*src == '\0' || *src == '\r') {
      src = pos + 1;
      continue;
    }
    // if (!strncmp(src, "#EXT-X-ENDLIST", 14)) {
    //   return 0;
    // }
    if (!strncmp(src, "#EXT-X-KEY:", 11)) {
      src += 11;
      while (get_next_attr(&src, &name, &value)) {
        if (!strcmp(name, "METHOD")) {
          if (!strcmp(value, "NONE")) {
            media_playlist->segments[i].key.method = NONE;
          }
          if (!strcmp(value, "AES-128")) {
            media_playlist->segments[i].key.method = AES_128;
          }
          if (!strcmp(value, "SAMPLE-AES")) {
            media_playlist->segments[i].key.method = SAMPLE_AES;
          }
        }
        if (!strcmp(name, "URI")) {
          media_playlist->segments[i].key.uri = value;
        }
        if (!strcmp(name, "IV")) {
          int j = 0;
          value += 2;
          while (j < 16) {
            char buf[3] = {value[0], value[1], 0};
            media_playlist->segments[i].key.iv[j] = strtol(buf, NULL, 16);
            value += 2;
            j += 1;
          }
        }
      }
      src = pos + 1;
      continue;
    }
    if (!strncmp(src, "#EXTINF:", 8)) {
      src += 8;
      media_playlist->segments[i].duration = strtof(src, NULL);
      src = pos + 1;
      continue;
    }
    if (*src != '#') {
      src[strcspn(src, "\r")] = '\0';
      media_playlist->segments[i].uri = src;
      i += 1;
    }
    src = pos + 1;
  }
  return 0;
}

void master_playlist_cleanup(struct MasterPlaylist *master_playlist) {
  free(master_playlist->streams);
  master_playlist->len = 0;
}

void media_playlist_cleanup(struct MediaPlaylist *media_playlist) {
  free(media_playlist->segments);
  media_playlist->len = 0;
}
