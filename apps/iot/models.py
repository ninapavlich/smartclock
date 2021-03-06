import os
import pytz
import datetime
from datetime import timedelta

from django.conf import settings
from django.core.files.storage import FileSystemStorage
from django.core.exceptions import ObjectDoesNotExist
from django.db import models
from django.db.models.signals import post_save
from django.dispatch import receiver
from django.utils import timezone
from django.utils.formats import date_format
from django.utils.module_loading import import_string

from cms.utils.md5_file_field import MD5FileField
from cms.utils.mqtt_utils import send_model_message


def get_storage():
    return import_string(settings.DEFAULT_FILE_STORAGE)()

def alarm_song_upload_path(instance, filename):
    filename, file_extension = os.path.splitext(filename)
    converted_filename = u'%s%s' % (filename.lower(), file_extension)
    return os.path.join('alarm', converted_filename)

def datetime_to_json(dt):
    """ Utility for facilitating date time parsing on lower-level programs"""
    if not dt:
        return {
            'year':0,
            'month':0,
            'day':0,
            'hour':0,
            'minute':0,
            'second':0,
            'timezone':0
        }

    return {
        'year':int(date_format(timezone.localtime(dt), "Y")),
        'month':int(date_format(timezone.localtime(dt), "m")),
        'day':int(date_format(timezone.localtime(dt), "d")),
        'hour':int(date_format(timezone.localtime(dt), "G")),
        'minute':int(date_format(timezone.localtime(dt), "i")),
        'second':int(date_format(timezone.localtime(dt), "s")),
        'timezone':(date_format(timezone.localtime(dt), "O"))
    }

def now_localized():
    tz = pytz.timezone(settings.TIME_ZONE)
    now = tz.localize(datetime.datetime.now())
    return now

class Alarm(models.Model):
    name = models.CharField(max_length=255, blank=True, null=True)
    
    sound = MD5FileField(upload_to=alarm_song_upload_path, blank=True, null=True, storage=get_storage(), md5_field='sound_md5')
    sound_md5 = models.CharField(max_length=255, blank=True, null=True)

    time = models.TimeField(null=True, blank=True)
    active = models.BooleanField(default=True)
    allow_snooze = models.BooleanField(default=True)

    next_alarm_time = models.DateTimeField(null=True, blank=True)
    last_stopped_time = models.DateTimeField(null=True, blank=True)
    last_snoozed_time = models.DateTimeField(null=True, blank=True)

    repeat_mon = models.BooleanField(default=True)
    repeat_tue = models.BooleanField(default=True)
    repeat_wed = models.BooleanField(default=True)
    repeat_thu = models.BooleanField(default=True)
    repeat_fri = models.BooleanField(default=True)
    repeat_sat = models.BooleanField(default=True)
    repeat_sun = models.BooleanField(default=True)

    def snooze(self):
        self.last_snoozed_time = timezone.now()
        self.save()

    def stop(self):
        self.last_stopped_time = timezone.now()
        self.save()

    def get_weekdays(self):
        output = []
        if self.repeat_mon:
            output.append(0)
        if self.repeat_tue:
            output.append(1)
        if self.repeat_wed:
            output.append(2)
        if self.repeat_thu:
            output.append(3)
        if self.repeat_fri:
            output.append(4)
        if self.repeat_sat:
            output.append(5)
        if self.repeat_sun:
            output.append(6)

        return output

    def calculate_next_alarm_time(self):
        if not self.time:
            return

        active_weekdays = self.get_weekdays()
        if len(active_weekdays) == 0:
            return

        now = now_localized()

        now_at_time = now.replace(hour=self.time.hour, minute=self.time.minute, second=self.time.second)
        
        # Apply time of day to now. Is that in the past? then add 24 hours.
        if now > now_at_time:
            now_at_time += timedelta(days=1)
        
        # Make sure its set to the right day of week
        current_weekday = now_at_time.weekday()
        while current_weekday not in active_weekdays:
            now_at_time += timedelta(days=1)
            current_weekday = now_at_time.weekday()
    
        self.next_alarm_time = now_at_time


    def save(self, *args, **kwargs):
        self.calculate_next_alarm_time()
        super().save(*args, **kwargs)

    @property
    def sound_filename_83(self):
        if self.sound:
            filename, file_extension = os.path.splitext(self.sound.url)
            return ('%s%s'%(str(self.pk).zfill(8), file_extension)).upper()
        return ''

    @property
    def sound_filename_md5(self):
        if self.sound:
            return ('%s%s'%(str(self.pk).zfill(8), '.md5')).upper()
        return ''

        

    def __str__(self):
        return 'Alarm %s at %s' % (self.name, self.time)
    

class AlarmClient(models.Model):
    name = models.CharField(max_length=255)
    slug = models.SlugField(max_length=255, unique=True, primary_key=True)

    alarm_delay = models.IntegerField(default=0, help_text="Length of time to delay before triggering this alarm client")

    @property
    def current_time(self):
        return datetime_to_json(timezone.now())

    @property
    def synchronized_alarms(self):
        all_alarms = Alarm.objects.all()
        items = []
        for alarm in all_alarms:
            item, created = AlarmClientAlarmSynchronized.objects.get_or_create(client=self,alarm=alarm)
            items.append({
                'alarm_pk':item.alarm.pk,
                'alarm_name':item.alarm.name,
                'last_synchronized':datetime_to_json(item.last_synchronized)
            })

        return items

    def synchronize(self, alarm_ids_raw):
        alarm_ids = alarm_ids_raw.split(',')
        for alarm_id in alarm_ids:
            alarm = None
            if alarm_id:
                try:
                    alarm = Alarm.objects.get(pk=alarm_id)
                except ObjectDoesNotExist:
                    pass

                if alarm:
                    item, created = AlarmClientAlarmSynchronized.objects.get_or_create(client=self,alarm=alarm)
                    item.synchronize()
            

    def __str__(self):
        return 'Alarm Client %s' % (self.name)


class AlarmClientAlarmSynchronized(models.Model):
    alarm = models.ForeignKey('Alarm', on_delete=models.CASCADE)
    client = models.ForeignKey('AlarmClient', on_delete=models.CASCADE)
    
    last_synchronized = models.DateTimeField(null=True, blank=True)

    @property
    def current_time(self):
        return datetime_to_json(timezone.now())

    @property
    def last_synchronized_time(self):
        return datetime_to_json(self.last_synchronized)

    def synchronize(self):
        self.last_synchronized = timezone.now()
        self.save()
    
    def __str__(self):
        return '%s on %s - %s' % (self.alarm, self.client, self.last_synchronized)        


@receiver(post_save, sender=Alarm)
def alarm_updated(sender, instance, created, **kwargs):
    from .api import AlarmSerializer
    send_model_message(instance, AlarmSerializer, 'updated')
    