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
		warnx("corrupt file: %s", *argv);
	}

	for (i = 0; i < nr; i += 2) {
		opcode = memory[i] << 8;
		opcode += memory[i + 1];

		if (lookup(opcode, buf, sizeof(buf)) == -1) {
			warnx("illegal opcode: %04X", opcode);
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
	int snd_nibb = (opcode & 0x0F00) >> 8;
	int trd_nibb = (opcode & 0x00F0) >> 4;
	int lst_nibb = opcode & 0x000F;
	int lst_byte = opcode & 0x00FF;
	int lst_3nib = opcode & 0x0FFF;

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
		snprintf(buf, nbytes, "SYS  %03X", lst_3nib);
		return 0;
	case 1:
		snprintf(buf, nbytes, "JP   %03X", lst_3nib);
		return 0;
	case 2:
		snprintf(buf, nbytes, "CALL %03X", lst_3nib);
		return 0;
	case 3:
		snprintf(buf, nbytes, "SE   V%01X, %02X", snd_nibb, lst_byte);
		return 0;
	case 4:
		snprintf(buf, nbytes, "SNE  V%01X, %02X", snd_nibb, lst_byte);
		return 0;
	case 5:
		switch (lst_nibb) {
		case 0:
			snprintf(buf, nbytes, "SE   V%01X, V%01X",
			    snd_nibb, trd_nibb);
			return 0;
		default:
			goto error;
		}
	case 6:
		snprintf(buf, nbytes, "LD   V%01X, %02X", snd_nibb, lst_byte);
		return 0;
	case 7:
		snprintf(buf, nbytes, "ADD  V%01X, %02X", snd_nibb, lst_byte);
		return 0;
	case 8:
		switch (lst_nibb) {
		case 0:
			snprintf(buf, nbytes, "LD   V%01X, V%01X",
			    snd_nibb, trd_nibb);
			return 0;
		case 1:
			snprintf(buf, nbytes, "OR   V%01X, V%01X",
			    snd_nibb, trd_nibb);
			return 0;
		case 2:
			snprintf(buf, nbytes, "AND  V%01X, V%01X",
			    snd_nibb, trd_nibb);
			return 0;
		case 3:
			snprintf(buf, nbytes, "XOR  V%01X, V%01X",
			    snd_nibb, trd_nibb);
			return 0;
		case 4:
			snprintf(buf, nbytes, "ADD  V%01X, V%01X",
			    snd_nibb, trd_nibb);
			return 0;
		case 5:
			snprintf(buf, nbytes, "SUB  V%01X, V%01X",
			    snd_nibb, trd_nibb);
			return 0;
		case 6:
			snprintf(buf, nbytes, "SHR  V%01X, {, V%01X}",
			    snd_nibb, trd_nibb);
			return 0;
		case 7:
			snprintf(buf, nbytes, "SUBN V%01X, V%01X",
			    snd_nibb, trd_nibb);
			return 0;
		case 0xE:
			snprintf(buf, nbytes, "SHL  V%01X, {, V%01X}",
			    snd_nibb, trd_nibb);
			return 0;
		default:
			goto error;
		}
	case 9:
		switch (lst_nibb) {
		case 0:
			snprintf(buf, nbytes, "SNE  V%01X, V%01X",
			    snd_nibb, trd_nibb);
			return 0;
		default:
			goto error;
		}
	case 0xA:
		snprintf(buf, nbytes, "LD   I, %03X", lst_3nib);
		return 0;
	case 0xB:
		snprintf(buf, nbytes, "JP   V0, %03X", lst_3nib);
		return 0;
	case 0xC:
		snprintf(buf, nbytes, "RND  V%01X, %02X", snd_nibb, lst_byte);
		return 0;
	case 0xD:
		snprintf(buf, nbytes, "DRW  V%01X, V%01X, %01X",
		    snd_nibb, trd_nibb, lst_nibb);
		return 0;
	case 0xE:
		switch (lst_byte) {
		case 0x9E:
			snprintf(buf, nbytes, "SKP  V%01X", snd_nibb);
			return 0;
		case 0xA1:
			snprintf(buf, nbytes, "SKNP V%01X", snd_nibb);
			return 0;
		default:
			goto error;
		}
	case 0xF:
		switch (lst_byte) {
		case 0x07:
			snprintf(buf, nbytes, "LD   V%01X, DT", snd_nibb);
			return 0;
		case 0x0A:
			snprintf(buf, nbytes, "LD   V%01X, K", snd_nibb);
			return 0;
		case 0x15:
			snprintf(buf, nbytes, "LD   DT, V%01X", snd_nibb);
			return 0;
		case 0x18:
			snprintf(buf, nbytes, "LD   ST, V%01X", snd_nibb);
			return 0;
		case 0x1E:
			snprintf(buf, nbytes, "ADD  I, V%01X", snd_nibb);
			return 0;
		case 0x29:
			snprintf(buf, nbytes, "LD   F, V%01X", snd_nibb);
			return 0;
		case 0x33:
			snprintf(buf, nbytes, "LD   B, V%01X", snd_nibb);
			return 0;
		case 0x55:
			snprintf(buf, nbytes, "LD   [I], V%01X", snd_nibb);
			return 0;
		case 0x65:
			snprintf(buf, nbytes, "LD   V%01X, [I]", snd_nibb);
			return 0;
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
