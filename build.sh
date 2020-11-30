#!/bin/sh
gcc \
	src/* \
	-Wall -Wextra -std=c99 -pedantic \
	--debug \
	-o freecell
