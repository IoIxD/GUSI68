
#include <PPCToolbox.h>

struct sockaddr_ppc {
	sa_family_t		sppc_family;	/* AF_PPC */
	LocationNameRec	sppc_location;
	PPCPortRec		sppc_port;
};
