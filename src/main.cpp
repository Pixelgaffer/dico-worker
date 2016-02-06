#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <string>
using namespace std;

#include <handshake.pb.h>
#include <self-describing-message.pb.h>
using namespace dicoprotos;

int main(int argc, char **argv)
{
	string host;
	string port = "7778";
	if (argc < 2)
	{
		cout << "Usage: " << argv[0] << " <host> [<port>]" << endl;
		return 1;
	}
	host = argv[1];
	if (argc > 2)
		port = argv[2];
	
	cerr << "Connecting to " << host << ":" << port << endl;
	struct addrinfo hints;
	struct addrinfo *result, *rp = 0;
	int sockfd;
	int ret;
	// lookup address
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family   = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags    = 0;
	hints.ai_protocol = 0;
	if (ret = (getaddrinfo(host.data(), port.data(), &hints, &result)) != 0)
	{
		cerr << "Error while looking up " << host << ": " << gai_strerror(ret) << endl;
		return 1;
	}
	for (rp = result; rp; rp = rp->ai_next)
	{
		sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sockfd == -1)
			continue;
		if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;
		close(sockfd);
	}
	if (!rp)
	{
		cerr << "Error while connecting to " << host << ":" << port << ": " << gai_strerror(ret) << endl;
		return 1;
	}
	freeaddrinfo(result);
	
	// send handshake
	Handshake h;
	h.set_runs_tasks(true);
	string hd = h.SerializeAsString();
	SelfDescribingMessage sdm;
	sdm.set_type(SelfDescribingMessage::HANDSHAKE);
	sdm.set_data(hd.data(), hd.length());
	if (!sdm.SerializeToFileDescriptor(sockfd))
	{
		cerr << "Failed to send handshake" << endl;
		close(sockfd);
		return 1;
	}
	cerr << "Successfully sent handshake" << endl;
}
