# Copyright (c) 2023 Onur Cinar. All Rights Reserved.
# The source code is provided under MIT License.
#
# https://github.com/cinar/especho

CPPFLAGS += -MD -MP

SRC = $(wildcard *.c)
HDR = $(wildcard *.h)
DEP = $(SRC:%.c=%.d)
BIN = esp_server esp_client

all: $(BIN)

fix:
	clang-format -i $(SRC) $(HDR)

clean:
	-rm $(BIN)
	-rm $(DEP)

-include $(DEP)
