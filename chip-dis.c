/*
 * Copyright (c) 2019 Christopher Hettrick <chris@structfoo.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int	lookup(uint16_t, void *, size_t);
void	usage(void);

int
main(int argc, char *argv[])
{
	int		fd, nr, i;
	uint8_t		memory[4096];
	uint16_t	opcode;
	char		buf[20];

	argc--;
	argv++;

	if (argc != 1) {
		usage();
	}

	if ((fd = open(*argv, O_RDONLY)) < 0) {
		err(2, "%s", *argv);
	}

	if ((nr = read(fd, memory, sizeof(memory))) == -1) {
		err(1, "read: %s", *argv);
	}

	if (nr % 2 == 1) {
		errx(1, "corrupt file: %s", *argv);
	}

	printf("Read: %d bytes\n", nr);

	for (i = 0; i < nr; i += 2) {
		opcode = memory[i] << 8;
		opcode += memory[i + 1];

		if (lookup(opcode, buf, sizeof(buf)) == -1) {
			errx(1, "illegal opcode: %04X", opcode);
		}

		printf("%04X\t%s\n", opcode, buf);
	}

	return 0;
}

int
lookup(uint16_t opcode, void *buf, size_t nbytes)
{
	char *s = "yo world!";

	snprintf(buf, nbytes, "%s", s);

	int fst_nibb = (opcode & 0xF000) >> 12;
	int lst_nibb = opcode & 0x000F;
	int lst_byte = opcode & 0x00FF;

	switch (opcode) {
	case 0x00E0:
		snprintf(buf, nbytes, "CLS");
		return 0;
	case 0x00EE:
		snprintf(buf, nbytes, "RET");
		return 0;
	default:
		break;
	}

	switch (fst_nibb) {
	case 0:
		snprintf(buf, nbytes, "SYS  %03X", opcode & 0x0FFF);
		return 0;
	case 1:
		snprintf(buf, nbytes, "JP   %03X", opcode & 0x0FFF);
		return 0;
	case 2:
		snprintf(buf, nbytes, "CALL %03X", opcode & 0x0FFF);
		return 0;
	case 3:
	case 4:
	case 5:
		switch (lst_nibb) {
		case 0:
		default:
			goto error;
		}
	case 6:
	case 7:
	case 8:
		switch (lst_nibb) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 0xE:
		default:
			goto error;
		}
	case 9:
		switch (lst_nibb) {
		case 0:
		default:
			goto error;
		}
	case 0xA:
	case 0xB:
	case 0xC:
	case 0xD:
	case 0xE:
		switch (lst_byte) {
		case 0x9E:
		case 0xA1:
		default:
			goto error;
		}
	case 0xF:
		switch (lst_byte) {
		case 0x07:
		case 0x0A:
		case 0x15:
		case 0x18:
		case 0x1E:
		case 0x29:
		case 0x33:
		case 0x55:
		case 0x65:
		default:
			goto error;
		}
	default:
		goto error;
	}

error:
	snprintf(buf, nbytes, "");
	return -1;
}

void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s file\n", __progname);
	exit(1);
}
