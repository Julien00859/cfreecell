#!/bin/sh
gcc \
	src/* \
	-Wall -Wextra -std=c99 -pedantic \
	-g -fsanitize=address -fsanitize=undefined \
	-o freecell
