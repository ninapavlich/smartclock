import json
import logging

from django.conf import settings
from django.db.models.signals import post_save
from django.dispatch import receiver

import paho.mqtt.publish as publish

logger = logging.getLogger('django')

def send_model_message(instance, serializer_class, action):




    if not settings.MQTT_USERNAME:
        logger.warn("MQTT credentials not provided. Server cannot send MQTT messages.")
        return

    message_topic = (u'%s/%s/%s/' % (instance._meta.app_label, instance._meta.object_name, action)).lower()
    serialized = serializer_class(instance, context={'view': None, 'request': None, 'format': None})

    print(message_topic)
    # print(serialized.data)
    # payload = json.dumps(serialized.data)
    payload = 'hi'

    try:
        publish.single(
            message_topic,
            payload=payload,
            hostname=settings.MQTT_HOST,
            port=settings.MQTT_PORT,
            client_id=settings.MQTT_CLIENT_NAME,
            keepalive=15,
            auth={'username': settings.MQTT_USERNAME,
                  'password': settings.MQTT_PASSWORD}
        )
    except Exception as ex:

        template = u"Error sendinging MQTT message: {0}: {1!r}"
        message = template.format(
            type(ex).__name__,
            ex.args)
        logger.error(message)