#include "libuvc/libuvc.h"
#include "libuvc/libuvc_internal.h"
#include <stdio.h>
#include <unistd.h>
#include <linux/usb/video.h>

int done_flag=0;

static const int REQ_TYPE_SET = 0x21;


void button_cb(int button, int state, void *user_ptr){
    printf("button_cb() invoked\n");
}

/* This callback function runs once per frame. Use it to perform any
 * quick processing you need, or have it put the frame into your application's
 * input queue. If this function takes too long, you'll start losing frames. */
void cb(uvc_frame_t *frame, void *ptr) {
  uvc_frame_t *rgb;
  uvc_error_t ret;
  enum uvc_frame_format *frame_format = (enum uvc_frame_format *)ptr;
  FILE *fp;
  static int jpeg_count = 0;
  /* static const char *H264_FILE = "iOSDevLog.h264";
   * static const char *MJPEG_FILE = ".jpeg";
 */
  char filename[16];

  /* We'll convert the image from YUV/JPEG to BGR, so allocate space */
  rgb = uvc_allocate_frame(frame->width * frame->height * 3);
  if (!rgb) {
    printf("unable to allocate rgb frame!\n");
    return;
  }

  printf("callback! frame_format = %d, width = %d, height = %d, length = %lu, ptr = %d\n",
    frame->frame_format, frame->width, frame->height, frame->data_bytes, (void *) ptr);

  switch (frame->frame_format) {
  case UVC_FRAME_FORMAT_H264:
    /* use `ffplay H264_FILE` to play */
    /* fp = fopen(H264_FILE, "a");
     * fwrite(frame->data, 1, frame->data_bytes, fp);
     * fclose(fp); */
    break;
  case UVC_COLOR_FORMAT_MJPEG:
    /* sprintf(filename, "%d%s", jpeg_count++, MJPEG_FILE);
     * fp = fopen(filename, "w");
     * fwrite(frame->data, 1, frame->data_bytes, fp);
     * fclose(fp); */
    break;
 case UVC_COLOR_FORMAT_YUYV:
    /* Do the BGR conversion */
    ret = uvc_any2rgb(frame, rgb);
    if (ret) {
      uvc_perror(ret, "uvc_any2rgb");
      uvc_free_frame(rgb);
      return;
    }
    sprintf(filename, "/tmp/frame_%i_%d.ppm", time(0), jpeg_count++);
    fp = fopen(filename, "w");
       char *header="P6\n2592 1944\n255\n";
    fwrite(header, 1, strlen(header), fp);
    if (fwrite(rgb->data, 1, rgb->data_bytes, fp)!=rgb->data_bytes) exit(1);
    fclose(fp);
    done_flag=1;
    break;
  default:
    break;
  }


  if (frame->sequence % 30 == 0) {
    printf(" * got image %u\n",  frame->sequence);
  }

  /* Call a user function:
   *
   * my_type *my_obj = (*my_type) ptr;
   * my_user_function(ptr, rgb);
   * my_other_function(ptr, rgb->data, rgb->width, rgb->height);
   */

  /* Call a C++ method:
   *
   * my_type *my_obj = (*my_type) ptr;
   * my_obj->my_func(rgb);
   */

  /* Use opencv.highgui to display the image:
   * 
   * cvImg = cvCreateImageHeader(
   *     cvSize(rgb->width, rgb->height),
   *     IPL_DEPTH_8U,
   *     3);
   *
   * cvSetData(cvImg, rgb->data, rgb->width * 3); 
   *
   * cvNamedWindow("Test", CV_WINDOW_AUTOSIZE);
   * cvShowImage("Test", cvImg);
   * cvWaitKey(10);
   *
   * cvReleaseImageHeader(&cvImg);
   */

  uvc_free_frame(rgb);
}

