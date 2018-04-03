from django.conf import settings
from django.urls import reverse

from django_filters.rest_framework import DjangoFilterBackend

from rest_framework import serializers, viewsets, permissions, filters
from rest_framework import viewsets, permissions, filters
from django_filters.rest_framework import DjangoFilterBackend

from .models import Alarm

class AlarmSerializer(serializers.ModelSerializer):

    class Meta:
        model = Alarm
        fields = '__all__'


class AlarmViewSet(viewsets.ModelViewSet):

    filter_backends = (DjangoFilterBackend, filters.OrderingFilter)
    filter_fields = ['active']
    
    queryset = Alarm.objects.all()
    serializer_class = AlarmSerializer
    permission_classes = (permissions.IsAuthenticated, )
