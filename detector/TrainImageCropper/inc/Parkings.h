#ifndef Parkings_H
#define Parkings_H

#include "ParkingRectangle.h"

#include <vector>

class Parkings
{
public:
	class ParseException {
	public:
		std::string message() const {
			return "error parsing cropper's config file";
		}
	};

public:
	Parkings(const std::string & _strConfigFile);

	const std::vector<ParkingRectangle> & getParkings() const {return m_parkings;}

private:
	std::vector<ParkingRectangle> m_parkings;
};

#endif // Parkings_H
