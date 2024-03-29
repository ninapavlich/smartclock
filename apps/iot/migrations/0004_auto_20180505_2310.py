# Generated by Django 2.0.4 on 2018-05-06 06:10

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('iot', '0003_auto_20180422_2049'),
    ]

    operations = [
        migrations.CreateModel(
            name='AlarmClient',
            fields=[
                ('name', models.CharField(max_length=255)),
                ('slug', models.SlugField(max_length=255, primary_key=True, serialize=False, unique=True)),
                ('alarm_delay', models.IntegerField(default=0, help_text='Length of time to delay before triggering this alarm client')),
                ('last_synchronized', models.DateTimeField(blank=True, null=True)),
            ],
        ),
        migrations.RemoveField(
            model_name='alarm',
            name='last_synchronized',
        ),
        migrations.AddField(
            model_name='alarm',
            name='last_snoozed_time',
            field=models.DateTimeField(blank=True, null=True),
        ),
        migrations.AddField(
            model_name='alarm',
            name='last_stopped_time',
            field=models.DateTimeField(blank=True, null=True),
        ),
        migrations.AddField(
            model_name='alarm',
            name='next_alarm_time',
            field=models.DateTimeField(blank=True, null=True),
        ),
    ]
