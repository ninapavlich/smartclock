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

When configuring the fields, be sure to take note of the App Name, as you will be using this in the next steops. 

After your application has deployed, run the following commands through the command line Heroku CLI and follow the prompts:

	> heroku run python manage.py createsuperuser --app=<replace-with-app-name>
	> heroku run python manage.py drf_create_token <replace-with-username> --app=<replace-with-app-name>
	> heroku ps:scale web=1

# Manual Heroku Configuration:
	
	> heroku create
	> heroku addons:create heroku-postgresql:hobby-dev
	> heroku config:set ENVIRONMENT='heroku' AWS_ACCESS_KEY_ID='REPLACEME' AWS_SECRET_ACCESS_KEY='REPLACEME' AWS_STORAGE_BUCKET_NAME='REPLACEME' SECRET_KEY='REPLACEME' APP_HOST_NAME='my-heroku-app-name.herokuapp.com'
	> git push heroku master


## Tools

### Postman Collection

To test API endpoints, a Postman API collection is included in the tools folder.

1. Download Postman
2. Import the JSON file by going to File -> Import
3. Once imported, right click on the collection labeled "Smart Alarm" and choose Edit. 
4. In the "Variables" tab, update the API_ROOT value with your Heroku URL (e.g. http://my-heroku-app-name.herokuapp.com) and the API_KEY value with your API key, which would be found at this path: /admin/authtoken/token/


#Front-End 
To bootstrap the front end GUI:

	> cd frontend/apps/gui
	> npm install
	> npm start

To build the front end code to be deployed:

	> npm run build