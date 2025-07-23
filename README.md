# video-streaming-benchmarking-toolkit

This is a 3-piece solution for measuring the performance of video cameras, more specifically focusing on webcams. It was used as part of a study regarding optimising low latency multimedia systems using G-Streamer as the multimedia framework.

A non-conventional approach was used, due to convenience and experimentation, to use computer vision in measuring the
end to end latency of video streaming from a webcam. More specifically the [tesseract-ocr](https://github.com/tesseract-ocr/tesseract) model was used.

Components:

- [mpromonet/libv4l2cpp](https://github.com/mpromonet/libv4l2cpp), a C++ wrapper of 
the Video for Linux 2 driver. It was modified to calculate an average and a standard delay of the webcam.
It can be used to give a baseline of how often a frame is received from the webcam(there is a slight deviation from the fps and this is important in low latency systems. as it means that frames might be dropped).

- **Timer**: A Qt-based timer that can count with a custom framerate, often useful in such applications.
<img width="957" height="1048" alt="timer-picture" src="https://github.com/user-attachments/assets/29c23aab-358c-4d82-a5fe-baf0aad0772a" />


- **Automated Reflected Timer**: A python script that uses computer vision in videos that contain the rendered timer,
and the actual current timer in order to determine the end to end delay of video streaming. The tool can also mass process a directory containing videos and saves the measurements to a csv file. Reflected timer basically is the method of pointing the camera to a clock, and then rendering that to the side and taking the difference of the two measurements as the e2e latency. A sample video is offered below. 


![automated-reflected-timer-video](https://github.com/user-attachments/assets/b4780602-fee9-41e0-bc56-c444a9f0edcd)
