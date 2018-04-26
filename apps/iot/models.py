import os

from django.conf import settings
from django.core.files.storage import FileSystemStorage
from django.db import models
from django.utils import timezone
from django.utils.module_loading import import_string

from cms.utils.md5_file_field import MD5FileField


def get_storage():
    return import_string(settings.DEFAULT_FILE_STORAGE)()

def alarm_song_upload_path(instance, filename):
    filename, file_extension = os.path.splitext(filename)
    converted_filename = u'%s%s' % (filename.lower(), file_extension)
    return os.path.join('alarm', converted_filename)

class Alarm(models.Model):
    name = models.CharField(max_length=255, blank=True, null=True)
    
    sound = MD5FileField(upload_to=alarm_song_upload_path, blank=True, null=True, storage=get_storage(), md5_field='sound_md5')
    sound_md5 = models.CharField(max_length=255, blank=True, null=True)

    time = models.TimeField(null=True, blank=True)
    active = models.BooleanField(default=True)
    allow_snooze = models.BooleanField(default=True)

    last_synchronized = models.DateTimeField(null=True, blank=True)

    repeat_mon = models.BooleanField(default=True)
    repeat_tue = models.BooleanField(default=True)
    repeat_wed = models.BooleanField(default=True)
    repeat_thu = models.BooleanField(default=True)
    repeat_fri = models.BooleanField(default=True)
    repeat_sat = models.BooleanField(default=True)
    repeat_sun = models.BooleanField(default=True)

    def synchronize(self):
        self.last_synchronized = timezone.now()
        self.save()

    def __str__(self):
        return 'Alarm %s at %s' % (self.name, self.time)
    

