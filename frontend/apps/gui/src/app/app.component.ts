import { Component, Inject, ElementRef } from '@angular/core';
import { AlarmsService } from './shared/alarm.service';

import '../style/app.scss';

@Component({
  selector: 'my-app', // <my-app></my-app>
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.scss'],
})
export class AppComponent {
  constructor(@Inject(ElementRef) private elementRef: ElementRef, alarmsService: AlarmsService) {
    let nativeElement = elementRef.nativeElement;
    alarmsService.init(nativeElement.getAttribute('data-api-root'), nativeElement.getAttribute('data-auth-header'));
  }
}
