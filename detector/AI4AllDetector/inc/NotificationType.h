#ifndef NotificationType_H
#define NotificationType_H

#include "AI4AllDetectorException.h"

#include <string>

enum NotificationType
{
  APPEARANCE,
  PRESENCE,
  ABSENCE
};

inline NotificationType string2notificationType(const std::string & _str) {
	if (_str == "appearance")
		return APPEARANCE;
	if (_str == "presence")
		return PRESENCE;
	if (_str == "absence")
		return ABSENCE;

	throw BadNotificationTypeException(_str);
}

#endif // NotificationType_H
