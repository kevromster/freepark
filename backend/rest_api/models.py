import os

from django.db import models
from django.db.models.signals import post_save
from django.contrib.auth.models import User
from django.dispatch import receiver
from django.utils.translation import gettext as _
from rest_framework.authtoken.models import Token
from threading import Timer


class Parking(models.Model):
    name = models.CharField(max_length=255, blank=True, default='')
    date_created = models.DateTimeField(auto_now_add=True)
    date_modified = models.DateTimeField(auto_now=True)

    # center point coordinates of the parking
    latitude = models.DecimalField(_('Latitude'), max_digits=10, decimal_places=6)
    longitude = models.DecimalField(_('Longitude'), max_digits=10, decimal_places=6)

    owning_camera = models.ForeignKey('auth.User', related_name='parkings', on_delete=models.CASCADE)

    def __str__(self):
        return "{}".format(self.name)

    class Meta:
        ordering = ('date_created',)


def images_path(instance, filename):
    """
    Returns path where free parking place images will be stored.
    """
    parking_name = instance.owning_parking.name
    if not parking_name:
        parking_name = '.'
    return 'parking_places/{0}/{1}'.format(parking_name.replace(' ', '_'), filename)


class FreeParkingPlace(models.Model):
    owning_parking = models.ForeignKey(Parking, related_name='free_parking_places', on_delete=models.CASCADE)
    image = models.ImageField(upload_to=images_path, blank=True, null=True)

    # center point coordinates of the parking place
    latitude = models.DecimalField(_('Latitude'), max_digits=10, decimal_places=6)
    longitude = models.DecimalField(_('Longitude'), max_digits=10, decimal_places=6)

    PARKING_COMPLEXITY = (
        (0, 'unknown'),
        (1, 'newbie driver'),
        (2, 'accurate driver'),
        (3, 'experienced driver'),
    )
    parking_complexity = models.CharField(max_length=4, choices=PARKING_COMPLEXITY, default=0)


# This receiver handles token creation immediately a new camera is created.
@receiver(post_save, sender=User)
def create_auth_token(sender, instance=None, created=False, **kwargs):
    if created:
        Token.objects.create(user=instance)


def remove_image_file(file_name):
    if os.path.isfile(file_name):
        os.remove(file_name)


def schedule_remove_image_file(file_name):
    DELAYED_TIME_SECS = 10 * 60  # 10 minutes delay
    Timer(DELAYED_TIME_SECS, remove_image_file, [file_name]).start()


@receiver(models.signals.post_delete, sender=FreeParkingPlace)
def delete_free_parking_place_image_on_delete(sender, instance, **kwargs):
    """
    Deletes file from the filesystem when corresponding FreeParkingPlace object is deleted.
    """
    if instance.image:
        schedule_remove_image_file(instance.image.path)


@receiver(models.signals.pre_save, sender=FreeParkingPlace)
def delete_free_parking_place_image_on_change(sender, instance, **kwargs):
    """
    Deletes old file from the filesystem when corresponding FreeParkingPlace object is updated with a new file.
    """
    if not instance.pk:
        return False

    try:
        old_image = FreeParkingPlace.objects.get(pk=instance.pk).image
    except FreeParkingPlace.DoesNotExist:
        return False

    new_image = instance.image
    if not old_image == new_image:
        schedule_remove_image_file(old_image.path)
