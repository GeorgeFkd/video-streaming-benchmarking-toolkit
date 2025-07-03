import cv2
import argparse
from typing import Union
import os
import subprocess
import sys
import numpy as np
import time
import csv
import pytesseract
import re
import logging
from tqdm import tqdm
import io
import matplotlib.pyplot as plt

logger = logging.getLogger(__name__)
LOGLEVEL=os.environ.get("LOGLEVEL","INFO").upper()
logging.basicConfig(encoding="utf-8",level=LOGLEVEL)
CONFIDENCE_LEVEL=float(os.environ.get("CONFIDENCE",0.75))
FFMPEG_OUTPUT_FOLDER = "outputs"

def get_files_of_folder(folder,filter_fn):
    return [f for f in os.listdir(folder) if filter_fn(f)]

def convert_video_to_jpegs(video_path:str,fps):
    # sample.mp4 -> sample
    new_folder_suffix = os.path.splitext(os.path.basename(video_path))[0]
    out_folder = os.getcwd() + "/" + FFMPEG_OUTPUT_FOLDER + "_" + new_folder_suffix
    if not(os.path.exists(out_folder)):
        os.mkdir(out_folder)
    else:
        # means ffmpeg has already been ran in that folder
        return out_folder

    logger.debug(f"Video: {video_path} Operation:jpeg-conversion")
    # ffmpeg -i ~/Videos/Screencasts/experiments-for-cv-2.mp4 -r 30 -q:v 1 Pictures/output%d.jpg
    command = ["ffmpeg","-i",video_path,"-r",str(fps),"-q:v",str(1),f"{out_folder}/img%d.jpg"]
    logger.debug(f"Running ffmpeg command: {str(command)}")
    subprocess.run(command)
    return out_folder
def process_images_in_folder(folder,limit,debug=False):
    images = get_files_of_folder(folder,lambda x: x.endswith(".jpg") or x.endswith(".jpeg"))
    logger.debug(f"Folder {folder} contains {len(images)} images")
    if len(images) == 0:
        logger.warning(f"Folder: {folder} Reason:Does not contain images")
        return {"delays":[],"frames_rejected":-1}
    if debug:
        images = images[1:7]
    delays = []
    frames_rejected = 0
    if limit:
        images = images[:limit]
    
    for img in tqdm(images,ascii=True,desc=f"Processing images in folder: {os.path.basename(folder)}"):
        print()
        left,right = process_image(folder + "/" + img)
        if left >= right: 
            frames_rejected = frames_rejected + 1
            continue
        delay = right - left
        # something probably went wrong ignore the measurement
        # anything more is intensely noticeable and will already be rejected by humans
        # It is required as one problematic measurement will ruin the result
        if delay > 250:
            logger.info(f"Rejected Buffer with delay: {delay}")
            frames_rejected = frames_rejected + 1
            continue
        logger.info(f"Image: {img} Difference: {delay}ms")
        delays.append(delay)
    return dict({'delays':delays,"frames_rejected":frames_rejected})

def find_text_on_img_with_ocr(img_array,threshold=150):
    # Preprocess (black digits on white background)
    # On the docs it says to not do that but i got an error by putting it away
    def process_roi(roi):
        gray = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)
        _, thresh = cv2.threshold(gray, threshold, 255, cv2.THRESH_BINARY)
        return thresh
    start = time.monotonic_ns()
    prepared_img = process_roi(img_array)

    tsv_result = pytesseract.image_to_data(prepared_img,config='--psm 6 digits')
    end = time.monotonic_ns()
    logger.info(f"Tesseract CV needed {(end-start)/1_000_000}ms for one image")
    logger.info(f"Tesseract result: {tsv_result}")
    tsv_file = io.StringIO(tsv_result)
    reader = csv.DictReader(tsv_file,delimiter='\t')
    # we filter out segments of the image where no text is found
    rows = list(filter(lambda r: float(r['conf']) != -1.0 and len(r['text']) != 0 ,reader))
    try:
        # the model might not return proper results
        confidence_left = rows[-2]['conf']
        predicted_text_left = rows[-2]['text']

        confidence_right = rows[-1]['conf']
        predicted_text_right = rows[-1]['text']
    except:
        logger.info("Could not find text in image")
        return (float(-1),"",float(-1),"")

    logger.info(f"Confidence Left: {confidence_left}, Predicted Text Left: {predicted_text_left},Confidence Right: {confidence_right}, Predicted Text Right: {predicted_text_right}")
    return (float(confidence_left),predicted_text_left,float(confidence_right),predicted_text_right)
def process_image(
        input_path: str,
) -> tuple[int,int]:
    """Process image split into left/right halves and return numbers as integers"""
    img = cv2.imread(input_path)
        
    # OCR on whole image(no need to split the image)
    confidence_left,text_left,confidence_right,text_right = find_text_on_img_with_ocr(img)
    
    if confidence_left < CONFIDENCE_LEVEL or confidence_right < CONFIDENCE_LEVEL:
        logger.debug("CV OCR Model has confidence under 0.75")
        return -1,-1
    
    if text_left == "" or text_right == "":
        logger.debug("CV OCR Model returned blank one of the measurements")
        return -1,-1
    if len(text_left) != len(text_right):
        logger.debug("CV OCR Model returned numbers of different length")
        return -1,-1

    return int(text_left), int(text_right)

