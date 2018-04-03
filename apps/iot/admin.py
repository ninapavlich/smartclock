from django import forms
from django.conf import settings
from django.contrib import admin

from .models import *

@admin.register(Alarm)
class AlarmAdmin(admin.ModelAdmin):
    pass