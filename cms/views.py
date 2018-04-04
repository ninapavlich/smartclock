from django.conf import settings
from django.contrib.auth.mixins import UserPassesTestMixin
from django.contrib.auth.views import redirect_to_login
from django.core.exceptions import PermissionDenied
from django.views.generic import TemplateView

from rest_framework.authtoken.models import Token

class GUITemplateView(UserPassesTestMixin, TemplateView):

    
    template_name = 'gui.html'

    def handle_no_permission(self):
        if not self.request.user.is_authenticated:
            return redirect_to_login(self.request.get_full_path(), self.get_login_url(), self.get_redirect_field_name())
        else:
            raise PermissionDenied(self.get_permission_denied_message())

    def test_func(self):
        if not self.request.user.is_authenticated:
            return False

        return self.request.user.is_superuser or self.request.user.is_staff

    def get_context_data(self, *args, **kwargs):
        context = super(GUITemplateView, self).get_context_data(*args, **kwargs)

        full_path = ('http', ('', 's')[
                     self.request.is_secure()], '://', self.request.META['HTTP_HOST'])
        api_root = ''.join(full_path)

        context['API_ROOT'] = api_root
        try:
            context['API_KEY'] = Token.objects.get(user=self.request.user)
        except:
            pass

        return context