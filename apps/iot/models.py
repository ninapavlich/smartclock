import os

from django.conf import settings
from django.core.files.storage import FileSystemStorage
from django.db import models
from django.utils import timezone


class OverwriteStorage(FileSystemStorage):

    def get_available_name(self, name, max_length=None):
        """Returns a filename that's free on the target storage system, and
        available for new content to be written to.

        Found at http://djangosnippets.org/snippets/976/

        This file storage solves overwrite on upload problem. Another
        proposed solution was to override the save method on the model
        like so (from https://code.djangoproject.com/ticket/11663):

        def save(self, *args, **kwargs):
            try:
                this = MyModelName.objects.get(id=self.id)
                if this.MyImageFieldName != self.MyImageFieldName:
                    this.MyImageFieldName.delete()
            except: pass
            super(MyModelName, self).save(*args, **kwargs)
        """
        # If the filename already exists, remove it as if it was a true file
        # system
        if self.exists(name):
            os.remove(os.path.join(settings.MEDIA_ROOT, name))
        return name

def alarm_song_upload_path(instance, filename):
    filename, file_extension = os.path.splitext(filename)
    converted_filename = u'%s%s' % (instance.pk, file_extension)
    return os.path.join('alarm', converted_filename)

class Alarm(models.Model):
    name = models.CharField(max_length=255)
    
    upload = models.FileField(upload_to=alarm_song_upload_path, blank=True, null=True, storage=OverwriteStorage())

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
    

