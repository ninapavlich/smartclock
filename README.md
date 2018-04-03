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

## One-Click Deploy Button

### Pre-reqs for deploying this project to Heroku:
 * Create a free [Heroku account](https://signup.heroku.com/) 
 * Create an [Amazon AWS account](https://portal.aws.amazon.com/billing/signup/)
 * Install the [Heroku CLI](https://devcenter.heroku.com/articles/heroku-cli)

[![Deploy](https://www.herokucdn.com/deploy/button.svg)](https://heroku.com/deploy?template=https://github.com/ninapavlich/smartclock/blob/master)

After your application has deployed, run the following command through the Heroku CLI and follow the prompts to create an initial user:

	> heroku run python manage.py createsuperuser


# Manual Heroku Configuration:
	
	> heroku create
	> heroku addons:create heroku-postgresql:hobby-dev
	> heroku config:set ENVIRONMENT='heroku' AWS_ACCESS_KEY_ID='REPLACEME' AWS_SECRET_ACCESS_KEY='REPLACEME+m/Knu1Hlw8SBp' AWS_STORAGE_BUCKET_NAME='REPLACEME' SECRET_KEY='REPLACEME'
	> git push heroku master
