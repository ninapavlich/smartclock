# smartclock
IOT Smart Clock CMS for Django

# Requirements 
* Python 3
* PIP
* Virtualenv
* PostgresSQL


# Quickstart
To bootstrap Django:

    > git clone git@github.com:ninapavlich/smartclock.git
    > cd smartclock
    > virtualenv venv -p python3
    > source venv/bin/activate
    > pip install -r requirements.txt
    > python manage.py migrate
    > python manage.py createsuperuser



# Manual Heroku Configuration:
	
	> heroku create
	> heroku addons:create heroku-postgresql:hobby-dev
	> heroku config:set AWS_ACCESS_KEY_ID='REPLACEME' AWS_SECRET_ACCESS_KEY='REPLACEME+m/Knu1Hlw8SBp' AWS_STORAGE_BUCKET_NAME='REPLACEME' SECRET_KEY='REPLACEME'
	> git push heroku master
