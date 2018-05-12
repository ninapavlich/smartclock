from django.conf import settings
from django.urls import reverse

from django_filters.rest_framework import DjangoFilterBackend

from rest_framework import serializers, viewsets, permissions, filters
from rest_framework import viewsets, permissions, filters
from rest_framework.decorators import detail_route
from rest_framework.response import Response

from django_filters.rest_framework import DjangoFilterBackend

from .models import Alarm, AlarmClient, datetime_to_json


class AlarmSerializer(serializers.ModelSerializer):

    next_alarm_time_expanded = serializers.SerializerMethodField()
    last_stopped_time_expanded = serializers.SerializerMethodField()
    last_snoozed_time_expanded = serializers.SerializerMethodField()

    def get_next_alarm_time_expanded(self, obj):
        return datetime_to_json(obj.next_alarm_time)

    def get_last_stopped_time_expanded(self, obj):
        return datetime_to_json(obj.last_stopped_time)

    def get_last_snoozed_time_expanded(self, obj):
        return datetime_to_json(obj.last_snoozed_time)

    class Meta:
        model = Alarm
        fields = ['url', 'pk', 'name', 'sound', 'sound_md5', 'time', 'active', 
                  'allow_snooze', 'next_alarm_time', 'next_alarm_time_expanded', 
                  'last_stopped_time', 'last_stopped_time_expanded',
                  'last_snoozed_time', 'last_snoozed_time_expanded', 
                  'repeat_mon','repeat_tue','repeat_wed',
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
    synchronized_alarms = serializers.JSONField()
    current_time = serializers.JSONField()

    class Meta:
        model = AlarmClient
        fields = ['url', 'name', 'slug', 'alarm_delay', 'synchronized_alarms', 'current_time']

class AlarmClientViewSet(viewsets.ModelViewSet):

    filter_backends = (DjangoFilterBackend, filters.OrderingFilter)
    filter_fields = []
    
    queryset = AlarmClient.objects.all()
    serializer_class = AlarmClientSerializer
    permission_classes = (permissions.IsAuthenticated, )


    @detail_route(methods=['post'])
    def synchronize(self, request, pk=None):
        alarm = self.get_object()
        alarm.synchronize( request.GET.get('alarms') )
        serializer = self.get_serializer(alarm, many=False)
        return Response(serializer.data)        

