import { Component, OnInit, Input, Output, ViewChild, EventEmitter } from '@angular/core';

@Component({
  selector: 'soundpicker',
  templateUrl: './soundpicker.component.html',
  styleUrls: ['./soundpicker.component.scss']
})
export class SoundPickerComponent implements OnInit {
  
  @Output() sourcechange = new EventEmitter<File>();
  @Output() playbackchange = new EventEmitter<boolean>();

  @Input('pk') pk: string;
  @Input('src') src: string;

  @ViewChild('audioplayer') audioplayer; 
  @ViewChild('audioinput') audioinput; 
  protected audioFileName:string = null;
  protected audioFile:File = null;
  // protected audioFileReader:FileReader = null;

  protected play_requested:boolean = false;
  protected picked_audio_ready:boolean = false;
  protected remote_audio_ready:boolean = false;
  protected picked_audio_loading:boolean = false;
  protected remote_audio_loading:boolean = false;
  
  public get has_sound():boolean{
    return this.has_remote_audio || this.has_picked_audio;
  }
  public get has_remote_audio():boolean{
    return (this.src != "" && this.src != null);
  }
  public get has_picked_audio():boolean{
    return (this.audioFileName != "" && this.audioFileName != null);
  }
  public get is_audio_loading():boolean{
    if(this.has_remote_audio === true){
      return this.remote_audio_loading;
    }else if(this.has_picked_audio === true){
      return this.picked_audio_loading;
    }
    return false;
  }
  public get is_ready_to_play():boolean{
    if(this.has_remote_audio === true){
      return this.remote_audio_ready;
    }else if(this.has_picked_audio === true){
      return this.picked_audio_ready;
    }
    return false;
  }

  constructor() {
   
  }

  ngOnInit() {
    if(this.has_remote_audio){
      let pieces = this.src.split("/");
      this.audioFileName = pieces[pieces.length -1];
    }
  }


  clearSound(){
    this.pause();
    this.src="";
    this.audioFile = null;
    this.audioFileName="";
    this.picked_audio_ready = false;
    this.picked_audio_loading = false;
    this.remote_audio_ready = false;
    this.remote_audio_loading = false;
    this.sourcechange.emit(null);
  }
  play(){
    this.audioplayer.nativeElement.currentTime = 0;
    this.audioplayer.nativeElement.play();
    if(this.play_requested == true){
      return;
    }
    this.play_requested = true;
    this.playbackchange.emit(true);
  }  

  pause(){
    this.audioplayer.nativeElement.pause();
    if(this.play_requested == false){
      return;
    }
    this.play_requested = false;
    this.playbackchange.emit(false);
  }


  onAudioPicked(){
    this.pause();
    this.picked_audio_ready = false;
    this.picked_audio_loading = false;
    this.loadPickedAudio();
  }
  loadPickedAudio() {

    this.picked_audio_loading = true;
    this.audioFile = this.audioinput.nativeElement.files[0];
    this.audioFileName = this.audioFile.name;
    this.audioplayer.nativeElement.src = URL.createObjectURL(this.audioFile);
    this.sourcechange.emit(this.audioFile);
  }
  onAudioLoaded(){
    //pass
  }
  onAudioEnded(){
    this.play_requested = false;
    this.playbackchange.emit(false);
  }
  
  loadRemoteAudio(){

  }
}
