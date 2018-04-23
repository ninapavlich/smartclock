import { Component, OnInit, Input, Output, ViewChild,  EventEmitter } from '@angular/core';
import { AlarmsService, AlarmService } from '../shared/alarm.service';

@Component({
  selector: 'alarm',
  templateUrl: './alarm.component.html',
  styleUrls: ['./alarm.component.scss']
})
export class AlarmComponent implements OnInit {

  @Output() playbackchange = new EventEmitter<any>();

  @Input('alarm') alarm: AlarmService;
   
  @ViewChild('name_input') name_input; 
  @ViewChild('time_input') time_input; 
  @ViewChild('active_input') active_input;
  @ViewChild('soundpicker_input') soundpicker_input; 
  @ViewChild('repeat_mon_input') repeat_mon_input; 
  @ViewChild('repeat_tue_input') repeat_tue_input; 
  @ViewChild('repeat_wed_input') repeat_wed_input; 
  @ViewChild('repeat_thu_input') repeat_thu_input; 
  @ViewChild('repeat_fri_input') repeat_fri_input; 
  @ViewChild('repeat_sat_input') repeat_sat_input; 
  @ViewChild('repeat_sun_input') repeat_sun_input; 
  @ViewChild('allow_snooze_input') allow_snooze_input; 

  subscription: any;
  pk:string = "";
  name:string = "";
  has_name:boolean = false;
  active:boolean = true;
  time:Date = null;
  sound_url:string = "";
  sound:File = null;
  repeat_mon:boolean = true;
  repeat_tue:boolean = true;
  repeat_wed:boolean = true;
  repeat_thu:boolean = false;
  repeat_fri:boolean = true;
  repeat_sat:boolean = true;
  repeat_sun:boolean = true;
  allow_snooze:boolean = true;
  last_synchronized:string = "";
  loading:boolean = false;



  constructor(private alarmsService: AlarmsService) {
    // Do stuff
  }

  ngOnInit() {
    
    this.pk = this.alarm.pk;

    this.name = this.alarm.name;
    this.has_name = this.name != "";
    // this.time = this.alarm.time;
    this.sound_url = this.alarm.sound_url;
    this.active = this.alarm.active;
    this.allow_snooze = this.alarm.allow_snooze;
    this.last_synchronized = this.alarm.last_synchronized;
    this.repeat_mon = this.alarm.repeat_mon;
    this.repeat_tue = this.alarm.repeat_tue;
    this.repeat_wed = this.alarm.repeat_wed;
    this.repeat_thu = this.alarm.repeat_thu;
    this.repeat_fri = this.alarm.repeat_fri;
    this.repeat_sat = this.alarm.repeat_sat;
    this.repeat_sun = this.alarm.repeat_sun;

    this.subscription = this.alarm.loading.subscribe(loading => this.loading = loading);
  }
  ngOnDestroy() {
    this.subscription.unsubscribe();
  }

  updateModel(debug_caller:string){
    this.alarm.name = this.name;
    // this.alarm.time = this.time;
    if(this.sound){
      this.alarm.sound = this.sound;  
      console.log("ADDING SOUND "+this.sound)
    }
    this.alarm.active = this.active;
    this.alarm.allow_snooze = this.allow_snooze;
    this.alarm.last_synchronized = this.last_synchronized;
    this.alarm.repeat_mon = this.repeat_mon;
    this.alarm.repeat_tue = this.repeat_tue;
    this.alarm.repeat_wed = this.repeat_wed;
    this.alarm.repeat_thu = this.repeat_thu;
    this.alarm.repeat_fri = this.repeat_fri;
    this.alarm.repeat_sat = this.repeat_sat;
    this.alarm.repeat_sun = this.repeat_sun;
    this.alarm.save(debug_caller);
  }
  updateName(value: string){
    if(this.name == value){ return; }
    this.name = value;
    this.has_name = this.name != "";
    this.updateModel("Alarm.updateName()");
  }
  updateTime(value:any){
    console.log("TODO: parse time from "+value)
    window['test_time'] = value;
    // this.time = value;
  }
  deleteAlarm(){
    this.alarmsService.deleteAlarm(this.pk)
  }
  pause(){
    this.soundpicker_input.pause();
  }
  onSoundpickerPlaybackChange(playing:boolean){
    this.playbackchange.emit({"playing":playing, "pk":this.pk})
  }
  onSoundpickerSourceChange(src:File){
    if(this.sound == src){ return; }
    this.sound = src;
    this.updateModel("Alarm.onSoundpickerSourceChange()");
  }

}
