from django.conf import settings
from django.urls import reverse

from django_filters.rest_framework import DjangoFilterBackend

from rest_framework import serializers, viewsets, permissions, filters
from rest_framework import viewsets, permissions, filters
from rest_framework.decorators import detail_route
from rest_framework.response import Response

from django_filters.rest_framework import DjangoFilterBackend

from .models import Alarm, AlarmClient

class AlarmSerializer(serializers.ModelSerializer):

    class Meta:
        model = Alarm
        fields = ['url', 'pk', 'name', 'sound', 'sound_md5', 'time', 'active', 
                  'allow_snooze', 'next_alarm_time', 'last_stopped_time', 
                  'last_snoozed_time', 'repeat_mon','repeat_tue','repeat_wed',
                  'repeat_thu','repeat_fri','repeat_sat','repeat_sun',
                  'sound_filename_83', 'sound_filename_md5']


class AlarmViewSet(viewsets.ModelViewSet):

    filter_backends = (DjangoFilterBackend, filters.OrderingFilter)
    filter_fields = ['active']
    
    queryset = Alarm.objects.all()
    serializer_class = AlarmSerializer
    permission_classes = (permissions.IsAuthenticated, )


    @detail_route(methods=['post'])
    def snooze(self, request, pk=None):
        alarm = self.get_object()
        alarm.snooze()
        serializer = self.get_serializer(alarm, many=False)
        return Response(serializer.data)

    @detail_route(methods=['post'])
    def stop(self, request, pk=None):
        alarm = self.get_object()
        alarm.stop()
        serializer = self.get_serializer(alarm, many=False)
        return Response(serializer.data)


class AlarmClientSerializer(serializers.ModelSerializer):

    class Meta:
        model = AlarmClient
        fields = ['url', 'name', 'slug', 'alarm_delay', 'last_synchronized']

class AlarmClientViewSet(viewsets.ModelViewSet):

    filter_backends = (DjangoFilterBackend, filters.OrderingFilter)
    filter_fields = []
    
    queryset = AlarmClient.objects.all()
    serializer_class = AlarmClientSerializer
    permission_classes = (permissions.IsAuthenticated, )


    @detail_route(methods=['post'])
    def synchronize(self, request, pk=None):
        alarm = self.get_object()
        alarm.synchronize()
        serializer = self.get_serializer(alarm, many=False)
        return Response(serializer.data)        

