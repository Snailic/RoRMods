#include "Net.h"

void Net::create(int port)
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 0), &WSAData);

	m_server.socket = socket(AF_INET, SOCK_STREAM, 0);
	m_server.addr.sin_addr.s_addr = htonl(INADDR_ANY);
	m_server.addr.sin_family = AF_INET;
	m_server.addr.sin_port = htons(port);

	bind(m_server.socket, (SOCKADDR*)&m_server.addr, sizeof(m_server.addr));
	listen(m_server.socket, 0);

	int sizeof_csin = sizeof(m_client.addr);

	m_client.socket = accept(m_server.socket, (SOCKADDR*)&m_client.addr, &sizeof_csin);	

	m_isServer = true;
}

void Net::conn(std::string ip, int port)
{
	m_client.server = gethostbyname(ip.c_str());
	m_server.socket = socket(AF_INET, SOCK_STREAM, 0);

	if (m_client.server == NULL)
		printf("[CLIENT] [ERROR] error creating server link\n");

	m_server.addr.sin_family = AF_INET;
	memcpy((char*)m_client.server->h_addr, (char*)&m_server.addr.sin_addr.s_addr,
		m_client.server->h_length);
	m_server.addr.sin_port = htons(port);

	connect(m_server.socket, (SOCKADDR*)&m_server.addr, sizeof(m_server.addr));
	m_isServer = false;
}

void Net::sendDouble(double val)
{
	char buffer[256];
	std::string valStr = std::to_string(val);
	strcpy(buffer, valStr.c_str());

	send(m_client.socket ? m_isServer : m_server.socket, buffer, sizeof(buffer), 0);
}

double Net::recvDouble()
{
	char buffer[256];
	
	recv(m_client.socket ? m_isServer : m_server.socket, buffer, sizeof(buffer), 0);

	return atof(buffer);
}

void Net::clear() { WSACleanup(); }