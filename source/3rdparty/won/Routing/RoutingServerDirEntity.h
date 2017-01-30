#ifndef ROUTINGSERVERDIRENTITY_H
#define ROUTINGSERVERDIRENTITY_H

#include "msg/Dir/DirEntity.h"

struct RoutingServerDirEntity : public WONMsg::DirEntity {
	bool mIsPasswordProtected;   // registration requires a password
	bool mIsFiltered;            // dirty word filtering is turned on

	unsigned short mClientCount; // number of clients registered

	RoutingServerDirEntity();
	void Parse(); // unpack dataobjects into fields listed above
};

#endif // ROUTINGSERVERDIRENTITY_H