def is_ffmpeg_available() -> bool:
    try:
        result = subprocess.call(["ffmpeg","--help",os.devnull]) == 0
        return result
    except:
        return False
def is_tesseract_available() -> bool:
    try:
        result = subprocess.call(["tesseract","--version",os.devnull]) == 0
        return result
    except:
        return False

def process_video(video_path,fps,limit,debug):
    jpegs_folder = convert_video_to_jpegs(video_path,fps)
    result = process_images_in_folder(jpegs_folder,limit,debug=debug)
    if result["delays"] == []:
        return;
    video_id = os.path.splitext(os.path.basename(video_path))[0]
    delays = np.array(result.get("delays",[]))
    save_path = f"{video_id}-measurements-numpy-" + str(time.time_ns()/1000)
    np.save(save_path,delays,allow_pickle=False)
    logger.info(f"Saved numpy array to {save_path}.npy")
    average_delay = np.average(delays)
    stdev_delay = np.std(delays,dtype=np.float128)
    if np.isnan(average_delay) or np.isnan(stdev_delay):
        logger.warning(f"Something went wrong, nan values in stdev or avg, skipping {video_id}")
        return;
    logger.info(f"Delay average {average_delay}")
    logger.info(f"Delay stdev {stdev_delay}")
    # testing should not be included in actual results
    if debug or limit:
        return delays;
    
    field_names = ["video_id","average_delay","stdev_delay","frames_rejected","frames_accepted","confidence_threshold"]
    with open('data.csv','a') as csvfile:
        writer = csv.DictWriter(csvfile,fieldnames=field_names,restval="ignore")
        record =  {'video_id':video_id,"average_delay":average_delay,"stdev_delay":stdev_delay,"frames_rejected":result.get("frames_rejected",-1),"frames_accepted":len(delays),"confidence_threshold":CONFIDENCE_LEVEL}
        logger.debug("Inserting into csv record:")
        logger.info(record)
        try:
            writer.writerow(record)
        except:
            logger.error("Something went wrong when trying to save the record to the csv")
    return delays
def get_videos_in_folder(folder):
    files = get_files_of_folder(folder,lambda f: os.path.splitext(os.path.basename(f))[1] == ".mp4")
    return files

def process_videos(folder,fps,limit,debug):
    videos = get_videos_in_folder(folder)
    print(f"{len(videos)} videos will be processed in {folder}")
    # [(np.arr,"label")]
    delay_data_for_box_plots = []
    for video_path in videos:
        logger.info(f"Processing video: {video_path}")        
        np_delays = process_video(folder + "/" + video_path,fps,limit,debug)
        if np_delays is not None and len(np_delays) > 0:
            delay_data_for_box_plots.append((np_delays,os.path.basename(video_path)))
    logger.info(f"Will now create plots for {len(delay_data_for_box_plots)} runs")
    create_box_plot(delay_data_for_box_plots)
    return videos

def create_box_plot(data_w_labels_and_colors):
    labels = list(map(lambda x: x[1],data_w_labels_and_colors))
    data = list(map(lambda x: x[0],data_w_labels_and_colors))
    logger.info(f"Labels: {len(labels)}, Data: {len(data)}")
    fig,ax = plt.subplots(figsize=(10,6))
    ax.set_title("GStreamer Latency Measurements for Video Transmission")
    ax.set_ylabel('E2E Delay(ms)')
    bplot = ax.boxplot(data,tick_labels=labels,sym="",widths=0.3)
    
    ax.set_xticks(range(1, len(labels) + 1))  # Set correct positions
    ax.set_xticklabels(labels, rotation=45, ha='center')  # Rotate and align

    plt.tight_layout()


    filename = f"gstreamer-measurements-{time.monotonic()}.pdf"
    plt.savefig(f"gstreamer-measurements-{time.monotonic()}.pdf",format="pdf")
    logger.info(f"Box plot saved as {filename}")


def check_valid_environment():
    if not is_ffmpeg_available():
        logger.error("ffmpeg is not available in this system, install it using your package manager, refer to the README")
        sys.exit(1)
    if not is_tesseract_available():
        logger.error("tesseract is not available in this system,install it using your package manager, refer to the README")
        sys.exit(2)
    return 


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--debug",default=False,action="store_true",help="Show extra debug output and only process a few files")
    parser.add_argument("--video", type=str,help="path to mp4 file to chop into jpegs and analyse,")
    parser.add_argument("--fps",type=int,default=30,help="Framerate to chop video into (set it to the framerate of the camera and the timer you used in the video)")
    parser.add_argument("--dir",type=str,help="path to folder with mp4 files to analyse")
    parser.add_argument("--limit",type=int,help="how many jpegs should be processed per video to calculate the results")
    args = parser.parse_args()
    check_valid_environment()
    if (not args.video) and (not args.dir):
        logger.error("you should either specify --video or --dir")
        sys.exit(1)
    if args.video and args.dir:
        logger.error("--video and --dir flags are mutually exclusive, only use one of them")
        sys.exit(1)
    if args.video:
        logger.info(f"Only video {args.video} will be processed")
        process_video(args.video,args.fps,args.limit,args.debug)
        sys.exit(0)
    if args.dir:
        logger.info(f"Folder with videos: {args.dir} will be processed")
        process_videos(args.dir,args.fps,args.limit,args.debug)
        sys.exit(0)
