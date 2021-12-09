from rest_api.geo_coordinate_point import GeoCoordinatePoint
from rest_api import models


class ParkSearcher:
    # parkings around this distance from the input point are treated as nearest ones
    near_boundary_meters = 100

    def __init__(self, near_boundary_meters=None):
        if near_boundary_meters is not None:
            self.near_boundary_meters = int(near_boundary_meters)

    def find_nearest_free_parking_places(self, input_latitude, input_longitude):
        """
        Finds free parking place near the specified point given in geographical coordinates.
        :param input_latitude: the latitude of the input point
        :param input_longitude: the longitude of the input point
        :return: the list of free parking places found sorted by distance from the input point
        """

        input_point = GeoCoordinatePoint(input_latitude, input_longitude)
        nearest_parkings = []

        # find nearest parkings
        for parking in models.Parking.objects.all():
            parking_center = GeoCoordinatePoint(parking.latitude, parking.longitude)
            distance = GeoCoordinatePoint.calculate_distance(input_point, parking_center)
            if distance <= self.near_boundary_meters:
                nearest_parkings.append(parking)

        nearest_parking_places = []

        # find free places on those parkings
        for parking in nearest_parkings:
            for parking_place in parking.free_parking_places.all():
                parking_place_center = GeoCoordinatePoint(parking_place.latitude, parking_place.longitude)
                distance = GeoCoordinatePoint.calculate_distance(input_point, parking_place_center)
                nearest_parking_places.append((parking_place, distance))

        # sort by distance
        return [p[0] for p in sorted(nearest_parking_places, key=lambda place: place[1])]