/**
 * @brief Perform a SET_CUR request to a terminal or unit.
 * 
 * @param devh UVC device handle
 * @param unit Unit or Terminal ID
 * @param ctrl Control number to set
 * @param data Data buffer to be sent to the device
 * @param len Size of data buffer
 * @return On success, the number of bytes actually transferred. Otherwise,
 *   a uvc_error_t error describing the error encountered.
 * @ingroup ctrl
int uvc_set_ctrl(uvc_device_handle_t *devh, uint8_t unit, uint8_t ctrl, void *data, int len) {
  return libusb_control_transfer(
    devh->usb_devh,
    REQ_TYPE_SET, UVC_SET_CUR,
    ctrl << 8,
    unit << 8 | devh->info->ctrl_if.bInterfaceNumber,          // XXX saki
    data,
    len,
    0 );
}
 */
/*uvc_error_t uvc_set_ae_mode(uvc_device_handle_t *devh, uint8_t mode) {
  ret = libusb_control_transfer(
    devh->usb_devh,
    REQ_TYPE_SET, UVC_SET_CUR,
    UVC_CT_AE_MODE_CONTROL << 8,
    uvc_get_camera_terminal(devh)->bTerminalID << 8 | devh->info->ctrl_if.bInterfaceNumber,
    data,
    sizeof(data),
    0);
*/
void tamtus_request(uvc_device_handle_t *devh, char *data,int len){
uvc_error_t res;
            puts("sending custom request...");
// https://libusb.sourceforge.io/api-1.0/group__libusb__syncio.html#gadb11f7a761bd12fc77a07f4568d56f38
/*
*/
  res = libusb_control_transfer(
    // dev_handle
    devh->usb_devh,
       // bmRequestType
// 0x21 /*REQ_TYPE_SET*/, 0x04 /*UVC_SET_CUR*/,
     0x21 /*REQ_TYPE_SET*/,
       // bRequest
        0x01,
       // wValue
    0x0400, // 0x04 << 8,
       // wIndex
       0x400,
    //uvc_get_camera_terminal(devh)->bTerminalID << 8 | devh->info->ctrl_if.bInterfaceNumber,
       // data, wLength
 //   "\x20\x90\xb0\x72", 4,
   data,len,
       // timeout
    0
);

/*
captured:
Setup Data
    bmRequestType: 0x21
        0... .... = Direction: Host-to-device
        .01. .... = Type: Class (0x1)
        ...0 0001 = Recipient: Interface (0x01)
    bRequest: 1
    wValue: 0x0400
    wIndex: 1024 (0x0400)
    wLength: 4
    Data Fragment: 2032b022
*/

if (0) res=uvc_set_ctrl(devh,uvc_get_camera_terminal(devh)->bTerminalID,0x04,data,len);


          if (res==4){
            puts("custom request success!");
          } else if (res<0){
            uvc_perror(res, "custom request failed");
          } else {
            puts("custom request incomplete!");
          }
}

void report_status(uvc_error_t res, const char* operation){
          if (res == UVC_SUCCESS) {
            fprintf(stdout,"%s: success\n",operation);
          //} else if (res == UVC_ERROR_PIPE) {
          //  uvc_perror(res, " ... uvc_set_ae_mode failed to enable manual exposure");
          //  /* this error indicates that the camera does not support ...
          } else {
            uvc_perror(res, operation);
          }

}

// left
void left(uvc_device_handle_t *devh){
//tamtus_request(devh,"\x20\x32\xb0\x22", 4);
//tamtus_request(devh,"\x20\x31\xb0\x22", 4);

//tamtus_request(devh,"\x20\x90\xb0\x22", 4);


tamtus_request(devh,"\x20\x90\xb0\x72", 4); // from end of right
tamtus_request(devh,"\x20\x32\xb0\x20", 4); // suffices to flip left, no lamp

//tamtus_request(devh,"\x20\x90\xb0\x22", 4);
//tamtus_request(devh,"\x20\x31\xb0\x20", 4);

//tamtus_request(devh,"\x20\x32\xb0\x72", 4); // flip image?
//tamtus_request(devh,"\x20\x31\xb0\x7d", 4);
//tamtus_request(devh,"\x20\x90\xb0\x72", 4);
}

