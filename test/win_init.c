#ifndef _WIN32_INNNT
#define _WIN332_WINNT 0x0600
#endif

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>

int main(void){
	WSADATA wsa_data;
	int result;

	result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result){
		printf("WSAStartup() failed: %d\n", result);
		return EXIT_FAILURE;
	}

	DWORD asize = 20000;
	PIP_ADAPTER_ADDRESSES adapters;
	do {
		adapters = (PIP_ADAPTER_ADDRESSES)malloc(asize);

		if (!adapters){
			printf("Couldn't allocated %ld bytes for adapters\n", asize);
			WSACleanup();
			return EXIT_FAILURE;
		}

		int r = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, 0, adapters, &asize);
		if (r == ERROR_BUFFER_OVERFLOW){
			printf("GetAdaptersAddresses() wants %ld bytes\n", asize);
			free(adapters);		
		}
		else if (r == ERROR_SUCCESS){
			break;
		}
		else {
			printf("GetAdaptersAddress() failed: %d\n", r);
			free(adapters);
			WSACleanup();
			return EXIT_FAILURE;
		}
	} while (!adapters);

	PIP_ADAPTER_ADDRESSES adapter = adapters;
	while (adapter){
		printf("Adapter name: %s\n", adapter->FriendlyName);

		PIP_ADAPTER_UNICAST_ADDRESS address = adapter->FirstUnicastAddress;
		while (address){
			printf("\t%s\n", address->Address.lpSockaddr->sa_family == AF_INET ? "IPv4" : "IPv6");

			char ap[100];
			getnameinfo(address->Address.lpSockaddr, address->Address.iSockaddrLength, ap, sizeof(ap), 0 , 0, NI_NUMERICHOST);
			printf("\t%s\n", ap);
			address = address->Next;
		}
		adapter = adapter->Next;
	}

	free(adapters);
	WSACleanup();
	printf("\nOK\n");
	return EXIT_SUCCESS;
}