## Automated Reflected Timer

This tool parses mp4 videos and extracts 2 numbers from each frame of the video, 1 from the left side and 1 from the right side (Left(Timer from Camera)|Right(Timer from App)). This is how the tool was tested, in theory it can read two numbers if they are in different image segments according to the way tesseract segments an image.

It also has the option to process folders with .mp4 files (--dir flag)

It uses the Tesseract model(configurable with the CONFIDENCE_LEVEL env var, by default 0.75) and simple OpenCV commands to do so. 
Ensure the video has ok quality and the two windows (Timer from Camera and Timer from App) cover the whole screen and there are not any gaps between the windows(especially numbers or letters).

It currently rejects measurements with delay over 250ms as invalid which might not be true in realistic
network conditions.

Install the tesseract CLI tool:
```bash
sudo apt install tesseract-ocr
```

Don't forget to install the packages from requirements.txt and create a virtual environment.

```bash 
python3 -m venv .venv
```

```bash
.venv/bin/pip install -r requirements.txt
```

Test the app using the sample mp4 file (with the --dir flag you can input a folder with .mp4 files)
```bash 
.venv/bin/python3 opencv-num-recog.py --video sample.mp4 --fps 30 --debug
```

Help page:
```bash
.venv/bin/python3 opencv-num-recog.py --help
```

Also you need the ffmpeg executable to run this tool.

Install it(In Ubuntu) using `sudo apt install ffmpeg`.

