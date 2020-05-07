#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>
#include <miniupnpc/portlistingparse.h>
#include <miniupnpc/upnperrors.h>
#ifdef __cplusplus
}
#endif

/* protofix() checks if protocol is "UDP" or "TCP"
 * returns NULL if not */
const char * protofix(const char * proto)
{
	static const char proto_tcp[4] = { 'T', 'C', 'P', 0};
	static const char proto_udp[4] = { 'U', 'D', 'P', 0};
	int i, b;
	for(i=0, b=1; i<4; i++)
		b = b && (   (proto[i] == proto_tcp[i])
		          || (proto[i] == (proto_tcp[i] | 32)) );
	if(b)
		return proto_tcp;
	for(i=0, b=1; i<4; i++)
		b = b && (   (proto[i] == proto_udp[i])
		          || (proto[i] == (proto_udp[i] | 32)) );
	if(b)
		return proto_udp;
	return 0;
}

/* Test function
 * 1 - get connection type
 * 2 - get extenal ip address
 * 3 - Add port mapping
 * 4 - get this port mapping from the IGD */
static int SetRedirectAndTest(struct UPNPUrls * urls,
			       struct IGDdatas * data,
			       const char * iaddr,
			       const char * iport,
			       const char * eport,
			       const char * proto,
			       const char * leaseDuration,
			       const char * remoteHost,
			       const char * description,
			       int addAny)
{
	char externalIPAddress[40];
	char intClient[40];
	char intPort[6];
	char reservedPort[6];
	char duration[16];
	int r;

	if(!iaddr || !iport || !eport || !proto)
	{
		fprintf(stderr, "Wrong arguments\n");
		return -1;
	}
	proto = protofix(proto);
	if(!proto)
	{
		fprintf(stderr, "invalid protocol\n");
		return -1;
	}

	r = UPNP_GetExternalIPAddress(urls->controlURL,
				      data->first.servicetype,
				      externalIPAddress);
	if(r!=UPNPCOMMAND_SUCCESS)
		printf("GetExternalIPAddress failed.\n");
	else
		printf("ExternalIPAddress = %s\n", externalIPAddress);

	if (addAny) {
		r = UPNP_AddAnyPortMapping(urls->controlURL, data->first.servicetype,
					   eport, iport, iaddr, description,
					   proto, remoteHost, leaseDuration, reservedPort);
		if(r==UPNPCOMMAND_SUCCESS)
			eport = reservedPort;
		else
			printf("AddAnyPortMapping(%s, %s, %s) failed with code %d (%s)\n",
			       eport, iport, iaddr, r, strupnperror(r));
	} else {
		r = UPNP_AddPortMapping(urls->controlURL, data->first.servicetype,
					eport, iport, iaddr, description,
					proto, remoteHost, leaseDuration);
		if(r!=UPNPCOMMAND_SUCCESS) {
			printf("AddPortMapping(%s, %s, %s) failed with code %d (%s)\n",
			       eport, iport, iaddr, r, strupnperror(r));
			return -2;
	}
	}

	r = UPNP_GetSpecificPortMappingEntry(urls->controlURL,
					     data->first.servicetype,
					     eport, proto, remoteHost,
					     intClient, intPort, NULL/*desc*/,
					     NULL/*enabled*/, duration);
	if(r!=UPNPCOMMAND_SUCCESS) {
		printf("GetSpecificPortMappingEntry() failed with code %d (%s)\n",
		       r, strupnperror(r));
		return -2;
	} else {
		printf("InternalIP:Port = %s:%s\n", intClient, intPort);
		printf("external %s:%s %s is redirected to internal %s:%s (duration=%s)\n",
		       externalIPAddress, eport, proto, intClient, intPort, duration);
	}
	return 0;
}

int main()
{
    struct UPNPDev * devlist = 0;
    unsigned char ttl = 2;
    const char * multicastif = 0;
    const char * minissdpdpath = 0;
    int localport = UPNP_LOCAL_PORT_ANY;
	const char * port = "1975";
    int ipv6 = 0;
    int error = 0;
    int i;
    char lanaddr[64] = "unset";	/* my ip address on the LAN */
	int retcode = 0;
	const char * proto = "TCP";
	const char * description = 0;

    if (devlist = upnpDiscover(2000, multicastif, minissdpdpath, localport, ipv6, ttl, &error))
    {
        struct UPNPDev * device;
		struct UPNPUrls urls;
		struct IGDdatas data;
		if(devlist)
		{
			printf("List of UPNP devices found on the network :\n");
			for(device = devlist; device; device = device->pNext)
			{
				printf(" desc: %s\n st: %s\n\n",
					   device->descURL, device->st);
			}
		}
        i = 1;
		if (i = UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr)))
		{
			switch(i) {
			case 1:
				printf("Found valid IGD : %s\n", urls.controlURL);
				break;
			case 2:
				printf("Found a (not connected?) IGD : %s\n", urls.controlURL);
				printf("Trying to continue anyway\n");
				break;
			case 3:
				printf("UPnP device found. Is it an IGD ? : %s\n", urls.controlURL);
				printf("Trying to continue anyway\n");
				break;
			default:
				printf("Found device (igd ?) : %s\n", urls.controlURL);
				printf("Trying to continue anyway\n");
			}
			printf("Local LAN ip address : %s\n", lanaddr);
			#if 0
			printf("getting \"%s\"\n", urls.ipcondescURL);
			descXML = miniwget(urls.ipcondescURL, &descXMLsize);
			if(descXML)
			{
				/*fwrite(descXML, 1, descXMLsize, stdout);*/
				free(descXML); descXML = NULL;
			}
			#endif

			if (SetRedirectAndTest(&urls, &data,
						   lanaddr, port,
						   port, proto,
						   "0",
						   NULL,
						   description, 0) < 0)
					retcode = 2;
			
			FreeUPNPUrls(&urls);
		}
		else
		{
			fprintf(stderr, "No valid UPNP Internet Gateway Device found.\n");
			retcode = 1;
		}
		freeUPNPDevlist(devlist); devlist = 0;
	}
	else
	{
		fprintf(stderr, "No IGD UPnP Device found on the network !\n");
		retcode = 1;
	}
	return retcode;
}
