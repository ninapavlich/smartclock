import hashlib

from django.db import models
from django.db.models.fields.files import FileDescriptor, FieldFile, ImageFieldFile, ImageFileDescriptor


class MD5FieldFileMixin(object):
    _md5 = None

    def __init__(self, *args, **kwargs):
        file_field = args[1]
        self.md5_field = file_field.md5_field
        super(MD5FieldFileMixin, self).__init__(*args, **kwargs)
        
    @property
    def md5(self):
        self._require_file()
        return self._md5

    def calculate_md5(self, content):
        hash_md5 = hashlib.md5()
        for chunk in content.chunks():
            hash_md5.update(chunk)
        self._md5 = hash_md5.hexdigest()

    def save(self, name, content, save=True):
        self.calculate_md5(content)
        super(MD5FieldFileMixin, self).save(name, content, save)
        if self.md5_field:
            setattr(self.instance, self.md5_field, self.md5)


class MD5FieldFile(MD5FieldFileMixin, FieldFile):
    pass


class MD5ImageFieldFile(MD5FieldFileMixin, ImageFieldFile):
    pass

class MD5FileFieldMixin(object):

    def __init__(self, *args, **kwargs):
        self.md5_field = kwargs.pop("md5_field", None)
        super(MD5FileFieldMixin, self).__init__(*args, **kwargs)

    def save_form_data(self, instance, data):
        if data==False:
            #Clear MD5 if field has been cleared out
            setattr(instance, self.md5_field, None) 
        super(MD5FileFieldMixin, self).save_form_data(instance, data)

class MD5FileField(MD5FileFieldMixin, models.FileField):
    attr_class = MD5FieldFile
    descriptor_class = FileDescriptor
    description = "MD5FileField"


class MD5ImageField(MD5FileFieldMixin, models.ImageField):
    attr_class = MD5ImageFieldFile
    descriptor_class = ImageFileDescriptor
    description = "MD5ImageField"

