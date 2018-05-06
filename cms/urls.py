from django.contrib import admin
from django.conf import settings
from django.conf.urls.static import static
from django.urls import path, re_path, include
from django.conf.urls import url
from django.views.generic import TemplateView

from rest_framework import routers
from rest_framework.documentation import include_docs_urls

from apps.iot.api import AlarmViewSet, AlarmClientViewSet
from .views import GUITemplateView

router = routers.DefaultRouter()
router.register(r'alarms', AlarmViewSet)
router.register(r'alarmclients', AlarmClientViewSet)

urlpatterns = [
    re_path('admin/', admin.site.urls),
    # re_path(r'^api/docs/', include_docs_urls(title='IOT API docs')),
    re_path(r'^api/', include(router.urls)),
    re_path(r'^api-auth/', include('rest_framework.urls',
                                   namespace='rest_framework')),

    url(r'^$', GUITemplateView.as_view(), name="gui"),
]

handler400 = 'cms.views.view_400'
handler403 = 'cms.views.view_403'
handler404 = 'cms.views.view_404'
handler500 = 'cms.views.view_500'




if settings.DEBUG:

    urlpatterns += static(settings.MEDIA_URL,
                              document_root=settings.MEDIA_ROOT)

    urlpatterns += [
        url(r'^error_400/', TemplateView.as_view(template_name="400.html"), name="error_400_preview" ),
        url(r'^error_403/', TemplateView.as_view(template_name="403.html"), name="error_403_preview" ),
        url(r'^error_404/', TemplateView.as_view(template_name="404.html"), name="error_404_preview" ),
        url(r'^error_500/', TemplateView.as_view(template_name="500.html"), name="error_500_preview" ),
        
    ]

    import debug_toolbar
    urlpatterns = [
        re_path(r'^__debug__/', include(debug_toolbar.urls)),
    ] + urlpatterns

