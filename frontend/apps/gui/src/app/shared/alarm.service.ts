import { Injectable, Output, EventEmitter } from '@angular/core';
import { Http, Headers } from '@angular/http';

@Injectable()
export class AlarmsService {

  static api_root:string = 'http://127.0.0.1:8000';
  static api_auth_header:string = 'Token c8e094d7b29e3798e4639610942bee1949275bfb';

  @Output() loading = new EventEmitter<boolean>();

  alarms = [];

  polling_interval = null;
  polling_interval_time = 1000 * 60 * 10; //Every 10 minutes

  constructor (
    private http: Http
  ) {
    this.loadAlarms();
    this.polling_interval = setInterval(this.loadAlarms.bind(this), this.polling_interval_time);
  }

  static getAPIURL():string{
    return AlarmsService.api_root+"/api/alarms/";
  }
  
  static getAuthHeaders():Headers{
    let headers = new Headers();
    headers.set('Authorization', AlarmsService.api_auth_header);
    headers.set('content-type', "application/json");
    return headers;
  }
  public loadAlarms() {
    this.loading.emit(true);

    return this.http.get(AlarmsService.getAPIURL(), {headers: AlarmsService.getAuthHeaders()})
      .map(res => res.json()).subscribe(
        data => { 
          this.alarms = [];
          this.fromJSON(data); 
          this.loading.emit(false);
         },
        err => { 
          console.log(err); 
          this.loading.emit(false); 
        }
      );
  }
  public fromJSON(alarm_list_json:any){
    for(let i=0; i<alarm_list_json.length; i++){
      let alarm_json = alarm_list_json[i];
      let alarm = this.getAlarmByPK(alarm_json.pk);
      if(alarm === null){
        this.createAlarm(alarm_json);
      }
    }
  }
  public getAlarmByPK(pk:string){
    for(let i=0; i<this.alarms.length; i++){
      if(pk == this.alarms[i].pk){
        return this.alarms[i];
      }
    }
    return null;
  }
  public createAlarm(alarm_json:any){
    let alarm = new AlarmService(this.http)
    alarm.fromJSON(alarm_json);
    this.alarms.push(alarm);
    return alarm
  }
  deleteAlarm(pk:string){
    let alarm = this.getAlarmByPK(pk);
    if( alarm !== null){
      let index = this.alarms.indexOf(alarm);
      if (index > -1) {
        this.alarms.splice(index, 1);
      }
      alarm.delete();  
    }
    

  }

}


export class AlarmService {
  
  @Output() loading = new EventEmitter<boolean>();

  url:string;
  pk:string;
  name:string;
  sound:File;
  sound_url:string;
  sound_md5:string;
  time:string;
  active:boolean;
  allow_snooze:boolean;
  last_synchronized:string;
  repeat_mon:boolean;
  repeat_tue:boolean;
  repeat_wed:boolean;
  repeat_thu:boolean;
  repeat_fri:boolean;
  repeat_sat:boolean;
  repeat_sun:boolean;

  currently_loading:boolean = false;
  reload_request = true;



  constructor (
    private http: Http
  ) {}

  public fromJSON(json:any){
    this.url = json.url? json.url : null;
    this.pk = json.pk? json.pk : gupk();
    this.name = json.name;
    this.sound_url = json.sound;
    this.sound_md5 = json.sound_md5;
    this.time = json.time;
    this.active = json.active;
    this.allow_snooze = json.allow_snooze;
    this.last_synchronized = json.last_synchronized;
    this.repeat_mon = json.repeat_mon;
    this.repeat_tue = json.repeat_tue;
    this.repeat_wed = json.repeat_wed;
    this.repeat_thu = json.repeat_thu;
    this.repeat_fri = json.repeat_fri;
    this.repeat_sat = json.repeat_sat;
    this.repeat_sun = json.repeat_sun;
  }
  private toFormData():FormData{
    let formData:FormData = new FormData();
    formData.append('active', String(this.active));
    formData.append('allow_snooze', String(this.allow_snooze));
    formData.append('repeat_mon', String(this.repeat_mon));
    formData.append('repeat_tue', String(this.repeat_tue));
    formData.append('repeat_wed', String(this.repeat_wed));
    formData.append('repeat_thu', String(this.repeat_thu));
    formData.append('repeat_fri', String(this.repeat_fri));
    formData.append('repeat_sat', String(this.repeat_sat));
    formData.append('repeat_sun', String(this.repeat_sun));


    if(this.name){ formData.append('name', this.name); }
    if(this.time){ formData.append('time', this.time);}
    // if(this.sound){ formData.append('sound', this.sound); }
    // if(this.sound){ formData.append('sound', this.sound, this.sound.name); }
  
    // formData.append("sound", "/Volumes/ARCHIVE/Example Images/Audio/Mariachi Guadalajara - Happy Birthday Original Mix.mp3");   

    window['test_formData'] = formData;

    return formData;

  }
  getSaveURL(){
    if(this.url){
      return this.url;
    }else{
      return AlarmsService.getAPIURL();
    }
  }
  getSaveMethod(){
    if(this.url){
      return "PUT";
    }else{
      return "POST";
    }
  }
  save(debug_caller:string) {
    console.log(this+" SAVE from "+debug_caller)
    this.loading.emit(true);
    let headers = AlarmsService.getAuthHeaders();
    // headers.set('Content-Type', "multipart/form-data");
    let data = this.toFormData()
    

    if(this.url){
      this.http;
      return this.http.put(this.url, data, {headers: headers})
        .map(res => res.json()).subscribe(
          data => { 
            this.fromJSON(data); 
            this.loading.emit(false);
           },
          err => { 
            console.log(err); 
            this.loading.emit(false); 
          }
        );
    }else{
      return this.http.post(AlarmsService.getAPIURL(), data, {headers: headers})
        .map(res => res.json()).subscribe(
          data => { 
            this.fromJSON(data); 
            this.loading.emit(false);
           },
          err => { 
            console.log(err); 
            this.loading.emit(false); 
          }
        );
    }
  }
  delete(){
    if(this.url){
      return this.http.delete(this.url, {headers: AlarmsService.getAuthHeaders()})
        .map(res => res.json()).subscribe(
          data => { 
            data;
            this.loading.emit(false);
           },
          err => { 
            console.log(err); 
            this.loading.emit(false); 
          }
        );
    }
  }
  toString() {
    return "Alarm "+this.pk;
  }
}

function gupk() {
  function s4() {
    return Math.floor((1 + Math.random()) * 0x10000)
      .toString(16)
      .substring(1);
  }
  return s4() + s4() + '-' + s4() + '-' + s4() + '-' + s4() + '-' + s4() + s4() + s4();
}