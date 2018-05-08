import { Component, OnInit, ViewChildren, QueryList } from '@angular/core';

import { AlarmsService } from '../shared/alarm.service';
import { AlarmComponent } from '../alarm/alarm.component';

@Component({
  selector: 'my-home',
  templateUrl: './home.component.html',
  styleUrls: ['./home.component.scss']
})
export class HomeComponent implements OnInit {

  @ViewChildren('alarms') alarms:QueryList<AlarmComponent>;

  constructor(private alarmsService: AlarmsService) {
    // Do stuff
  }

  ngOnInit() {
    
  }

  addAlarm(){
    this.alarmsService.createAlarm({
        "name": "",
        "sound": null,
        "sound_md5": "",
        "time": "07:00:00",
        "active": true,
        "allow_snooze": true,
        "last_synchronized": null,
        "repeat_mon": true,
        "repeat_tue": true,
        "repeat_wed": true,
        "repeat_thu": true,
        "repeat_fri": true,
        "repeat_sat": true,
        "repeat_sun": true
    });
  }

  onSoundpickerPlaybackChange(event:any){
    if(event.playing == false){ return; }

    let alarm_list = this.alarms.toArray();
    //Pause any other playing alarms
    for( let i=0; i< alarm_list.length; i++ ){
      let alarm = alarm_list[i];
      if(alarm && alarm.pk && (String(alarm.pk)!=String(event.pk))){
        alarm.pause();  
      }
    } 
  }

}
