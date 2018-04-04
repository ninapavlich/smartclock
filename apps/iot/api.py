from django.conf import settings
from django.urls import reverse

from django_filters.rest_framework import DjangoFilterBackend

from rest_framework import serializers, viewsets, permissions, filters
from rest_framework import viewsets, permissions, filters
from rest_framework.decorators import detail_route
from rest_framework.response import Response

from django_filters.rest_framework import DjangoFilterBackend

from .models import Alarm

class AlarmSerializer(serializers.ModelSerializer):

    class Meta:
        model = Alarm
        fields = ['url', 'pk', 'name', 'sound', 'sound_md5', 'time', 'active', 'allow_snooze', 'last_synchronized', 'repeat_mon','repeat_tue','repeat_wed','repeat_thu','repeat_fri','repeat_sat','repeat_sun']

class AlarmViewSet(viewsets.ModelViewSet):

    filter_backends = (DjangoFilterBackend, filters.OrderingFilter)
    filter_fields = ['active']
    
    queryset = Alarm.objects.all()
    serializer_class = AlarmSerializer
    permission_classes = (permissions.IsAuthenticated, )


    @detail_route(methods=['post'])
    def synchronize(self, request, pk=None):
        alarm = self.get_object()
        alarm.synchronize()
        serializer = self.get_serializer(alarm, many=False)
        return Response(serializer.data)