// right
void right(uvc_device_handle_t *devh){
//tamtus_request(devh,"\x19\x32\xb0\x22", 4); // alone turns on light after delay
//tamtus_request(devh,"\x20\x31\xb0\x22", 4);
//tamtus_request(devh,"\x20\x90\xb0\x22", 4);
tamtus_request(devh,"\x20\x32\xb0\x22", 4);

tamtus_request(devh,"\x20\x90\xb0\x20", 4);
//tamtus_request(devh,"\x20\x31\xb0\x20", 4);

//tamtus_request(devh,"\x20\x32\xb0\x72", 4); // flip image?
//tamtus_request(devh,"\x20\x31\xb0\x7d", 4);
//tamtus_request(devh,"\x20\x90\xb0\x72", 4);
}

int main(int argc, char **argv) {
  uvc_context_t *ctx;
  uvc_device_t *dev;
  uvc_device_handle_t *devh;
  uvc_stream_ctrl_t ctrl;
  uvc_error_t res;

  /* Initialize a UVC service context. Libuvc will set up its own libusb
   * context. Replace NULL with a libusb_context pointer to run libuvc
   * from an existing libusb context. */
  res = uvc_init(&ctx, NULL);

  if (res < 0) {
    uvc_perror(res, "uvc_init");
    return res;
  }

  puts("UVC initialized");

  /* Locates the first attached UVC device, stores in dev */
  res = uvc_find_device(
      ctx, &dev,
//      0, 0, NULL); /* filter devices: vendor_id, product_id, "serial_num" */
      0x0ac8, 0xcd58, NULL); /* filter devices: vendor_id, product_id, "serial_num" */

// 0ac8:cd58 Z-Star Microelectronics Corp. TAMTUS-CAM500


  if (res < 0) {
    uvc_perror(res, "uvc_find_device"); /* no devices found */
  } else {
    puts("Device found");

    /* Try to open the device: requires exclusive access */
    res = uvc_open(dev, &devh);

    if (res < 0) {
      uvc_perror(res, "uvc_open"); /* unable to open device */
    } else {
      puts("Device opened");

      /* Print out a message containing all the information that libuvc
       * knows about the device */
      uvc_print_diag(devh, stderr);

      const uvc_format_desc_t *format_desc = uvc_get_format_descs(devh);
      const uvc_frame_desc_t *frame_desc = format_desc->frame_descs;
      enum uvc_frame_format frame_format;
      int width = 640;
      int height = 480;
      int fps = 30;

      switch (format_desc->bDescriptorSubtype) {
      case UVC_VS_FORMAT_MJPEG:
        frame_format = UVC_COLOR_FORMAT_MJPEG;
        break;
      case UVC_VS_FORMAT_FRAME_BASED:
        frame_format = UVC_FRAME_FORMAT_H264;
        break;
      default:
        frame_format = UVC_FRAME_FORMAT_YUYV;
        break;
      }

      if (frame_desc) {
        width = frame_desc->wWidth;
        height = frame_desc->wHeight;
        fps = 10000000 / frame_desc->dwDefaultFrameInterval;
      }

      printf("\nFirst format: (%4s) %dx%d %dfps\n", format_desc->fourccFormat, width, height, fps);

      /* Try to negotiate first stream profile */
      res = uvc_get_stream_ctrl_format_size(
          devh, &ctrl, /* result stored in ctrl */
          frame_format,
          width, height, fps /* width, height, fps */
      );

      /* Print out the result */
      uvc_print_stream_ctrl(&ctrl, stderr);




      if (res < 0) {
        uvc_perror(res, "get_mode"); /* device doesn't provide a matching stream */
      } else {
        /* Start the video stream. The library will call user function cb:
         *   cb(frame, (void *) 12345)
         */
        //res = uvc_start_streaming(devh, &ctrl, cb, (void *) 12345, 0);
        //res = uvc_start_streaming(devh, &ctrl, NULL, NULL, 0);
uvc_stream_handle_t * strmp;
        res = uvc_stream_open_ctrl(devh, &strmp, &ctrl);
        if (res < 0) {
          uvc_perror(res, "uvc_stream_open_ctrl()"); /* unable to start stream */
          return 1;
        }
        res = uvc_stream_start(strmp, NULL, NULL, 0);
        if (res < 0) {
          uvc_perror(res, "uvc_stream_start()"); /* unable to start stream */
          return 1;
        }

        uvc_set_button_callback(devh, button_cb, NULL);
        if (res < 0) {
          uvc_perror(res, "uvc_set_button_callback()"); /* unable to start stream */
          return 1;
        }

        if (res < 0) {
          uvc_perror(res, "start_streaming"); /* unable to start stream */
        } else {
          puts("Streaming...");

          /* enable auto exposure - see uvc_set_ae_mode documentation */
/*
          puts("Enabling auto exposure ...");
          const uint8_t UVC_AUTO_EXPOSURE_MODE_MANUAL = 1;
          const uint8_t UVC_AUTO_EXPOSURE_MODE_AUTO = 2;
          const uint8_t UVC_AUTO_EXPOSURE_MODE_SHUTTER_PRIORITY = 4;
          const uint8_t UVC_AUTO_EXPOSURE_MODE_APERTURE_PRIORITY = 8;
          //res = uvc_set_ae_mode(devh, UVC_AUTO_EXPOSURE_MODE_MANUAL);
          res = uvc_set_ae_mode(devh, UVC_AUTO_EXPOSURE_MODE_AUTO);
          if (res == UVC_SUCCESS) {
            puts(" ... enabled manual exposure");
          } else if (res == UVC_ERROR_PIPE) {
            uvc_perror(res, " ... uvc_set_ae_mode failed to enable manual exposure");
            /* this error indicates that the camera does not support the full AE mode;
             * try again, using aperture priority mode (fixed aperture, variable exposure time) */
            //puts(" ... full AE not supported, trying aperture priority mode");
            //res = uvc_set_ae_mode(devh, UVC_AUTO_EXPOSURE_MODE_APERTURE_PRIORITY);
            //if (res < 0) {
            //  uvc_perror(res, " ... uvc_set_ae_mode failed to enable aperture priority mode");
            //} else {
            //  puts(" ... enabled aperture priority auto exposure mode");
            //}
 /*         } else {
            uvc_perror(res, " ... uvc_set_ae_mode failed to enable manual exposure mode");
          }
*/
/*
uvc_error_t uvc_set_exposure_abs(uvc_device_handle_t *devh, uint32_t time);
uvc_error_t uvc_set_exposure_rel(uvc_device_handle_t *devh, int8_t step);
uvc_error_t uvc_set_iris_abs(uvc_device_handle_t *devh, uint16_t iris);
uvc_error_t uvc_set_iris_rel(uvc_device_handle_t *devh, uint8_t iris_rel);
uvc_error_t uvc_set_brightness(uvc_device_handle_t *devh, int16_t brightness);
uvc_error_t uvc_set_contrast(uvc_device_handle_t *devh, uint16_t contrast);
uvc_error_t uvc_set_contrast_auto(uvc_device_handle_t *devh, uint8_t contrast_auto);
*/
/*
           res = uvc_set_contrast(devh, 0x0000);
           report_status(res,"uvc_set_contrast");

           res = uvc_set_brightness(devh, 0x0000);
           report_status(res,"uvc_set_brightness");

           res = uvc_set_iris_abs(devh, 0x0003);
           report_status(res,"uvc_set_iris_abs");

           res = uvc_set_exposure_abs(devh, 0x0010);
           report_status(res,"uvc_set_exposure_abs");

*/



          if (0) res=uvc_set_ctrl(devh,
            uvc_get_camera_terminal(devh)->bTerminalID << 8 | ((uvc_device_handle_t *)devh)->info->ctrl_if.bInterfaceNumber,
            0x04,
            "\x20\x90\xb0\x72", 4
          );
//     tamtus_request(devh,"\x20\x90\xb0\x72", 4);
//     tamtus_request(devh,"\x20\x90\xb0\x72", 4);
//     tamtus_request(devh,"\x20\x90\xb0\x72", 4);
//     tamtus_request(devh,"\x20\x90\xb0\x72", 4);

while(1){

  uvc_frame_t *frame = uvc_allocate_frame(0);
  uvc_frame_t *rgbframe = uvc_allocate_frame(0);

int mytime = time(0);
char filename_r[256];
char filename_l[256];
right(devh);
usleep(1000000);
uvc_stream_get_frame(strmp, &frame, 0);
uvc_stream_get_frame(strmp, &frame, 0);

    int ret = uvc_any2rgb(frame, rgbframe);
    if (ret) {
      uvc_perror(ret, "uvc_any2rgb");
      return 1;
    }
    sprintf(filename_r, "/tmp/frame_%i_right.ppm", mytime);
    FILE *fp = fopen(filename_r, "w");
       char *header="P6\n2592 1944\n255\n";
    fwrite(header, 1, strlen(header), fp);
    if (fwrite(rgbframe->data, 1, rgbframe->data_bytes, fp)!=rgbframe->data_bytes) exit(1);
    fclose(fp);

left(devh);

usleep(1000000);
uvc_stream_get_frame(strmp, &frame, 0);
uvc_stream_get_frame(strmp, &frame, 0);

    ret = uvc_any2rgb(frame, rgbframe);
    if (ret) {
      uvc_perror(ret, "uvc_any2rgb");
      return 1;
    }
    sprintf(filename_l, "/tmp/frame_%i_left.ppm", mytime);
    fp = fopen(filename_l, "w");
    fwrite(header, 1, strlen(header), fp);
    if (fwrite(rgbframe->data, 1, rgbframe->data_bytes, fp)!=rgbframe->data_bytes) exit(1);
    fclose(fp);

system("beep");

char command[1024];
char output[1024];

//char *output="| ppmtopgm | cjpeg -q 95 >/dump/TAMTUS/double_%i.jpg";
char *outputf="| ppmtopgm | pnmtopng >/dump/TAMTUS/double_%i.png";

sprintf(output,outputf,mytime);

//sprintf(command,"bash -c 'pnmcat -lr <(pnmflip -r90 <%s) <(pnmflip -r90 <%s) | ppmtopgm | cjpeg -q 95 >/dump/TAMTUS/double_%i.jpg'",filename_r,filename_l,mytime);
sprintf(command,"bash -c 'set -x ; pnmcat -lr <(pnmflip -r270 <%s) <(pnmflip -r270 <%s)  %s'",filename_r,filename_l,output);
system(command);
// pnmcat -lr <(pnmflip -r90 </tmp/frame_1646873681_right.ppm) <(pnmflip -r90 </tmp/frame_1646873681_left.ppm) >/tmp/frame_1646873681.ppm
unlink(filename_r);
unlink(filename_l);
sleep(2);
}



//while (done_flag==0) usleep(10000);
//          sleep(10); /* stream for 10 seconds */

          /* End the stream. Blocks until last callback is serviced */
          uvc_stop_streaming(devh);
          puts("Done streaming.");
        }
      }

      /* Release our handle on the device */
      uvc_close(devh);
      puts("Device closed");
    }

    /* Release the device descriptor */
    uvc_unref_device(dev);
  }

  /* Close the UVC context. This closes and cleans up any existing device handles,
   * and it closes the libusb context if one was not provided. */
  uvc_exit(ctx);
  puts("UVC exited");

  return 0;
}

