import { Component, OnInit, Input } from '@angular/core';

@Component({
  selector: 'alarm',
  templateUrl: './alarm.component.html',
  styleUrls: ['./alarm.component.scss']
})
export class AlarmComponent implements OnInit {

  @Input('id') id: string;

  constructor() {
    // Do stuff
  }

  ngOnInit() {
    console.log('Hello Alarm');
  }

}
