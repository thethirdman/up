/*
 * Copyright 2011 Gabriel Laskar. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY GABRIEL LASKAR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Gabriel Laskar.
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#define QOTD_PORT	17

#ifndef PERIOD_RENEWAL
#define PERIOD_RENEWAL	(24 * 3600)
#endif

static char *cmd = NULL;
static char actual_msg[513];

static void	usage(void);
static int	write_all(int, const char *, size_t);
static int	new_msg(void);
static int	update_msg(void);

static void
usage(void)
{
	extern const char* __progname;

	printf("usage: %s cmd\n", __progname);
	exit(EXIT_FAILURE);
}

static int
write_all(int sock, const char* msg, size_t nb)
{
	int wr;

	for (size_t nb_sent = 0; nb_sent < nb; nb_sent += wr) {
		wr = write(sock, msg + nb_sent, nb - nb_sent);
		if (wr == -1) {
			warn("write()");
			return 1;
		}
	}

	return 0;
}

static int
new_msg(void)
{
	FILE* cmd_output;

	if (cmd == NULL)
		return 0;

	if ((cmd_output = popen(cmd, "r")) == NULL) {
		warn("popen()");
		return 0;
	}

	size_t size_read = 0;

	while (!feof(cmd_output) && size_read < sizeof(actual_msg) - 1) {
		size_t r = fread(actual_msg, 1, sizeof(actual_msg) - 1 - r,
		    cmd_output);
		if (r == 0 && ferror(cmd_output)) {
			warn("fread()");
			return 0;
		}
		size_read += r;
	}

	if (pclose(cmd_output) == -1) {
		warn("pclose()");
		return 0;
	}

	actual_msg[size_read] = '\0';

	return 1;
}

static int
update_msg(void)
{
	static time_t yesterday = 0;

	time_t today = time(NULL) / PERIOD_RENEWAL;

	if (yesterday == 0)
		yesterday = today;

	if (today != yesterday || actual_msg[0] == '\0') {
		yesterday = today;
		new_msg();
		return 1;
	}

	return 0;
}

int
main(int argc, char **argv)
{
	int sock;
	struct sockaddr_in addr;

	if (argc != 2)
		usage();

	cmd = argv[1];

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		err(EXIT_FAILURE, "socket()");

	memset(&addr, 0, sizeof(addr));
	addr.sin_port = htons(QOTD_PORT);

	if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1)
		err(EXIT_FAILURE, "bind()");

	if (listen(sock, SOMAXCONN) == -1)
		err(EXIT_FAILURE, "listen()");

	for (;;) {
		int client_sock = accept(sock, 0, 0);

		if (client_sock == -1)
			err(EXIT_FAILURE, "accept()");

		update_msg();
		write_all(client_sock, actual_msg, strlen(actual_msg));

		if (close(client_sock) == -1)
			err(EXIT_FAILURE, "close()");
	}

	close(sock);

	return 0;
}
