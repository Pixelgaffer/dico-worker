#include <error.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <string>
using namespace std;

#include <do-task.pb.h>
#include <handshake.pb.h>
#include <self-describing-message.pb.h>
using namespace dicoprotos;

template<typename MessageType>
string serialize(MessageType m, SelfDescribingMessage_MessageType mt)
{
	SelfDescribingMessage sdm;
	sdm.set_type(mt);
	string d = m.SerializeAsString();
	sdm.set_data(d.data(), d.size());
	d = sdm.SerializeAsString();
	int s = d.size();
	d = (char)( s        & 0xFF) + d;
	d = (char)((s >>  8) & 0xFF) + d;
	d = (char)((s >> 16) & 0xFF) + d;
	d = (char)((s >> 24) & 0xFF) + d;
	cout << "header: " << (int)d[0] << " " << (int)d[1] << " " << (int)d[2] << " " << (int)d[3] << " (=" << s << ")" << endl;
	return d;
}

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
	string d = serialize(h, SelfDescribingMessage::HANDSHAKE);
	if (write(sockfd, d.data(), d.length()) == -1)
	{
		perror("Failed to send handshake");
		close(sockfd);
		return 1;
	}
	cerr << "Successfully sent handshake" << endl;
	
	while (true)
	{
		int len = 0;
		char p, *buf = 0;
		int read;
		for (int i = 0; i<4 || len>i-4; i++)
		{
			if ((read = recv(sockfd, &p, 1, 0)) < 0)
				break;
			if (i < 4)
			{
				len = len | (p << (24 - 8*i));
				continue;
			}
			
			if (!buf)
				buf = new char[len];
			buf[i-4] = p;
		}
		
		SelfDescribingMessage sdm;
		sdm.ParseFromArray(buf, len);
		switch (sdm.type())
		{
		case SelfDescribingMessage::DO_TASK: {
				DoTask task;
				task.ParseFromString(sdm.data());
				cout << "received task with id " << task.id() << endl;
			}
			break;
		default:
			cerr << "Unknown message type received: " << sdm.type() << endl;
		}
	}
}

/*
[12:32:38] Flo B.: Der Worker verbindet sich zu port 7778 und schickt nen Handshake.
Dann wartet er auf ein DoTask Paket und fängt den Task an.
Falls der Task gefailed hat er ein TaskStatusUpdate.FAILED
Wenn der Task fertig ist schickt er ein TaskResult Paket.
 */
