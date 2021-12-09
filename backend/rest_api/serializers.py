from rest_framework import serializers
from django.contrib.auth.models import User
from rest_api import models
from rest_api.base64_image_field import Base64ImageField


class CameraSerializer(serializers.HyperlinkedModelSerializer):
    parkings = serializers.HyperlinkedRelatedField(many=True, view_name='parking-detail', read_only=True)

    class Meta:
        model = User
        fields = ('url', 'id', 'username', 'parkings')


class ParkingSerializer(serializers.HyperlinkedModelSerializer):
    owning_camera = serializers.ReadOnlyField(source='owning_camera.username')
    free_parking_places = serializers.HyperlinkedRelatedField(many=True, view_name='freeparkingplace-detail', read_only=True)

    class Meta:
        model = models.Parking
        fields = ('id', 'name', 'date_created', 'date_modified', 'owning_camera', 'latitude', 'longitude', 'free_parking_places')
        read_only_fields = ('date_created', 'date_modified')


class FreeParkingPlaceSerializer(serializers.HyperlinkedModelSerializer):
    owning_parking = serializers.HyperlinkedRelatedField(view_name='parking-detail', read_only=True)
    image = Base64ImageField(required=False, max_length=None, use_url=True)

    class Meta:
        model = models.FreeParkingPlace
        fields = ('id', 'owning_parking', 'image', 'latitude', 'longitude', 'parking_complexity')
