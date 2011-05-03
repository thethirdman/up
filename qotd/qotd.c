#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/socket.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#define QOTD_PORT	1700

#ifndef PERIOD_RENEWAL
#define PERIOD_RENEWAL	(24 * 3600)
#endif

static void usage(void)
{
	extern const char* __progname;
	printf("usage: %s cmd\n", __progname);
	exit(EXIT_FAILURE);
}

static int write_all(int sock, const char* msg, size_t nb)
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

static char *cmd = NULL;
static char actual_msg[513];

static int new_msg(void)
{
	if (cmd == NULL)
		return 0;

	FILE* cmd_output = popen(cmd, "r");

	if (cmd_output == NULL) {
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

static int generate_msg(void)
{
	static time_t yesterday = 0;

	if (yesterday == 0)
		yesterday = time(NULL) / PERIOD_RENEWAL;
	time_t today = time(NULL) / PERIOD_RENEWAL;

	if (today != yesterday || actual_msg[0] == '\0') {
		new_msg();
		return 1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	if (argc != 2)
		usage();

	cmd = argv[1];

	int sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock == -1)
		err(EXIT_FAILURE, "socket()");

	struct sockaddr_in addr;
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

		generate_msg();
		write_all(client_sock, actual_msg, strlen(actual_msg));

		if (close(client_sock) == -1)
			err(EXIT_FAILURE, "close()");
	}

	close(sock);

	return 0;
}
