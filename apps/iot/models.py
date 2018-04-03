import os

from django.conf import settings
from django.core.files.storage import FileSystemStorage
from django.db import models
from django.utils import timezone

from cms.storage_backends import PublicMediaStorage


def alarm_song_upload_path(instance, filename):
    filename, file_extension = os.path.splitext(filename)
    converted_filename = u'%s%s' % (filename.lower(), file_extension)
    return os.path.join('alarm', converted_filename)

class Alarm(models.Model):
    name = models.CharField(max_length=255)
    
    upload = models.FileField(upload_to=alarm_song_upload_path, blank=True, null=True, storage=PublicMediaStorage())

    time = models.TimeField(blank=True)
    active = models.BooleanField(default=True)
    allow_snooze = models.BooleanField(default=True)

    repeat_mon = models.BooleanField(default=True)
    repeat_tue = models.BooleanField(default=True)
    repeat_wed = models.BooleanField(default=True)
    repeat_thu = models.BooleanField(default=True)
    repeat_fri = models.BooleanField(default=True)
    repeat_sat = models.BooleanField(default=True)
    repeat_sun = models.BooleanField(default=True)

    def __str__(self):
        return 'Alarm %s at %s' % (self.name, self.time)
    

