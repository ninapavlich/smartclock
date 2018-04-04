from django.contrib import admin
from django.conf import settings
from django.conf.urls.static import static
from django.urls import path, re_path, include
from django.conf.urls import url

from rest_framework import routers
from rest_framework.documentation import include_docs_urls

from apps.iot.api import AlarmViewSet
from .views import GUITemplateView

router = routers.DefaultRouter()
router.register(r'alarms', AlarmViewSet)


urlpatterns = [
    re_path('admin/', admin.site.urls),
    # re_path(r'^api/docs/', include_docs_urls(title='IOT API docs')),
    re_path(r'^api/', include(router.urls)),
    re_path(r'^api-auth/', include('rest_framework.urls',
                                   namespace='rest_framework')),

    url(r'^$', GUITemplateView.as_view(), name="gui"),
]

if settings.DEBUG:

    urlpatterns += static(settings.MEDIA_URL,
                              document_root=settings.MEDIA_ROOT)
    import debug_toolbar
    urlpatterns = [
        re_path(r'^__debug__/', include(debug_toolbar.urls)),
    ] + urlpatterns