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
                'last_synchronized'
            )
      }),
      
    )
    readonly_fields = ['sound_md5']
    list_display = ['name', 'time', 'allow_snooze', 'repeat_mon', 'repeat_tue', 'repeat_wed', 'repeat_thu', 'repeat_fri', 'repeat_sat', 'repeat_sun', 'active','last_synchronized']
    list_filter = ['allow_snooze', 'active', 'repeat_mon', 'repeat_tue', 'repeat_wed', 'repeat_thu', 'repeat_fri', 'repeat_sat', 'repeat_sun']

