# <p align=center> A Talking Clock in Chinese for the ESP32 S3 Box
<br/>


While going through the examples available on github, I wanted to see if I could copy and paste my way through a talking clock (in Chinese). &nbsp; This is my first time venturing into these niches.


---
## The clock has the following features:

   * *offline.*
   * *listen to commands to tell time and date,* 
   * *temperature (required a DHT22 hardware, G38 is data pin),*
   * *play mp3 from sc card (required MMC card hardware: Digilent Pmod MicroSD was suggested by Espressif people).&nbsp;  mp3 files can be stored in different folders.  &nbsp;Can go to next or back to previous song. &nbsp; Music volume can be adjusted.*
   * *all activities are carrying out via voice commands.*

----


## Summary:

`wifi and file / http server` are from [esp-idf](https://github.com/espressif/esp-idf).

`ispeech.c, ispeak.c`: &nbsp; The sr and tts - speech recognition and text to speech modules came from [esp-skainet](https://github.com/espressif/esp-skainet).

`play_mp3_control_example.c`: &nbsp; The MMC sd card and mp3 player came from [esp-adf](https://github.com/espressif/esp-adf).

`DHT22.c`:  &nbsp; DHT22 - temperature module came from [DHT22](https://github.com/gosouth/DHT22).

`main.c, clock_ui.c, setdatetime.c`:  &nbsp; Display / clock module came from [esp-box](https://github.com/espressif/esp-box).

Others - either were from stackoverflow.com or somewhere out there by googling.


While a great effort has been made to avoid modifying any original components directly and to do it at the frontend as much as possible, some changes have to be done at the backend.

If you don't want to compile yourself, the `misc` folder has all bin files readily to be flashed.
<br/><br/>

----
## Commands:

* ask for time: &nbsp; `几点了`, &nbsp; `时间呢`
* ask for date: &nbsp; `是什么今号`, &nbsp; `什么今号`
* ask for temperature: &nbsp; `是什么温度`，&nbsp; `温度呢`
* start music: &nbsp; `开始音乐`, &nbsp; `唱歌吧`
* stop music: &nbsp; `停止音乐`, &nbsp; `关掉音乐`
* previous song: &nbsp; `从前的`, &nbsp; `以前来的`
* next song: &nbsp; `未来的`, &nbsp; `后来的`
* increase volume: &nbsp; `更大音乐`, &nbsp; `大声音乐`
* decrease volume: &nbsp; `降低声音`, &nbsp; `小声音乐`
* previous mp3 folder: &nbsp; `上专辑`
* next mp3 folder: &nbsp; `下目录`

No comment on the Chinese command phrases! &nbsp; These were chosen to adapt to my Chinese accent. &nbsp;  You can modify it to adapt to your voice in the `sdkconfig` file. &nbsp; It is in pinyin format.
<br/><br/>

----

## Hints:
* boot button (upper left) to goto setting clock menu, and to select mp3 folder.
* after you have selected a new mp3 folder, either stop then start music again if it is currently playing. &nbsp; Or just let it finish the current song.
* the voice volume is independent from music volume.  
* See [SD_CARD.md](SD_CARD.md) for setting mp3 files structure on the sd card.

----


## Notes:

* Updated: This project was compiled against `idf v5.0`. 
* You will need to manually flash `esp_tts_voice_data_xiaole.dat` at address `0x410000`. &nbsp; This for the Chinese tts. &nbsp;  The file is located in `misc\bin_files`
* No attempt was made to handle in the absence of MMC sd card and DHT22 hardware.  &nbsp; So if you do not have these two pieces of hardware and you want to use the clock only, then you will need to dig in and take out those related code.
* This is provided "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied. &nbsp;
   All code modules are from the mentioned entities. &nbsp; I just glued it together!

-----
<br/>
<p> 
<img src="https://i.imgur.com/FHnwwc6.jpg" width="32%">
<img src="https://i.imgur.com/UBk2ir1.jpg" width="32%">
<img src="https://i.imgur.com/5wdMTSw.jpg" width="32%">
</p>

https://user-images.githubusercontent.com/116512015/211103816-064f97ce-a421-4015-b901-3b5c4b9a339b.mov



https://user-images.githubusercontent.com/116512015/211103827-36ddb841-57c4-4fd6-ba64-a413dd7602c3.mov



https://user-images.githubusercontent.com/116512015/211103832-7ed268f1-9538-4d71-a8f6-3471dcedecbf.mov



https://user-images.githubusercontent.com/116512015/211103835-3c6e3bc8-20b5-4059-a980-0bcfd98e8c21.mov



https://user-images.githubusercontent.com/116512015/211103838-e33d799b-aee2-4db2-96f2-1755d91e3cf7.mov



https://user-images.githubusercontent.com/116512015/211103844-e9f58e44-7333-4885-8262-3efc464645e4.mov

<br/>
<br/>

## Full Demo on YouTube
[![Full Demo on YouTube](https://i.imgur.com/AlJaVJG.png)](https://www.youtube.com/watch?v=Xuv5Fm0Sf6c "Full Demo on YouTube")

## <p align=center>------------------- A !Mundane World -------------------





