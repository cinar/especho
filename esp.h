#pragma once

/**
 * Copyright (c) 2023 Onur Cinar. All Rights Reserved.
 * The source code is provided under MIT License.
 *
 * https://github.com/cinar/especho
 */

struct esp_packet {
  int spi;
  int seq;
  short timeout;
  char pad;
  char next;
};
