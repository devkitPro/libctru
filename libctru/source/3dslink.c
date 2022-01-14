#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <3ds/3dslink.h>

struct in_addr __3dslink_host = {0};

static int sock = -1;

int link3dsConnectToHost(bool redirStdout, bool redirStderr)
{
	int ret = -1;
	struct sockaddr_in srv_addr;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (!sock) {
		return ret;
	}

	bzero(&srv_addr, sizeof srv_addr);
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr = __3dslink_host;
	srv_addr.sin_port = htons(LINK3DS_COMM_PORT);

	ret = connect(sock, (struct sockaddr *) &srv_addr, sizeof(srv_addr));
	if (ret != 0) {
		close(sock);
		return -1;
	}

	if (redirStdout) {
		// redirect stdout
		fflush(stdout);
		dup2(sock, STDOUT_FILENO);
	}

	if (redirStderr) {
		// redirect stderr
		fflush(stderr);
		dup2(sock, STDERR_FILENO);
	}

	return sock;
}
