# video-trans-withUDP

#### envs:
* opencv3
* gstreamer1.0 (compile with opencv3 using `cmake`)
#### compile:
```
g++ xxx.cpp -o xxx `pkg-config --cflags --libs opencv`
```
----
## 2020.01.18
> * remove debug print and write code comment in **send.cpp** and **receive.cpp**
#### TBD
> * add more functions

----
## 2019.12.26
> * use **send.cpp** to send video with UDP  in clinet A  
> * use **receive.cpp** to receive video with UDP in client B  
> * remember to replace the video src in **send.cpp** (rtsp、local file、gstreamer pipeline both OK)  
> * remember to replace the IP address in **send.cpp**  
#### TBD
> * remove debug print
> * add more comment in code  
  
----
