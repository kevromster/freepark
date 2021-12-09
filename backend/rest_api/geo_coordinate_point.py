import math


class GeoCoordinatePoint:
    def __init__(self, latitude, longitude):
        self.latitude = float(latitude)
        self.longitude = float(longitude)

    def __repr__(self):
        return "GeoCoordinatePoint(%g, %g)" % (self.latitude, self.longitude)

    @staticmethod
    def calculate_distance(pt1, pt2):
        """
        Calculates distance in meters between two points specified by geographical coordinates.
        :param pt1: first coordinate point
        :param pt2: second coordinate point
        :return: distance in meters between points
        """
        earth_radius_km = 6371

        delta_latitude = math.radians(pt2.latitude - pt1.latitude)
        delta_longitude = math.radians(pt2.longitude - pt1.longitude)

        begin_latitude = math.radians(pt1.latitude)
        end_latitude = math.radians(pt2.latitude)

        a = math.sin(delta_latitude / 2) * math.sin(delta_latitude / 2) + math.sin(delta_longitude / 2) * math.sin(
            delta_longitude / 2) * math.cos(begin_latitude) * math.cos(end_latitude)

        c = 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))
        return c * earth_radius_km * 1000
