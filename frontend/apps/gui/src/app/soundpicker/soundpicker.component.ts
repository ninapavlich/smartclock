import { Component, OnInit, Input, ViewChild } from '@angular/core';

@Component({
  selector: 'soundpicker',
  templateUrl: './soundpicker.component.html',
  styleUrls: ['./soundpicker.component.scss']
})
export class SoundPickerComponent implements OnInit {

  @Input('id') id: string;
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
    return (this.src != "");
  }
  public get has_picked_audio():boolean{
    return (this.audioFileName != "");
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
    if(this.src != ""){
      let pieces = this.src.split("/");
      this.audioFileName = pieces[pieces.length -1];
      
    }
    
    console.log('Hello sound picker for '+this.audioFileName);
  }


  clearSound(){
    this.src="";
    this.audioFileName="";
    this.picked_audio_ready = false;
    this.picked_audio_loading = false;
    this.remote_audio_ready = false;
    this.remote_audio_loading = false;
  }
  play(){
    this.play_requested = true;

    this.audioplayer.nativeElement.play();
  
  }

  pause(){
    this.play_requested = false;

    this.audioplayer.nativeElement.pause();
  }


  onAudioPicked(){
    console.log("AUDIO PICKED!")
    this.picked_audio_ready = false;
    this.picked_audio_loading = false;
    this.loadPickedAudio();

  }
  loadPickedAudio() {
    this.picked_audio_loading = true;
    console.log("audioinput? "+this.audioinput)
    console.log("audioplayer? "+this.audioplayer.nativeElement)
    window['soundpicker'] = this;
    
    this.audioFile = this.audioinput.nativeElement.files[0];
    this.audioFileName = this.audioFile.name;
    this.audioplayer.nativeElement.src = URL.createObjectURL(this.audioFile);

  }
  onAudioLoaded(){
    console.log("onAudioLoaded")
  }
  onAudioEnded(){
    console.log("onAudioEnded")
    this.play_requested = false;

  }
  // onAudioFileLoaded(){
  //   console.log(("Filename: '" + this.audioFile.name + "'"), ( "(" + ((Math.floor(this.audioFile.size/1024/1024*100))/100) + " MB)" ));
  //   console.log(this.audioFileReader.result);
  //   // self.playAudioFile(fileReader.result);
  //   this.picked_audio_loading = false;
  //   this.picked_audio_ready = true;
  // }
  // playAudioFile(file) {
  //   var context = new AudioContext();
  //   context.decodeAudioData(file, function(buffer) {
  //     var source = context.createBufferSource();
  //       source.buffer = buffer;
  //       source.loop = false;
  //       source.connect(context.destination);
  //       source.start(0); 
  //   });
  // }

  loadRemoteAudio(){

  }
}
