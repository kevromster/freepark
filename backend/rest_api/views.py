from rest_framework import permissions
from rest_framework import status
from rest_framework import viewsets
from rest_framework.decorators import detail_route, api_view
from rest_framework.response import Response
from django.contrib.auth.models import User

from rest_api import models
from rest_api import serializers
from rest_api.permissions import IsOwningCameraOrReadOnly
from rest_api.park_searcher import ParkSearcher


class CameraViewSet(viewsets.ReadOnlyModelViewSet):
    """
    Provides information about existing cameras.
    """
    queryset = User.objects.all()
    serializer_class = serializers.CameraSerializer


class ParkingViewSet(viewsets.ModelViewSet):
    """
    Provides and edits information about existing parkings.
    """
    queryset = models.Parking.objects.all()
    serializer_class = serializers.ParkingSerializer
    permission_classes = (permissions.IsAuthenticatedOrReadOnly, IsOwningCameraOrReadOnly)

    def perform_create(self, serializer):
        serializer.save(owning_camera=self.request.user)

    @detail_route(methods=['post'], url_path='set-free-parking-places')
    def set_free_parking_places(self, request, pk=None):
        """
        Sets free parking places in the specified parking erasing previous ones.
        Accepts list of new parking places data in json format.
        """
        parking = self.get_object()
        serializer = serializers.FreeParkingPlaceSerializer(data=request.data, context={'request': request}, many=True)

        if serializer.is_valid():
            parking.free_parking_places.all().delete()
            parking.save()
            serializer.save(owning_parking=parking)
            return Response(serializer.data)
        else:
            return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)


class FreeParkingPlaceViewSet(viewsets.ReadOnlyModelViewSet):
    """
    Provides information about free parking places on the specified parking.
    """
    queryset = models.FreeParkingPlace.objects.all()
    serializer_class = serializers.FreeParkingPlaceSerializer


@api_view(http_method_names=['GET'])
def find_nearest_free_parking_places(request):
    """
    Returns free parking place found near the given point.
    Expected input parameters: latitude, longitude, boundary (optional, boundary around which to search parkings)
    """
    latitude = request.query_params.get('latitude', None)
    longitude = request.query_params.get('longitude', None)
    boundary = request.query_params.get('boundary', None)

    if latitude is None or longitude is None:
        return Response({"content": "bad input parameters"}, status=status.HTTP_400_BAD_REQUEST)

    free_parking_places = ParkSearcher(boundary).find_nearest_free_parking_places(latitude, longitude)
    serializer = serializers.FreeParkingPlaceSerializer(free_parking_places, context={'request': request}, many=True)
    return Response(serializer.data)
