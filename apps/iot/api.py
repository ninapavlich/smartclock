from django.conf import settings
from django.urls import reverse

from django_filters.rest_framework import DjangoFilterBackend

from rest_framework import serializers, viewsets, permissions, filters

from .models import Alarm

class AlarmSerializer(serializers.ModelSerializer):

    class Meta:
        model = Alarm
        fields = '__all__'


class AlarmViewSet(viewsets.ModelViewSet):
    
    queryset = Alarm.objects.all()
    serializer_class = AlarmSerializer
    permission_classes = (permissions.IsAuthenticated, )
