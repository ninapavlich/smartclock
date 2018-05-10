from django import forms
from django.conf import settings
from django.contrib import admin

from .models import *

@admin.register(Alarm)
class AlarmAdmin(admin.ModelAdmin):
    fieldsets = (
      ('Alarm info', {
            'fields': (
                'name',
                ('sound', 'sound_md5'),
                ('time','allow_snooze','active'),
                ('repeat_mon','repeat_tue','repeat_wed','repeat_thu','repeat_fri','repeat_sat','repeat_sun'),
                ('next_alarm_time', 'last_stopped_time', 'last_snoozed_time')
            )
      }),  
    )
    search_fields = ['name', 'sound', 'sound_md5']
    readonly_fields = ['sound_md5', 'next_alarm_time', 'last_stopped_time', 'last_snoozed_time']
    list_display = ['name', 'time', 'allow_snooze', 'repeat_mon', 'repeat_tue', 'repeat_wed', 'repeat_thu', 'repeat_fri', 'repeat_sat', 'repeat_sun', 'active', 'next_alarm_time']
    list_filter = ['allow_snooze', 'active', 'repeat_mon', 'repeat_tue', 'repeat_wed', 'repeat_thu', 'repeat_fri', 'repeat_sat', 'repeat_sun']

@admin.register(AlarmClient)
class AlarmClientAdmin(admin.ModelAdmin):
    prepopulated_fields = {"slug": ("name",)}
    list_display = ['name', 'slug', 'alarm_delay']
    
    search_fields = ['name', 'slug']


@admin.register(AlarmClientAlarmSynchronized)
class AlarmClientAlarmSynchronizedAdmin(admin.ModelAdmin):
    readonly_fields = ['last_synchronized']    